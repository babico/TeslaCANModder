# Optimisations Performance — ESP32-S3 Camera

> Analyse du firmware `Camera/src/main.cpp` (380 lignes) sur ESP32-S3 Freenove  
> ESP32-S3 dual-core 240 MHz, 8MB PSRAM OPI, OV5640, USB-C  
> MAC: E0:72:A1:D8:FE:2C

---

## Architecture actuelle

```
┌──────────────── CORE 0 ────────────────┬──────────────── CORE 1 ────────────────┐
│ WiFi stack (système, prio 23)          │ streamTask (prio 1)                    │
│ lwIP TCP/IP stack                      │ ├─ poll ctrl socket (MSG_DONTWAIT)     │
│                                        │ ├─ esp_camera_fb_get()                 │
│ Intentionnellement libre pour WiFi     │ ├─ sendFrame() — 10-15 chunks UDP      │
│                                        │ │   ├─ memcpy header × 4              │
│ ≈ 20-40% utilisé (WiFi TX processing)  │ │   ├─ memcpy payload                 │
│                                        │ │   ├─ sendto() × N chunks            │
│                                        │ │   └─ vTaskDelay(1ms) every 3 chunks │
│                                        │ ├─ esp_camera_fb_return()              │
│                                        │ └─ taskYIELD()                         │
│                                        │                                        │
│ loop() — delay(1000) (inutile)         │ ≈ 70-90% utilisé                       │
└────────────────────────────────────────┴────────────────────────────────────────┘
```

**Pipeline streaming :**
```
[Camera DMA → PSRAM] → [CPU memcpy → packet] → [sendto → lwIP → WiFi TX queue → Radio DMA]
     ~6-10ms               ~0.1ms/chunk            ~0.5ms/chunk + 1ms pace/3 chunks
```

---

## Paramètres actuels

| Paramètre | Valeur | Commentaire |
|-----------|--------|-------------|
| Résolution | HVGA (480×320) | Plus grand que l'écran 360×360 |
| JPEG quality | 18 | Compression forte, ~8-15 KB/frame |
| Frame buffers | 2 (PSRAM) | Double-buffer camera |
| XCLK | 20 MHz | OV5640 supporte 24 MHz |
| Chunk size | 1400 octets | Conservateur, sous MTU |
| Pacing | 1ms delay / 3 chunks | ~3ms ajouté par frame |
| UDP target | Unicast 192.168.4.2 | Correct pour receiver unique |
| Send buffer | 32 KB | Suffisant pour ~2 frames |
| grab_mode | CAMERA_GRAB_LATEST | Évite les frames périmées |
| Sensor sleep | OV5640 standby register | 0x3008 bit 6 |

---

## Tableau récapitulatif

