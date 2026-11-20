# Optimisations Performance — ESP32-S3 Ecran

> Analyse du firmware `main.cpp` (1854 lignes) sur JC3636W518C  
> ESP32-S3 dual-core 240 MHz, 8MB PSRAM OPI, 16MB Flash, ST77916 QSPI 360×360

---

## État actuel des cores

```
┌──────────────── CORE 0 ────────────────┬──────────────── CORE 1 ────────────────┐
│ WiFi stack (système, prio 23)          │ Arduino loop() (prio 1)                │
│ ESP-NOW callback — sporadique          │ ├─ updateDashboard() — formatting      │
│ udpRecvTask (prio 2) — idle en dash    │ ├─ lv_timer_handler() — LVGL render    │
│                                        │ ├─ lvFlushCb() — SPI bloquant ██████   │
│ ≈ 5% utilisé en mode dashboard         │ ├─ JPEG decode (mode caméra)           │
│ ≈ 30% utilisé en mode caméra           │ ├─ Touch polling                       │
│                                        │ └─ Blink/halo logic                    │
│                                        │ ≈ 90%+ utilisé                         │
└────────────────────────────────────────┴────────────────────────────────────────┘
```

**Problème principal** : Core 0 quasi-inactif, Core 1 surchargé (SPI bloquant + render + data formatting).

---

## Tableau récapitulatif

| # | Optimisation | Gain perf | Complexité | Ratio | Dual-core |
|---|-------------|-----------|------------|-------|-----------|
| 1 | [DMA flush SPI (QSPI)](#1-dma-flush-spi) | ★★★★★ | ★★★☆☆ | 1.67 | Indirect |
| 2 | [Draw buffers LVGL → SRAM interne](#2-draw-buffers-lvgl--sram-interne) | ★★★★☆ | ★☆☆☆☆ | 4.00 | Non |
| 3 | [LVGL render task sur Core 0](#3-lvgl-render-task-sur-core-0) | ★★★★★ | ★★★★☆ | 1.25 | ✅ Oui |
| 4 | [JPEG decode sur Core 0](#4-jpeg-decode-sur-core-0) | ★★★★☆ | ★★☆☆☆ | 2.00 | ✅ Oui |
| 5 | [Dirty flags updateDashboard](#5-dirty-flags-updatedashboard) | ★★★☆☆ | ★★☆☆☆ | 1.50 | Non |
| 6 | [recolorCanvas — skip si même couleur](#6-recolorcanvas--skip-si-mme-couleur) | ★★★☆☆ | ★☆☆☆☆ | 3.00 | Non |
| 7 | [Précalculer les ticks](#7-prcalculer-les-ticks) | ★★☆☆☆ | ★☆☆☆☆ | 2.00 | Non |
| 8 | [Int math dans ESP-NOW callback](#8-int-math-dans-esp-now-callback) | ★★☆☆☆ | ★★☆☆☆ | 1.00 | Non |
| 9 | [CAN data processing task Core 0](#9-can-data-processing-task-core-0) | ★★☆☆☆ | ★★★☆☆ | 0.67 | ✅ Oui |
| 10 | [Augmenter LV_BUF_LINES](#10-augmenter-lv_buf_lines) | ★★☆☆☆ | ★☆☆☆☆ | 2.00 | Non |
| 11 | [Rate-limit dashboard loop](#11-rate-limit-dashboard-loop) | ★★☆☆☆ | ★☆☆☆☆ | 2.00 | Non |
| 12 | [Mutex canData](#12-mutex-candata) | ★☆☆☆☆ | ★☆☆☆☆ | 1.00 | Fiabilité |
| 13 | [Éliminer double copie caméra](#13-liminer-double-copie-camra) | ★★★☆☆ | ★★★★☆ | 0.75 | Non |
| 14 | [DST pré-calculé](#14-dst-pr-calcul) | ★☆☆☆☆ | ★☆☆☆☆ | 1.00 | Non |
| 15 | [Pipeline caméra Core 0 decode + Core 1 display](#15-pipeline-camra-dual-core) | ★★★★☆ | ★★★☆☆ | 1.33 | ✅ Oui |

> **Ratio** = Gain / Complexité (plus c'est haut, meilleur est le rapport effort/résultat)  
> Classement recommandé : par ratio décroissant → #2, #6, #7, #10, #11, #4, #1, #5, #15, #3, ...

---

## Détails par optimisation

### 1. DMA flush SPI

**Gain : ★★★★★ | Complexité : ★★★☆☆**

#### Analyse approfondie du driver Arduino_ESP32QSPI

Le code source a été analysé en détail. Voici la chaîne d'appel complète :

```
lvFlushCb(buf, 360×40)
  └→ Arduino_TFT::draw16bitRGBBitmap(x, y, bitmap, w, h)
       ├→ writeAddrWindow(x, y, w, h)        ← set display window via QSPI
       └→ _bus->writePixels(bitmap, 14400)    ← envoie les pixels
            └→ Arduino_ESP32QSPI::writePixels()
                 └→ BOUCLE : 15 chunks de 1024 pixels (ESP32QSPI_MAX_PIXELS_AT_ONCE)
                      ├─ CPU byte-swap via MSB_32_16_16_SET → copie dans _buffer (2KB DMA)
                      ├─ spi_device_polling_start()  ← lance DMA buffer→SPI FIFO
                      └─ spi_device_polling_end()    ← ⚠️ CPU BUSY-LOOP jusqu'à fin
```

**Constats critiques :**

| Paramètre | Valeur | Impact |
|-----------|--------|--------|
| Fréquence QSPI | 40 MHz × 4 lignes = 20 MB/s | Débit OK mais chunks trop petits |
| `ESP32QSPI_MAX_PIXELS_AT_ONCE` | 1024 px = 2048 octets/chunk | **15 transactions SPI par flush** |
| `queue_size` | 1 | Pas de pipeline, 1 seule transaction à la fois |
| `max_transfer_sz` | 16,392 octets | Artificiel — GDMA supporte 4 MB |
| Mode SPI | **Polling** (`spi_device_polling_start/end`) | **Pire que `spi_device_transmit`** (pas ISR) |
| `_buffer` + `_2nd_buffer` | 2× 2KB en SRAM DMA | Double-buffer EXISTS mais inutilisé pour pipeline |
| `_handle` | **private** | Pas accessible sans patch |
| `pre_cb` / `post_cb` | nullptr | Aucun callback async configuré |

> **Surprise** : Le driver s'appelle "QSPI" et initialise bien un canal DMA (`SPI_DMA_CH_AUTO`), mais le DMA ne sert qu'à transférer buffer→SPI FIFO. Le CPU **busy-loop** pendant toute la durée du transfert via `spi_device_polling_start/end`. C'est pire qu'un simple `spi_device_transmit()` qui au moins utilise des interrupts.

**Calcul de temps perdu :**
```
Par chunk : 2048 octets / 20 MB/s = 102.4 μs (transfer SPI)
           + ~20 μs (byte-swap 1024 px) + ~5 μs (overhead SPI transaction)
           ≈ 127 μs par chunk

Par flush LVGL : 15 chunks × 127 μs ≈ 1.9 ms   (360×40 pixels)
Par frame complet : 9 flushes × 1.9 ms ≈ 17.1 ms  (360×360 pixels)
```

→ **17ms/frame** de CPU mort en polling. À 30 FPS = **51% du temps CPU de Core 1 perdu en attente**.

#### Solutions proposées (4 options)

**Option A — Background flush task sur Core 0 (FACILE)**
```cpp
static QueueHandle_t flushQueue;
struct FlushCmd { lv_disp_drv_t *drv; int16_t x, y; uint16_t w, h; uint16_t *buf; };

void flushTask(void *param) {
    FlushCmd cmd;
    while (true) {
        if (xQueueReceive(flushQueue, &cmd, portMAX_DELAY)) {
            gfx->draw16bitRGBBitmap(cmd.x, cmd.y, cmd.buf, cmd.w, cmd.h);
            lv_disp_flush_ready(cmd.drv);
        }
    }
}

void lvFlushCb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *buf) {
    FlushCmd cmd = { drv, area->x1, area->y1, 
                     (uint16_t)(area->x2-area->x1+1), (uint16_t)(area->y2-area->y1+1),
                     (uint16_t*)buf };
    xQueueSend(flushQueue, &cmd, portMAX_DELAY);
    // PAS de lv_disp_flush_ready — sera appelé par flushTask quand SPI terminé
}

// setup()
flushQueue = xQueueCreate(1, sizeof(FlushCmd));
xTaskCreatePinnedToCore(flushTask, "flush", 4096, NULL, 3, NULL, 0);  // Core 0
```

- **Avantage** : Aucune modification de lib. Core 1 est libéré pendant le flush.
- **Inconvénient** : Core 0 bloque pendant le polling. Le double-buffer LVGL est utilisé car `lvFlushCb` retourne immédiatement.
- **Gain** : ~17ms/frame libérés sur Core 1. FPS dashboard devrait doubler.

**Option B — Patcher writePixels pour pipeline les chunks (MOYEN)**

Le driver a déjà `_buffer` et `_2nd_buffer` mais ne les utilise pas en pipeline. On pourrait modifier `Arduino_ESP32QSPI::writePixels` pour :
1. Byte-swap chunk N dans `_buffer`
2. Start polling transfer de `_buffer`  
3. Pendant le transfer : byte-swap chunk N+1 dans `_2nd_buffer`
4. End polling, swap pointers, repeat

```cpp
// Modification dans Arduino_ESP32QSPI.cpp :: writePixels
// Overlap byte-swap avec transfer SPI
for (chunk = 0; chunk < totalChunks; chunk++) {
    uint8_t *currentBuf = (chunk & 1) ? _2nd_buffer : _buffer;
    // Byte-swap dans currentBuf
    byteSwapChunk(data, currentBuf, chunkLen);
    data += chunkLen;
    
    if (chunk > 0) POLL_END();  // attendre fin du chunk précédent
    
    _spi_tran_ext.base.tx_buffer = currentBuf;
    _spi_tran_ext.base.length = chunkLen << 3;
    POLL_START();              // lancer le chunk courant
}
POLL_END();  // attendre le dernier chunk
```

- **Avantage** : ~30% gain sur le temps de flush (byte-swap masqué par transfer)
- **Inconvénient** : Modif dans `.pio/libdeps` (écrasée au clean). Doit être forkée.

**Option C — Async SPI via `spi_device_queue_trans` + `post_cb` (MOYEN-DUR)**

Modifier `Arduino_ESP32QSPI` pour supporter un mode asynchrone :
1. Changer `queue_size` de 1 à 2
2. Ajouter `post_cb` dans la config du device SPI
3. Ajouter une méthode `writePixelsAsync(data, len, callback)`
4. Le callback ISR appelle `lv_disp_flush_ready()`

```cpp
// Modification dans Arduino_ESP32QSPI (fork nécessaire)
static void IRAM_ATTR spiPostCb(spi_transaction_t *trans) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(spiDoneSem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
}

// Pour le flush LVGL : envoyer TOUT le buffer en une seule transaction
// au lieu de 15 chunks de 1024 px
void writePixelsAsync(uint16_t *data, uint32_t len) {
    // Byte-swap le buffer entier (LVGL buffer, pas le _buffer interne)
    byteSwapInPlace(data, len);
    
    _spi_tran_ext.base.tx_buffer = data;
    _spi_tran_ext.base.length = len << 4;  // bits
    spi_device_queue_trans(_handle, _spi_tran, portMAX_DELAY);
    // Retour immédiat — ISR signalera la fin
}
```

- **Avantage** : CPU véritablement libéré pendant le transfer. Meilleur gain possible.
- **Inconvénient** : Fork Arduino_GFX nécessaire. `_handle` privé. `max_transfer_sz` doit être augmenté. Buffer LVGL doit être en mémoire DMA-capable (SRAM interne, cf #2).
- **Prérequis** : #2 (buffers en SRAM) est obligatoire, car le DMA SPI sur ESP32-S3 fonctionne mieux avec la SRAM interne.

**Option D — Remplacer par `esp_lcd_panel_io` (DUR)**

Utiliser le driver natif ESP-IDF qui gère DMA + async nativement :
```cpp
esp_lcd_panel_io_handle_t io_handle;
esp_lcd_panel_io_spi_config_t io_config = {
    .dc_gpio_num = -1,  // not used in QSPI
    .cs_gpio_num = 10,
    .pclk_hz = 40000000,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
    .spi_mode = 0,
    .trans_queue_depth = 10,  // DMA pipeline depth
    .on_color_trans_done = lvgl_flush_ready_cb,  // async callback
    .flags = { .quad_mode = true },
};
esp_lcd_new_panel_io_spi(spi_bus, &io_config, &io_handle);
```

- **Avantage** : Architecture de référence Espressif. DMA pipeline optimal.
- **Inconvénient** : Réécriture de toute l'initialisation display + perte de Arduino_GFX pour le mode caméra.

#### Recommandation

**Commencer par Option A** (background flush task) — gain immédiat, 0 modification de lib, exploite Core 0.  
**Ensuite évaluer Option C** (async SPI) si le gain d'Option A est insuffisant.

---

### 2. Draw buffers LVGL → SRAM interne

**Gain : ★★★★☆ | Complexité : ★☆☆☆☆**

**Problème** : `lvBuf1` et `lvBuf2` alloués en PSRAM (`MALLOC_CAP_SPIRAM`). La PSRAM OPI est **3-4× plus lente** que la SRAM interne pour les accès aléatoires. LVGL fait beaucoup d'accès aléatoires pendant le rendu (pixels, blending, anti-aliasing).

**Solution** (2 lignes) :
```cpp
// AVANT
lvBuf1 = (lv_color_t *)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
lvBuf2 = (lv_color_t *)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);

// APRÈS
lvBuf1 = (lv_color_t *)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
lvBuf2 = (lv_color_t *)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
```

**Mémoire** : 360 × 40 × 2 = 28,800 octets × 2 buffers = **57,600 octets** en SRAM interne. Le S3 a ~512KB, c'est faisable.

**Bonus** : Prérequis pour le DMA (#1) — le DMA SPI nécessite des buffers en mémoire DMA-capable.

---

### 3. LVGL render task sur Core 0

**Gain : ★★★★★ | Complexité : ★★★★☆**

**Problème** : Tout le rendu LVGL (render + flush) tourne sur Core 1 via `lv_timer_handler()` dans `loop()`.

**Solution** : Créer une tâche dédiée LVGL sur Core 0 :
```cpp
void lvglTask(void *param) {
    while (true) {
        xSemaphoreTake(lvglMutex, portMAX_DELAY);
        lv_timer_handler();
        xSemaphoreGive(lvglMutex);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
// setup()
xTaskCreatePinnedToCore(lvglTask, "lvgl", 8192, NULL, 3, NULL, 0);
```

**Attention** : LVGL n'est **pas thread-safe**. Toute modification d'objets LVGL (depuis `updateDashboard` sur Core 1) doit prendre le même mutex :
```cpp
xSemaphoreTake(lvglMutex, portMAX_DELAY);
updateDashboard();  // modifie labels, arcs, etc.
xSemaphoreGive(lvglMutex);
```

**Architecture résultante** :
```
Core 0: lvglTask (render + flush) + WiFi/ESP-NOW + UDP recv
Core 1: loop() (updateDashboard + touch + camera display)
```

**Risque** : Contention sur le mutex si updateDashboard est lent. Combiné avec dirty flags (#5), ce risque est mitigé.

---

### 4. JPEG decode sur Core 0

**Gain : ★★★★☆ | Complexité : ★★☆☆☆**

**Problème** : `decodeAndDisplay()` fait le décodage JPEG **et** le push display sur Core 1. Le décodage JPEG est CPU-intensif (~5-15ms par frame selon la résolution).

**Solution** : Séparer decode (Core 0) et display (Core 1) :
```cpp
// Core 0 : décode dans screenBuf
void jpegDecodeTask(void *param) {
    while (true) {
        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50))) {
            // Decode JPEG → screenBuf
            jpeg.openRAM(jpegBuf[decodeIdx], decodeLen, jpegDrawCallback);
            jpeg.decode(0, 0, 0);
            jpeg.close();
            decodeReady = true;
        }
    }
}

// Core 1 : loop() push display quand ready
if (decodeReady) {
    gfx->draw16bitRGBBitmap(0, 0, screenBuf, SCREEN_W, SCREEN_H);
    decodeReady = false;
}
```

**Gain** : Core 1 ne fait plus que le push display (~2ms), libérant ~10ms pour d'autres tâches. Le décodage utilise le temps CPU libre de Core 0.

---

### 5. Dirty flags updateDashboard

**Gain : ★★★☆☆ | Complexité : ★★☆☆☆**

**Problème** : `updateDashboard()` fait `snprintf` + `lv_label_set_text` pour **tous** les champs à **chaque** appel (chaque itération de loop), même si les valeurs n'ont pas changé. Calcule aussi la DST (Zeller) à chaque appel.

**Solution** : Flags de modification dans le callback ESP-NOW :
```cpp
struct CanData {
    volatile int speed;
    volatile int soc;
    // ...
    volatile uint32_t dirtyFlags;  // bitfield
};

#define DIRTY_SPEED   (1 << 0)
#define DIRTY_SOC     (1 << 1)
#define DIRTY_RANGE   (1 << 2)
// ...

// ESP-NOW callback
canData.speed = newSpeed;
canData.dirtyFlags |= DIRTY_SPEED;

// updateDashboard
uint32_t flags = canData.dirtyFlags;
canData.dirtyFlags = 0;

if (flags & DIRTY_SPEED) {
    snprintf(buf, sizeof(buf), "%d", canData.speed);
    lv_label_set_text(lblSpeed, buf);
}
```

**Quand rien ne change** (voiture garée), `updateDashboard()` ne fait presque rien au lieu de ~15 `snprintf` + label updates.

---

### 6. recolorCanvas — skip si même couleur

**Gain : ★★★☆☆ | Complexité : ★☆☆☆☆**

**Problème** : `recolorCanvas()` itère **129,600 pixels × 3 bytes = 388,800 octets** en PSRAM pour changer la couleur du halo. Appelé à chaque cycle de blink (toutes les 500ms) depuis `updateDashboard()`.

**Solution** (3 lignes) :
```cpp
void recolorCanvas(lv_color_t *cbuf, const uint8_t *mask, lv_color_t color, int pixels) {
    static lv_color_t lastColor = {0};
    if (color.full == lastColor.full) return;  // ← skip si même couleur
    lastColor = color;
    // ... reste du code
}
```

**Note** : Si les 3 canvas (left, right, hazard) ont des couleurs différentes, il faut un `lastColor` par canvas.

---

### 7. Précalculer les ticks

**Gain : ★★☆☆☆ | Complexité : ★☆☆☆☆**

**Problème** : `drawTicksCb()` est un event callback LVGL appelé à **chaque redraw** de l'arc. Il calcule `sinf()` + `cosf()` pour **37 ticks** à chaque fois. Ces positions sont statiques.

**Solution** :
```cpp
struct TickPos { int16_t x1, y1, x2, y2; };
static TickPos tickCache[37];
static bool ticksCached = false;

void drawTicksCb(lv_event_t *e) {
    if (!ticksCached) {
        for (int i = 0; i < 37; i++) {
            float angle = ...;
            tickCache[i] = { cosf(angle)*r1, sinf(angle)*r1,
                             cosf(angle)*r2, sinf(angle)*r2 };
        }
        ticksCached = true;
    }
    // Dessiner depuis tickCache[]
}
```

---

### 8. Int math dans ESP-NOW callback

**Gain : ★★☆☆☆ | Complexité : ★★☆☆☆**

**Problème** : `onEspNowRecv()` s'exécute sur le WiFi task (Core 0). Contient des float pour les conversions temp/range :
```cpp
canData.cabinTemp = rawVal * 0.1f - 40.0f;  // float multiply + subtract
canData.rangeKm = raw * 0.1f;               // float multiply
```

**Solution** : Stocker les valeurs brutes en int, convertir dans `updateDashboard()` :
```cpp
// ESP-NOW callback (rapide, int seulement)
canData.cabinTempRaw = rawVal;  // int16_t

// updateDashboard (Core 1, float OK)
float temp = canData.cabinTempRaw * 0.1f - 40.0f;
```

**Gain** : ~1-2μs par callback. Faible en absolu, mais réduit la latence WiFi et évite la contention FPU entre cores (le S3 a un seul FPU partagé — attention !).

> **Note ESP32-S3** : Le S3 a en fait **deux FPU** (un par core), contrairement au S2. Le gain est donc surtout en temps de callback, pas en contention FPU.

---

### 9. CAN data processing task Core 0

**Gain : ★★☆☆☆ | Complexité : ★★★☆☆**

**Problème** : Le callback ESP-NOW fait du parsing CAN (bit extraction, conversion) sur le WiFi task, ce qui augmente la latence WiFi.

**Solution** : Le callback ESP-NOW enqueue les données brutes dans un ring buffer. Un task dédié sur Core 0 fait le parsing :
```cpp
// ESP-NOW callback (ultra rapide)
void onEspNowRecv(...) {
    RawCanMsg msg = { canId, dataLen, data };
    xQueueSendFromISR(canQueue, &msg, NULL);
}

// Core 0 : task de parsing
void canProcessTask(void *param) {
    RawCanMsg msg;
    while (true) {
        if (xQueueReceive(canQueue, &msg, pdMS_TO_TICKS(100))) {
            parseCanMessage(msg);  // bit extraction, conversion
        }
    }
}
```

**Avantage** : Le callback ESP-NOW prend <1μs au lieu de 10-50μs. Meilleure réactivité WiFi/ESP-NOW.

---

### 10. Augmenter LV_BUF_LINES

**Gain : ★★☆☆☆ | Complexité : ★☆☆☆☆**

**Problème** : `LV_BUF_LINES=40` → 9 flush/frame. Chaque flush a un overhead fixe (setup SPI window, etc.).

**Solution** : Passer à 120 lignes (3 flush/frame) ou 180 (2 flush/frame) :
```cpp
#define LV_BUF_LINES 120  // 360×120×2 = 86,400 octets × 2 = 172,800
```

**Compromis** : Plus de mémoire utilisée. Si en SRAM interne (#2), 172,800 octets reste faisable (<35% de la SRAM).

**Avec DMA** : Moins critique car l'overhead par flush est réduit. Mais toujours bénéfique car moins de context switches.

---

### 11. Rate-limit dashboard loop

**Gain : ★★☆☆☆ | Complexité : ★☆☆☆☆**

**Problème** : En mode dashboard, `loop()` appelle `updateDashboard()` + `lv_timer_handler()` en boucle serrée. Core 1 tourne à 100% même quand rien ne change.

**Solution** (1 ligne) :
```cpp
// Dans le else (dashboard mode) de loop()
updateDashboard();
uint32_t nextMs = lv_timer_handler();  // retourne le temps avant le prochain timer
vTaskDelay(pdMS_TO_TICKS(min(nextMs, (uint32_t)16)));  // cap à ~60Hz
```

**Gain** : Réduit la charge CPU de Core 1 de ~100% à ~30-50%. Libère du temps pour les ISR, WiFi, et réduit la consommation.

---

### 12. Mutex canData

**Gain : ★☆☆☆☆ | Complexité : ★☆☆☆☆**

**Problème** : `canData` est `volatile` mais sans mutex. Les lectures multi-champs (ex: `soc` + `rangeKm` + `energyAtDest` pour le calcul SoC dest) ne sont pas atomiques.

**Solution** :
```cpp
portMUX_TYPE canMux = portMUX_INITIALIZER_UNLOCKED;

// ESP-NOW callback
taskENTER_CRITICAL(&canMux);
canData.soc = newSoc;
canData.rangeKm = newRange;
taskEXIT_CRITICAL(&canMux);

// updateDashboard - copie locale
CanData local;
taskENTER_CRITICAL(&canMux);
local = canData;
taskEXIT_CRITICAL(&canMux);
```

**Note** : `taskENTER_CRITICAL` sur ESP32 est un spinlock (~100ns). Négligeable en performance, mais élimine les lectures incohérentes.

---

### 13. Éliminer double copie caméra

**Gain : ★★★☆☆ | Complexité : ★★★★☆**

**Problème** : En mode caméra, le flux est : JPEG decode → `screenBuf` (PSRAM, 259,200 octets) → `draw16bitRGBBitmap` (SPI bloquant). Double copie.

**Solution** : Envoyer les MCU blocks directement au display depuis le callback JPEG :
```cpp
static int jpegDrawCallback(JPEGDRAW *pDraw) {
    // Direct push au display au lieu de copier dans screenBuf
    gfx->draw16bitRGBBitmap(pDraw->x, pDraw->y, 
                             pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
    return 1;
}
```

**Problème** : Nécessite que `setAddrWindow` soit rapide et que l'écran supporte des fenêtres arbitraires (le ST77916 le supporte). Peut causer du tearing visible si les MCU blocks ne sont pas en ordre.

---

### 14. DST pré-calculé

**Gain : ★☆☆☆☆ | Complexité : ★☆☆☆☆**

**Problème** : La formule de Zeller pour le calcul DST (heure d'été) est exécutée à **chaque** appel de `updateDashboard()`.

**Solution** : Ne recalculer que quand l'heure (minute) change :
```cpp
static int lastMinute = -1;
if (canData.utcMinute != lastMinute) {
    lastMinute = canData.utcMinute;
    // Recalculer DST, formater l'heure
}
```

---

### 15. Pipeline caméra dual-core

**Gain : ★★★★☆ | Complexité : ★★★☆☆**

**Problème** : En mode caméra, Core 1 fait séquentiellement : grab frame → decode JPEG → push display. Le pipeline n'est pas parallélisé.

**Solution** : Pipeline 3 étages avec les 2 cores :
```
Core 0: [UDP recv] → [JPEG decode frame N] → [JPEG decode frame N+1] → ...
Core 1: [idle]     → [display push frame N] → [display push frame N+1] → ...
                         ↑ notification task         ↑ notification task
```

**Implémentation** :
```cpp
// Core 0 : udpRecvTask reçoit + assemble (déjà fait)
// Core 0 : jpegDecodeTask décode quand un frame est ready
void jpegDecodeTask(void *param) {
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        int idx = grabReadyFrame(&len);
        if (idx >= 0) {
            decodeJpegToScreenBuf(idx, len);  // CPU-intensive, sur Core 0
            xTaskNotify(displayTaskHandle, 0, eNoAction);  // signal Core 1
        }
    }
}

// Core 1 : displayTask push quand decode est fini
void loop() {
    if (ulTaskNotifyTake(pdFALSE, 0)) {
        gfx->draw16bitRGBBitmap(0, 0, screenBuf, SCREEN_W, SCREEN_H);
    }
}
```

**Gain** : Decode (5-15ms) et display push (2-3ms) se chevauchent. FPS caméra augmente de ~30-50%.

---

## Plan d'exécution recommandé

### Phase 1 — Quick wins (effort faible, gain immédiat)
1. **#2** — Draw buffers → SRAM interne (2 lignes)
2. **#6** — recolorCanvas skip (3 lignes)
3. **#7** — Précalculer ticks (cache statique)
4. **#11** — Rate-limit loop (1 ligne)
5. **#14** — DST pré-calculé (guard minute)
6. **#10** — Augmenter LV_BUF_LINES (1 ligne)

### Phase 2 — Dual-core exploitation
7. **#4** — JPEG decode sur Core 0 (nouvelle tâche)
8. **#5** — Dirty flags updateDashboard (refactoring modéré)
9. **#12** — Mutex canData (spinlock)

### Phase 3 — DMA pipeline
10. **#1** — DMA flush SPI (investigation Arduino_GFX + implémentation)
11. **#15** — Pipeline caméra dual-core (avec #4)

### Phase 4 — Architecture avancée (optionnel)
12. **#3** — LVGL render task Core 0 (seulement si #1 ne suffit pas)
13. **#9** — CAN processing task Core 0
14. **#8** — Int math ESP-NOW
15. **#13** — Éliminer double copie caméra

---

## Métriques à mesurer

Pour valider chaque optimisation, instrumenter avec :
```cpp
unsigned long t0 = micros();
// ... code à mesurer ...
unsigned long dt = micros() - t0;
Serial.printf("[PERF] operation: %lu µs\n", dt);
```

Indicateurs clés :
- **FPS dashboard** : `lv_timer_handler()` temps total par frame
- **FPS caméra** : frames décodées + affichées par seconde (déjà mesuré)
- **CPU idle** : `vTaskGetRunTimeStats()` ou `esp_timer_get_time()` dans idle hook
- **Flush time** : temps dans `lvFlushCb` (avant/après DMA)
- **PSRAM vs SRAM** : benchmark LVGL avec `LV_USE_PERF_MONITOR 1`