| # | Optimisation | Gain perf | Complexité | Ratio | Dual-core |
|---|-------------|-----------|------------|-------|-----------|
| 1 | [Réduire le pacing UDP](#1-rduire-le-pacing-udp) | ★★★★☆ | ★☆☆☆☆ | 4.00 | Non |
| 2 | [Pipeline capture/send dual-core](#2-pipeline-capturesend-dual-core) | ★★★★☆ | ★★★☆☆ | 1.33 | ✅ Oui |
| 3 | [Résolution optimale pour 360×360](#3-rsolution-optimale-pour-360360) | ★★★☆☆ | ★☆☆☆☆ | 3.00 | Non |
| 4 | [Éliminer les memcpy headers](#4-liminer-les-memcpy-headers) | ★★☆☆☆ | ★☆☆☆☆ | 2.00 | Non |
| 5 | [Augmenter XCLK à 24 MHz](#5-augmenter-xclk--24-mhz) | ★★☆☆☆ | ★☆☆☆☆ | 2.00 | Non |
| 6 | [Send task sur Core 0](#6-send-task-sur-core-0) | ★★★☆☆ | ★★☆☆☆ | 1.50 | ✅ Oui |
| 7 | [Scatter-gather sendmsg()](#7-scatter-gather-sendmsg) | ★★☆☆☆ | ★★☆☆☆ | 1.00 | Non |
| 8 | [JPEG quality adaptatif](#8-jpeg-quality-adaptatif) | ★★☆☆☆ | ★★☆☆☆ | 1.00 | Non |
| 9 | [UDP checksum hardware offload](#9-udp-checksum-hardware-offload) | ★☆☆☆☆ | ★☆☆☆☆ | 1.00 | Non |
| 10 | [Ctrl socket sur task séparée](#10-ctrl-socket-sur-task-spare) | ★☆☆☆☆ | ★☆☆☆☆ | 1.00 | ✅ Oui |

---

## Détails par optimisation

### 1. Réduire le pacing UDP

**Gain : ★★★★☆ | Complexité : ★☆☆☆☆**

**Problème** : Toutes les 3 chunks, un `vTaskDelay(pdMS_TO_TICKS(1))` est ajouté. Pour une frame HVGA JPEG (~12 KB = 9 chunks), c'est **3 délais de 1ms = 3ms ajoutés par frame**. À 30 FPS, c'est 90ms/s de temps gâché.

Le but du pacing est d'éviter le overflow du buffer WiFi TX. Mais 1ms est très conservateur — le WiFi TX drains à ~2-4 MB/s en SoftAP, soit ~3 KB/ms. En 1ms, on peut envoyer 2 chunks.

**Solution** :
```cpp
// Option A: Réduire le délai
if (i % 3 == 2) taskYIELD();  // yield simple au lieu de delay(1)

// Option B: Pacing adaptatif basé sur le retour de sendto
int ret = sendto(udp_sock, packet, HEADER_SIZE + size, 0, ...);
if (ret < 0 && errno == ENOMEM) {
    vTaskDelay(pdMS_TO_TICKS(1));  // Seulement si buffer plein
    sendto(udp_sock, packet, HEADER_SIZE + size, 0, ...);  // retry
}
// Sinon : pas de délai

// Option C: MSG_DONTWAIT + retry
int ret = sendto(udp_sock, packet, HEADER_SIZE + size, MSG_DONTWAIT, ...);
if (ret < 0) {
    vTaskDelay(pdMS_TO_TICKS(1));
    sendto(udp_sock, packet, HEADER_SIZE + size, 0, ...);
}
```

**Gain estimé** : 3ms/frame → ~0.3ms/frame. FPS potentiel +10-15%.

---

### 2. Pipeline capture/send dual-core

**Gain : ★★★★☆ | Complexité : ★★★☆☆**

**Problème** : Actuellement séquentiel sur Core 1 :
```
[capture 8ms] → [send 5ms] → [capture 8ms] → [send 5ms] → ...
Total: 13ms/frame = ~77 FPS max
```

**Solution** : Capture sur Core 1, envoi sur Core 0 :
```
Core 1: [capture N] ─────────── [capture N+1] ─────────── [capture N+2]
Core 0:              [send N] ──              [send N+1] ──
                     ↑ queue                  ↑ queue
```

```cpp
static QueueHandle_t frameQueue;

// Core 1 : capture task
void captureTask(void *param) {
    while (true) {
        if (!streamEnabled) { vTaskDelay(50); continue; }
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) {
            // Non-blocking: si la queue est pleine, drop la frame
            if (xQueueSend(frameQueue, &fb, 0) != pdTRUE) {
                esp_camera_fb_return(fb);  // drop
            }
        }
    }
}

// Core 0 : send task
void sendTask(void *param) {
    camera_fb_t *fb;
    while (true) {
        if (xQueueReceive(frameQueue, &fb, pdMS_TO_TICKS(100))) {
            sendFrame(fb);
            esp_camera_fb_return(fb);
        }
    }
}

// setup()
frameQueue = xQueueCreate(1, sizeof(camera_fb_t*));
xTaskCreatePinnedToCore(captureTask, "capture", 4096, NULL, 2, NULL, 1);
xTaskCreatePinnedToCore(sendTask, "send", 4096, NULL, 2, NULL, 0);
```

**Attention** : Placer le send sur Core 0 entre en concurrence avec le WiFi driver. Il faudrait mesurer si le WiFi driver est gêné. Alternative : garder le send sur Core 1 et déplacer le ctrl polling sur Core 0.

**Gain estimé** : Throughput augmente de ~30% car capture et envoi se chevauchent. Passe de ~77 FPS théorique à ~100+ FPS (limité par le WiFi).

---

### 3. Résolution optimale pour 360×360

**Gain : ★★★☆☆ | Complexité : ★☆☆☆☆**

**Problème** : HVGA = 480×320 pixels. L'écran est 360×360. Le côté Ecran doit :
- Décoder le JPEG complet (480×320)
- Cropper au centre (60 px perdus en largeur)
- L'OV5640 encode/transmet des pixels inutiles

Le crop actuel (dans le firmware Ecran) :
```cpp
cropX = (imgW >= SCREEN_W) ? (imgW - SCREEN_W) / 2 : 0;  // (480-360)/2 = 60
cropY = (imgH >= SCREEN_H) ? (imgH - SCREEN_H) / 2 : 0;  // (320-360)/2 → 0 (plus petit)
```

480×320 → crop 360×320 → affiche avec bandes noires en haut/bas (40px manquants).

**Option A: FRAMESIZE_CIF (400×296)** — plus petit, moins de crop :
```cpp
cfg.frame_size = FRAMESIZE_CIF;  // 400×296 : crop X=20, manque 64px en Y
```

Économise (480×320 - 400×296) = 35,200 pixels de moins à encoder/transmettre/décoder (-23%).

**Option B: Custom 360×360 via OV5640 `set_res_raw`** — résolution carrée exacte :

#### Analyse de faisabilité détaillée

**L'API existe et est implémentée** : Confirmé dans le source espressif/esp32-camera (ov5640.c L1013). L'OV5640 supporte le mode aspect ratio 1:1 via sa `ratio_table` interne.

**Pipeline ISP OV5640 pour 360×360 :**
```
Sensor array 2592×1944
    ↓ Window 1:1 (crop côtés, startX=320)
ISP input 1920×1920
    ↓ Binning 2×2
960×960
    ↓ ISP Scale ÷2.67
360×360 ← output direct du capteur
    ↓ JPEG encoder interne
Flux JPEG 360×360 (~6-10 KB)
```

**Code d'implémentation :**
```cpp
// 1. Init avec HVGA pour le PLL et les buffers DMA
cfg.frame_size = FRAMESIZE_HVGA;
esp_camera_init(&cfg);

// 2. Appliquer vflip/hmirror d'abord
sensor_t *s = esp_camera_sensor_get();
s->set_vflip(s, 1);
s->set_hmirror(s, 0);

// 3. Override vers 360×360 carré
int ret = s->set_res_raw(s,
    320,    // startX — crop côtés pour 1:1
    0,      // startY
    2543,   // endX
    1951,   // endY
    32,     // offsetX
    16,     // offsetY
    2684,   // totalX (timing)
    1968,   // totalY (timing)
    360,    // outputX ← résolution cible
    360,    // outputY ← résolution cible
    true,   // scale   — ISP downscale activé
    true    // binning — 2×2
);
Serial.printf("[CAM] set_res_raw 360x360: %d\n", ret);
```

**Risques identifiés :**

| Risque | Sévérité | Mitigation |
|--------|----------|------------|
| JPEG MCU alignment (360 ≠ 16×N) | Moyen | Tester 352×352 (=22×16) d'abord, puis 360 |
| `fb->width/height` incorrect | Faible | L'Ecran utilise `jpeg.getWidth()` du header JPEG, pas fb metadata |
| PLL/timing | Faible | PLL configuré par HVGA en amont, 360² < 480×320 → plus rapide |
| DMA | Faible | JPEG = flux d'octets terminé par VSYNC, indépendant de la résolution pixel |
| Fallback | Aucun | Supprimer `set_res_raw` → retour à HVGA, aucun effet de bord |

**Gains attendus :**

| Métrique | HVGA (480×320) | Custom 360×360 | Delta |
|----------|---------------|----------------|-------|
| Pixels encodés | 153,600 | 129,600 | **-16%** |
| Taille JPEG estimée | ~12 KB | ~10 KB | **-17%** |
| Chunks UDP | ~9 | ~7 | **-22%** |
| Crop/decode Ecran | 480×320 + crop | 360×360 direct | **-17% CPU** |
| Pixels pertinents | 115,200 (75%) | 129,600 (100%) | **100% utile** |

**Verdict : Faisable, effort faible (~10 lignes), fallback trivial. Tester avec 352×352 d'abord pour sécuriser l'alignement MCU.**

---

### 4. Éliminer les memcpy headers

**Gain : ★★☆☆☆ | Complexité : ★☆☆☆☆**

**Problème** : 4 × `memcpy` de 2 octets chacun pour construire le header de chaque chunk. Micro-optimisation mais simple.

**Solution** : Utiliser un struct packed :
```cpp
#pragma pack(push, 1)
struct ChunkHeader {
    uint16_t frameId;
    uint16_t chunkId;
    uint16_t totalChunks;
    uint16_t chunkSize;
};
#pragma pack(pop)

void sendFrame(camera_fb_t *fb) {
    uint8_t packet[HEADER_SIZE + CHUNK_PAYLOAD];
    ChunkHeader *hdr = (ChunkHeader *)packet;
    hdr->frameId = frame_id;
    // ...
    for (uint16_t i = 0; i < total_chunks; i++) {
        hdr->chunkId = i;
        hdr->chunkSize = size;
        memcpy(packet + HEADER_SIZE, fb->buf + offset, size);
        sendto(...);
    }
}
```

**Gain** : ~0.5μs par chunk. Négligeable en absolu mais plus propre.

---

### 5. Augmenter XCLK à 24 MHz

**Gain : ★★☆☆☆ | Complexité : ★☆☆☆☆**

**Problème** : `xclk_freq_hz = 20000000` (20 MHz). L'OV5640 est spécifié jusqu'à 24 MHz, et beaucoup de boards utilisent 24 MHz par défaut.

**Solution** (1 ligne) :
```cpp
cfg.xclk_freq_hz = 24000000;  // 24 MHz — plus rapide capture
```

**Gain** : Le pixel clock augmente proportionnellement. Temps de capture réduit de ~17% (480×320 à HVGA). Impact potentiel : +3-5 FPS.

**Risque** : Vérifier que le signal est stable (capacité de pin, longueur de piste). Tester sur le PCB réel.

---

### 6. Send task sur Core 0

**Gain : ★★★☆☆ | Complexité : ★★☆☆☆**

**Problème** : Le choix de garder Core 0 libre pour WiFi est bon en théorie, mais le WiFi driver ne consomme que 20-40% de Core 0. Le reste est gâché dans `loop() { delay(1000); }`.

**Solution** : Version simplifiée de #2 — déplacer uniquement le `sendFrame()` sur Core 0 :
```cpp
// Core 0 : envoie les frames (cohabite avec WiFi stack)
xTaskCreatePinnedToCore(sendTask, "send", 4096, NULL, 1, NULL, 0);
```

**Avantage par rapport à #2** : Plus simple car pas de pipeline full duplex, juste un déplacement de la charge.

**Risque** : Le `sendto()` lui-même utilise lwIP qui tourne partiellement sur Core 0. Envoyer depuis Core 0 pourrait être plus efficace (cache locality) ou créer de la contention. À mesurer.

---

### 7. Scatter-gather sendmsg()

**Gain : ★★☆☆☆ | Complexité : ★★☆☆☆**

**Problème** : Actuellement, le header (8 octets) est copié dans `packet[]`, puis le payload (1400 octets) est aussi copié. Le `sendto` recopie ensuite le tout dans le buffer lwIP. Double copie pour le payload.

**Solution** : Utiliser `sendmsg()` avec scatter-gather (iovec) pour éviter la copie payload :
```cpp
struct iovec iov[2];
iov[0].iov_base = &header;
iov[0].iov_len = HEADER_SIZE;
iov[1].iov_base = fb->buf + offset;  // Pointeur direct dans le framebuffer
iov[1].iov_len = size;

struct msghdr msg = {};
msg.msg_name = &dest_addr;
msg.msg_namelen = sizeof(dest_addr);
msg.msg_iov = iov;
msg.msg_iovlen = 2;

sendmsg(udp_sock, &msg, 0);
```

**Gain** : Élimine 1 memcpy de 1400 octets par chunk. Pour 10 chunks/frame : ~14 KB évités. À 30 FPS : ~420 KB/s de memcpy économisé.

**Note** : Vérifier que lwIP ESP-IDF supporte sendmsg. La plupart des versions récentes le supportent.

---

### 8. JPEG quality adaptatif

**Gain : ★★☆☆☆ | Complexité : ★★☆☆☆**

**Problème** : JPEG quality=18 est fixe. Pour des scènes simples (ciel, parking vide), on pourrait réduire davantage. Pour des scènes complexes (manœuvre serrée), on voudrait plus de détail.

**Solution** : Ajuster la qualité en fonction de la taille du dernier frame :
```cpp
static int currentQuality = 18;

// Après sendFrame:
if (fb->len > 15000) {
    currentQuality = min(currentQuality + 2, 30);  // plus de compression
} else if (fb->len < 8000) {
    currentQuality = max(currentQuality - 1, 10);  // plus de qualité
}
sensor_t *s = esp_camera_sensor_get();
s->set_quality(s, currentQuality);
```

**Objectif** : Garder la taille JPEG dans une bande cible (~10-12 KB) pour une latence UDP constante.

---

### 9. UDP checksum hardware offload

**Gain : ★☆☆☆☆ | Complexité : ★☆☆☆☆**

**Problème** : lwIP calcule le checksum UDP en logiciel par défaut.

**Solution** : Vérifier que le hardware checksum offload est activé dans sdkconfig :
```
CONFIG_LWIP_CHECKSUM_CHECK_ICMP=n
CONFIG_LWIP_CHECKSUM_CHECK_UDP=n
```

Ou désactiver le checksum UDP sortant (acceptable pour du réseau local point-à-point) :
```cpp
int no_chksum = 1;
setsockopt(udp_sock, IPPROTO_UDP, UDP_NO_CHECK6, &no_chksum, sizeof(no_chksum));
```

**Note** : Sur le réseau local SoftAP, les erreurs de transmission sont rares et le JPEG est tolérant aux pertes. Le checksum est superflu.

---

### 10. Ctrl socket sur task séparée

**Gain : ★☆☆☆☆ | Complexité : ★☆☆☆☆**

**Problème** : Le contrôle (STRT/STOP) est pollé dans la boucle de streaming avec `MSG_DONTWAIT`. Un appel système `recvfrom` inutile à chaque itération.

**Solution** : Task séparée (légère) pour le contrôle :
```cpp
void ctrlTask(void *param) {
    // bind ctrl_sock...
    while (true) {
        char cbuf[8];
        int clen = recvfrom(ctrl_sock, cbuf, sizeof(cbuf), 0, NULL, NULL);  // blocking
        if (clen >= 4) {
            if (memcmp(cbuf, "STRT", 4) == 0) streamEnabled = true;
            else if (memcmp(cbuf, "STOP", 4) == 0) streamEnabled = false;
        }
    }
}
```

**Gain** : Supprime un syscall par frame dans la boucle de streaming. Marginal.

---

## Plan d'exécution recommandé

### Phase 1 — Quick wins (effort faible)
1. **#1** — Pacing adaptatif (plus gros gain/effort)
2. **#5** — XCLK 24 MHz (1 ligne)
3. **#4** — Struct packed pour headers
4. **#3** — Tester CIF (400×296) au lieu de HVGA

### Phase 2 — Dual-core
5. **#6** — Send task sur Core 0 (ou #2 pipeline complet)
6. **#10** — Ctrl socket séparé

### Phase 3 — Raffinements
7. **#7** — Scatter-gather sendmsg (si supporté)
8. **#8** — JPEG quality adaptatif
9. **#9** — UDP checksum offload

---

## Métriques à mesurer

Le firmware a déjà un compteur FPS (log toutes les 60 frames). Pour des mesures plus fines :

```cpp
// Temps de capture
unsigned long t0 = micros();
camera_fb_t *fb = esp_camera_fb_get();
unsigned long captureUs = micros() - t0;

// Temps d'envoi
unsigned long t1 = micros();
sendFrame(fb);
unsigned long sendUs = micros() - t1;

Serial.printf("[PERF] capture=%lu µs, send=%lu µs, size=%u\n", captureUs, sendUs, fb->len);
```

Indicateurs clés :
- **FPS** : déjà mesuré (target: 30+ FPS stable)
- **Latence bout-en-bout** : capture → reception Ecran (mesure via timestamp dans header)
- **Taux de perte** : chunks envoyés vs reçus (compteur Ecran)
- **Taille JPEG** : influence directe sur le nombre de chunks et la latence
- **WiFi TX queue** : monitorer `ESP_ERR_*` retours de sendto pour détecter les saturations

---

## Comparaison avec le firmware Ecran

| Aspect | Camera | Ecran | Goulot d'étranglement |
|--------|--------|-------|----------------------|
| Core 0 utilisation | ~20-40% (WiFi) | ~5% (ESP-NOW sporadique) | Camera = mieux réparti |
| Core 1 utilisation | ~70-90% (capture+send) | ~90%+ (LVGL+SPI polling) | **Ecran = plus critique** |
| Bottleneck principal | UDP pacing (3ms/frame) | SPI polling (17ms/frame) | Ecran 5× pire |
| Double-buffer | Oui (camera fb) | Oui mais inutile (flush sync) | Ecran gaspille |
| PSRAM access | Frame buffer read | LVGL draw + canvas read | Similaire |

**Conclusion** : Le firmware Camera est raisonnablement bien optimisé. Les gains potentiels sont de l'ordre de **+10-30% FPS**. Le firmware Ecran a un problème structurel beaucoup plus grave (SPI polling) avec un potentiel de gain de **+100%**.
