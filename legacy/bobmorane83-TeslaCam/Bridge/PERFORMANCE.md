# Optimisations Performance — ESP32-S3 Bridge

> Analyse du firmware `main.cpp` (554 lignes) sur LilyGo T-2CAN  
> ESP32-S3 dual-core 240 MHz, 512 KB SRAM, 8 MB PSRAM OPI, 16 MB Flash  
> Dual CAN : MCP2515 (SPI, VehicleBus) + TWAI (built-in, ChassisBus)

---

## État actuel des cores

```
┌──────────────── CORE 0 (PRO_CPU) ──────┬──────────────── CORE 1 (APP_CPU) ──────┐
│ WiFi stack (système, prio 23)          │ canRxTask (prio 5) — polling dédié     │
│ espnowTxTask (prio 3)                 │ ├─ checkCANBus() — MCP2515 SPI read    │
│ ├─ xQueueReceive (bloquant)           │ ├─ checkTWAIBus() — TWAI receive       │
│ ├─ Rate limiter                       │ ├─ isValidCanId() — binary search ×2   │
│ ├─ transmitViaESPNOW()                │ └─ queueCANMessage() — memcpy + enqueue│
│ └─ sendHeartbeat()                    │ vTaskDelay(1) — yield WDT              │
│ loop() — stats seulement (prio 1)     │                                        │
│ ≈ 15-30% utilisé                       │ ≈ 20-40% utilisé (selon trafic CAN)    │
└────────────────────────────────────────┴────────────────────────────────────────┘
```

**Constat** : Les deux cores sont globalement sous-utilisés. Le bottleneck n'est pas le CPU mais la **latence** (polling MCP2515, ESP-NOW frame-par-frame) et la **fiabilité** (perte de frames, pas de priorité).

---

## Flux de données actuel

```
Tesla VehicleBus (500 kbps)          Tesla ChassisBus (500 kbps)
        │                                     │
   MCP2515 (SPI)                         TWAI (built-in)
   2 HW RX buffers                       64-deep RX queue
   HW filter: 0x100-0x5FF,0x700+        Accept-all filter
        │                                     │
        └──── checkCANBus() ──┬── checkTWAIBus() ────┘
                              │
                     isValidCanId()        ← binary search (136+25 IDs)
                              │
                     queueCANMessage()     ← memcpy 8B + xQueueSend
                              │
                   FreeRTOS Queue (256 slots, 18B/msg = 4.6 KB)
                              │
                     espnowTxTask()        ← Core 0
                     ├─ rate limiter       ← 200ms par CAN ID
                     ├─ priority bypass    ← 12× comparaisons ||
                     └─ transmitViaESPNOW  ← 1 CAN msg = 1 ESP-NOW frame (18B)
                              │
                     ESP-NOW broadcast (250B MTU, WiFi ch.1)
                              │
                         Ecran (Dash)
```

---

## Tableau récapitulatif

| # | Optimisation | Gain perf | Complexité | Ratio | Catégorie |
|---|-------------|-----------|------------|-------|-----------|
| 1 | [ESP-NOW Batching (multi-msg/frame)](#1-esp-now-batching) | ★★★★★ | ★★★☆☆ | 1.67 | Latence + débit |
| 2 | [File de priorité séparée](#2-file-de-priorité-séparée) | ★★★★☆ | ★★☆☆☆ | 2.00 | Fiabilité |
| 3 | [Polling MCP2515 — réduire vTaskDelay](#3-polling-mcp2515--réduire-vtaskdelay) | ★★★★☆ | ★☆☆☆☆ | 4.00 | Latence |
| 4 | [isValidCanId — bitfield O(1)](#4-isvalidcanid--bitfield-o1) | ★★★☆☆ | ★☆☆☆☆ | 3.00 | CPU |
| 5 | [TWAI filtre hardware](#5-twai-filtre-hardware) | ★★★☆☆ | ★★☆☆☆ | 1.50 | CPU + queue |
| 6 | [Priorité IDs — lookup table](#6-priorité-ids--lookup-table) | ★★☆☆☆ | ★☆☆☆☆ | 2.00 | CPU |
| 7 | [ESP-NOW unicast vs broadcast](#7-esp-now-unicast-vs-broadcast) | ★★☆☆☆ | ★☆☆☆☆ | 2.00 | Fiabilité |
| 8 | [MCP2515 SPI clock → 10 MHz](#8-mcp2515-spi-clock--10-mhz) | ★★☆☆☆ | ★☆☆☆☆ | 2.00 | Latence |
| 9 | [HW filter — IDs manquants 0x0xx, 0x6xx](#9-hw-filter--ids-manquants) | ★★☆☆☆ | ★★☆☆☆ | 1.00 | Couverture |
| 10 | [TWAI error recovery (bus-off)](#10-twai-error-recovery) | ★☆☆☆☆ | ★☆☆☆☆ | 1.00 | Robustesse |
| 11 | [memcpy simplifié dans queueCANMessage](#11-memcpy-simplifié) | ★☆☆☆☆ | ★☆☆☆☆ | 1.00 | Micro |

> **Ratio** = Gain / Complexité (plus c'est haut, meilleur est le rapport effort/résultat)  
> Classement recommandé : par ratio décroissant → #3, #4, #6, #8, #7, #2, #1, #5, …

---

## Détails par optimisation

### 1. ESP-NOW Batching

**Gain : ★★★★★ | Complexité : ★★★☆☆**

#### Problème

Chaque CAN message (18 octets) est envoyé dans un ESP-NOW frame séparé. Le MTU ESP-NOW est de 250 octets → on pourrait envoyer **13 messages par frame**.

```
Actuel :  1 CAN msg → 1 ESP-NOW frame
          500 msg/s → 500 ESP-NOW frames/s
          
Overhead par frame ESP-NOW :
  - CSMA/CA backoff : ~200-500 μs (aléatoire)
  - Préambule WiFi  : ~20 μs
  - Header 802.11   : ~40 μs
  - ACK wait (broadcast = pas d'ACK) : ~0 μs
  - Total overhead   : ~300-600 μs par frame
  
Temps radio actuel : 500 × 400 μs ≈ 200 ms/s = 20% du temps radio
```

#### Solution : Buffer + flush périodique

```cpp
#define BATCH_MAX_MSGS     10           // Max messages par frame ESP-NOW
#define BATCH_FLUSH_MS     5            // Flush toutes les 5ms max
#define BATCH_BUF_SIZE     (sizeof(ESP_CAN_Message_t) * BATCH_MAX_MSGS) // 180B < 250B

static ESP_CAN_Message_t batchBuf[BATCH_MAX_MSGS];
static uint8_t batchCount = 0;
static unsigned long lastFlush = 0;

void flushBatch() {
    if (batchCount == 0) return;
    uint8_t broadcast_addr[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    // Envoyer le batch (header: 1 byte count + N messages)
    uint8_t txBuf[1 + BATCH_BUF_SIZE];
    txBuf[0] = batchCount;
    memcpy(txBuf + 1, batchBuf, batchCount * sizeof(ESP_CAN_Message_t));
    esp_now_send(broadcast_addr, txBuf, 1 + batchCount * sizeof(ESP_CAN_Message_t));
    stats_espnow_tx += batchCount;
    batchCount = 0;
    lastFlush = millis();
}

// Dans espnowTxTask, remplacer transmitViaESPNOW par :
batchBuf[batchCount++] = msg;
if (batchCount >= BATCH_MAX_MSGS || priority) {
    flushBatch();  // Flush immédiat si batch plein ou message prioritaire
}
// + vérifier timeout flush dans la boucle
if (millis() - lastFlush >= BATCH_FLUSH_MS) {
    flushBatch();
}
```

**Impact côté Ecran** : La callback `onCANReceive` doit être modifiée pour déballer le batch :
```cpp
void onCANReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len < 1) return;
    uint8_t count = data[0];
    const ESP_CAN_Message_t *msgs = (const ESP_CAN_Message_t *)(data + 1);
    for (uint8_t i = 0; i < count && (1 + (i+1)*sizeof(ESP_CAN_Message_t)) <= len; i++) {
        processCANMessage(&msgs[i]);  // Traiter chaque message du batch
    }
}
```

**Gains attendus :**
- Temps radio : 500 frames/s → ~50 frames/s (10× moins d'overhead WiFi)
- Latence max : +5ms (flush timeout) — acceptable pour dashboard 30 FPS
- Messages prioritaires : flush immédiat → latence ~0 pour turn signals, blindspot

---

### 2. File de priorité séparée

**Gain : ★★★★☆ | Complexité : ★★☆☆☆**

#### Problème

Une seule FIFO queue (256 slots). Si elle est pleine de messages basse-prio (0x401 BrickVoltages à ~1248 msg/s), un message priority (turn signal 0x3F5) est **droppé silencieusement** (`stats_queue_full++`).

```
Scénario de perte :
  0x401 arrive à ~1248/s, rate-limited à 5/s → 1243/s passent par la queue
  Queue se remplit en 256/1243 = 206 ms si TX bloqué
  Turn signal 0x3F5 arrive → queue pleine → PERDU
```

#### Solution : Deux queues

```cpp
#define PRIO_QUEUE_SIZE  32    // Petite queue, jamais pleine
#define NORM_QUEUE_SIZE  256   // Queue normale

QueueHandle_t prio_queue = NULL;
QueueHandle_t norm_queue = NULL;

void queueCANMessage(uint32_t id, uint8_t dlc, uint8_t* data) {
    if (!isValidCanId(id)) { stats_errors++; return; }
    
    ESP_CAN_Message_t msg;
    msg.can_id = id;
    msg.dlc = dlc;
    msg.timestamp = millis();
    msg.is_extended = 0;
    memcpy(msg.data, data, 8);
    
    bool isPrio = isPriorityId(id);  // Voir #6
    QueueHandle_t q = isPrio ? prio_queue : norm_queue;
    
    if (xQueueSend(q, &msg, 0) != pdTRUE) {
        stats_queue_full++;
    }
}

// Dans espnowTxTask :
ESP_CAN_Message_t msg;
// Toujours drainer la priority queue en premier
if (xQueueReceive(prio_queue, &msg, 0) == pdTRUE) {
    transmitViaESPNOW(&msg);  // Pas de rate limiting
} else if (xQueueReceive(norm_queue, &msg, pdMS_TO_TICKS(100)) == pdTRUE) {
    // Rate limiting normal
    // ...
}
```

**Gain** : Les messages critiques (turn signals, blindspot, brake) ne sont jamais droppés.

---

### 3. Polling MCP2515 — réduire vTaskDelay

**Gain : ★★★★☆ | Complexité : ★☆☆☆☆**

#### Problème

```cpp
void canRxTask(void* param) {
    while (1) {
        checkCANBus();   // Drain MCP2515
        checkTWAIBus();  // Drain TWAI
        vTaskDelay(1);   // ⚠️ 1 tick = 1ms (si CONFIG_FREERTOS_HZ=1000)
    }
}
```

Le MCP2515 n'a que **2 RX buffers**. À 500 kbps :

```
Frame CAN (8 bytes data) = ~130 bits (avec stuff bits)
Durée par frame = 130 / 500000 = 260 μs

En 1ms de vTaskDelay → ~3.8 frames peuvent arriver
MCP2515 a 2 RX buffers + 1 MAB = 3 frames max
→ Overflow possible si ≥4 frames arrivent pendant le delay
```

Sur le VehicleBus Tesla avec certaines bursts (0x401 BrickVoltages à ~1248/s = 1 frame/800μs), le risque d'overflow est réel.

#### Solution

```cpp
void canRxTask(void* param) {
    while (1) {
        checkCANBus();
        checkTWAIBus();
        
        // Option A : yield sans délai (plus agressif, ~10000 polls/s)
        taskYIELD();
        esp_task_wdt_reset();  // Reset WDT manuellement
        
        // Option B : délai minimal avec WDT safe (recommandé)
        // vTaskDelay(1) est OK si CONFIG_FREERTOS_HZ=1000
        // Mais vérifier que c'est bien 1ms et pas 10ms (si HZ=100)
    }
}
```

**Vérification recommandée** : Ajouter dans `setup()` :
```cpp
Serial.printf("[TICK] CONFIG_FREERTOS_HZ = %d (1 tick = %d ms)\n",
              configTICK_RATE_HZ, portTICK_PERIOD_MS);
```

Si `portTICK_PERIOD_MS = 10` (tick = 10ms), c'est un **bug critique** — chaque poll laisse passer ~38 frames avec seulement 3 buffers.

**Gain** : Élimine le risque de perte de frames MCP2515 par overflow.

---

### 4. isValidCanId — bitfield O(1)

**Gain : ★★★☆☆ | Complexité : ★☆☆☆☆**

#### Problème

```cpp
bool isValidCanId(uint32_t id) {
    uint16_t id16 = (uint16_t)(id & 0x7FF);
    return bsearch16(VEHICLE_BUS_IDS, VEHICLE_BUS_IDS_COUNT, id16)   // O(log₂ 136) ≈ 8
        || bsearch16(CHASSIS_BUS_IDS, CHASSIS_BUS_IDS_COUNT, id16);  // O(log₂ 25)  ≈ 5
}
```

13 comparaisons × 2000+ msg/s = ~26 000 branches/s. Non critique mais facilement éliminable.

#### Solution : Bitfield 256 octets

```cpp
// valid_can_ids.h — ajouter à la fin :
// Bitfield lookup : 2048 IDs / 8 = 256 bytes
static uint8_t CAN_ID_BITFIELD[256];
static bool    CAN_ID_PRIORITY[2048];  // Voir #6

static inline void initCanIdBitfield() {
    memset(CAN_ID_BITFIELD, 0, sizeof(CAN_ID_BITFIELD));
    for (uint16_t i = 0; i < VEHICLE_BUS_IDS_COUNT; i++) {
        CAN_ID_BITFIELD[VEHICLE_BUS_IDS[i] >> 3] |= (1 << (VEHICLE_BUS_IDS[i] & 7));
    }
    for (uint16_t i = 0; i < CHASSIS_BUS_IDS_COUNT; i++) {
        CAN_ID_BITFIELD[CHASSIS_BUS_IDS[i] >> 3] |= (1 << (CHASSIS_BUS_IDS[i] & 7));
    }
}

static inline bool isValidCanIdFast(uint32_t id) {
    uint16_t id16 = id & 0x7FF;
    return CAN_ID_BITFIELD[id16 >> 3] & (1 << (id16 & 7));
}
```

**Gain** : 13 comparaisons → 1 load + 1 AND + 1 shift. Constant time, cache-friendly (256 bytes tient dans L1).

---

### 5. TWAI filtre hardware

**Gain : ★★★☆☆ | Complexité : ★★☆☆☆**

#### Problème

```cpp
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
```

TWAI accepte **tous** les frames CAN du ChassisBus. La RX queue (64 slots) se remplit de messages non désirés → overflow + software filter inutile.

Le ChassisBus whitelist (25 IDs) va de 0x045 à 0x3FD. Distribution :

```
0x045, 0x04F                    → 0x0xx  (2 IDs)
0x101, 0x111, 0x116, 0x145,     → 0x1xx  (6 IDs)
0x155, 0x175, 0x185
0x20E, 0x219, 0x238, 0x239,     → 0x2xx  (6 IDs)
0x24A, 0x25B, 0x2B9, 0x2D3
0x31F, 0x389, 0x399, 0x39D,     → 0x3xx  (7 IDs)
0x3D9, 0x3F3, 0x3F8, 0x3FD
```

#### Solution : Filtre par plage haute

Le TWAI n'a qu'un seul filtre (acceptance code + mask). On ne peut pas matcher 25 IDs exactement. Mais on peut éliminer les plages inutiles :

```cpp
// Accepter 0x000-0x3FF (bits 10:9 = 00 ou 01) en masquant le bit 10
// Cela bloque 0x400-0x7FF (50% du traffic CAN potentiel)
twai_filter_config_t f_config = {
    .acceptance_code = 0x00000000,  // ID = 0x000
    .acceptance_mask = 0xF8000000 | (0x400 << 21),  // Masquer bit 10 → accepter 0x000-0x3FF
    .single_filter = true
};

// Plus agressif : accepter 0x000-0x3FF seulement
// Pour 11-bit standard IDs en single filter mode :
// acceptance_code = ID << 21 (bits 31:21 du registre)
// acceptance_mask = bits à ignorer << 21
```

⚠️ Le calcul exact du filtre TWAI ESP-IDF est délicat. Format single filter :
```
Bits 31:21 = ID[10:0]
Bit 20     = RTR
Bits 19:16 = unused (standard)
```

```cpp
// Accepter tout de 0x000 à 0x3FF (masquer bits 9:0, accepter bit 10 = 0)
twai_filter_config_t f_config = {
    .acceptance_code = 0x00000000,            // bit 10 = 0
    .acceptance_mask = ~(1UL << 31) | 0x001FF800, // ne vérifier que bit 10
    // Simplifié : mask = 0x7FFFFFFF sauf bit 31
    .single_filter = true
};
```

**Gain** : Réduit de ~50% le trafic hardware TWAI → moins d'overflows, moins de CPU pour le filtre software.

> Note : Le filtre exact dépend du trafic réel du ChassisBus. Mesurer d'abord avec `TWAI_FILTER_CONFIG_ACCEPT_ALL` + compteur d'IDs invalides pour quantifier le gain.

---

### 6. Priorité IDs — lookup table

**Gain : ★★☆☆☆ | Complexité : ★☆☆☆☆**

#### Problème

```cpp
bool priority = (msg.can_id == 0x257 || msg.can_id == 0x266 ||
                 msg.can_id == 0x2E5 || msg.can_id == 0x33A ||
                 msg.can_id == 0x292 ||
                 msg.can_id == CAN_ID_VCFRONT_LIGHT ||
                 msg.can_id == CAN_ID_TURN_STALK ||
                 msg.can_id == CAN_ID_BLIND_SPOT ||
                 msg.can_id == 0x249 ||
                 msg.can_id == 0x311 ||
                 msg.can_id == 0x3E2);
```

12 comparaisons séquentielles à chaque message.

#### Solution

```cpp
// Initialiser dans setup() (réutilise le même bitfield que #4) :
static bool CAN_ID_PRIORITY[2048] = {false};

void initPriorityTable() {
    const uint16_t prio_ids[] = {
        0x257, 0x266, 0x2E5, 0x33A, 0x292,
        0x3F5, 0x045, 0x399, 0x249, 0x311, 0x3E2
    };
    for (auto id : prio_ids) CAN_ID_PRIORITY[id] = true;
}

// Usage :
bool priority = CAN_ID_PRIORITY[msg.can_id & 0x7FF];
```

**Gain** : 12 branches → 1 array lookup. 2 KB de RAM (trivial sur ESP32-S3).

---

### 7. ESP-NOW unicast vs broadcast

**Gain : ★★☆☆☆ | Complexité : ★☆☆☆☆**

#### Problème

```cpp
uint8_t broadcast_addr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_send(broadcast_addr, ...);
```

Le broadcast ESP-NOW :
- Pas d'ACK → aucune garantie de livraison
- Envoyé à tous les devices ESP-NOW sur le canal → radio plus longue
- Pas de retry automatique en cas de collision

#### Solution : Unicast vers Ecran

```cpp
// MAC Ecran connue : 20:6E:F1:9F:6E:C8
const uint8_t ECRAN_MAC[6] = {0x20, 0x6E, 0xF1, 0x9F, 0x6E, 0xC8};

void initializeESPNOW(void) {
    // ... init ...
    
    // Peer unicast — Ecran
    esp_now_peer_info_t ecran = {};
    memcpy(ecran.peer_addr, ECRAN_MAC, 6);
    ecran.channel = WIFI_CHANNEL;
    ecran.encrypt = false;
    ecran.ifidx = WIFI_IF_STA;
    esp_now_add_peer(&ecran);
}

void transmitViaESPNOW(ESP_CAN_Message_t* msg) {
    esp_now_send(ECRAN_MAC, (uint8_t*)msg, sizeof(*msg));
    // Unicast → ACK automatique + retry (1-2 retries par défaut)
}
```

**Gains** :
- Livraison confirmée (ACK) → meilleure fiabilité
- Retry automatique sur collision → moins de perte
- Callback `onESPNOWSend` devient significatif (status = SUCCESS/FAIL)

**Inconvénient** : Si on veut ajouter un 2ème receiver (logging), il faudra l'ajouter comme peer explicite ou revenir au broadcast.

---

### 8. MCP2515 SPI clock → 10 MHz

**Gain : ★★☆☆☆ | Complexité : ★☆☆☆☆**

#### Problème

La bibliothèque `mcp_can` utilise la fréquence SPI par défaut d'Arduino (~4 MHz). Le MCP2515 supporte jusqu'à **10 MHz** en mode SPI.

```
Lecture d'un frame : ~13 bytes SPI (ID + DLC + data + status)
  @ 4 MHz : 13 × 8 / 4M = 26 μs par read
  @ 10 MHz : 13 × 8 / 10M = 10.4 μs par read
  
Gain : 15.6 μs par frame × 2000 frames/s = 31.2 ms/s économisés
```

#### Solution

```cpp
void initializeCAN(void) {
    SPI.begin(MCP_SCK_PIN, MCP_MISO_PIN, MCP_MOSI_PIN, MCP_CS_PIN);
    SPI.setFrequency(10000000);  // 10 MHz (max MCP2515)
    // ...
}
```

> Note : Vérifier la stabilité avec un oscilloscope. Les fils T-2CAN internes sont courts → 10 MHz devrait fonctionner.

---

### 9. HW filter — IDs manquants

**Gain : ★★☆☆☆ | Complexité : ★★☆☆☆**

#### Problème

Le filtrage hardware MCP2515 couvre 0x100-0x5FF + 0x700-0x7FF via les 6 filtres. **Manquent** :

| Plage | IDs whitelist présents | Impact |
|-------|----------------------|--------|
| 0x000-0x0FF | `0x00C`, `0x016` | Rejetés par HW → jamais reçus |
| 0x600-0x6FF | `0x656` | Rejeté par HW → jamais reçu |

```
Le commentaire dit :
// Coverage: 154/159 DBC IDs (97%) — skips 0x000-0x0FF and 0x6xx (rarely seen)
```

#### Solution

Si ces IDs sont nécessaires, il faut réorganiser les filtres. Le MPC2515 a 2 masks × 6 filters. Avec `mask = 0x700`, on ne peut matcher que 6 plages de 256 IDs. Il y a 8 plages (0x0xx-0x7xx) mais on n'a que 6 slots.

**Option A** — Élargir un mask :
```
mask0 = 0x600 → filter 0x000 accepte 0x000-0x0FF ET 0x100-0x1FF
mask0 = 0x600 → filter 0x200 accepte 0x200-0x2FF ET 0x300-0x3FF
```
→ Les 2 filtres RXB0 couvrent 0x000-0x3FF (4 plages avec 2 filtres).

**Option B** — Accepter la perte si 0x00C/0x016/0x656 ne sont pas utilisés par l'Ecran actuellement.

> Vérifier avec `grep` dans le code Ecran quels CAN IDs sont effectivement parsés. Si 0x00C/0x016/0x656 ne sont pas utilisés, aucun impact.

---

### 10. TWAI error recovery

**Gain : ★☆☆☆☆ | Complexité : ★☆☆☆☆**

#### Problème

Si le ChassisBus provoque un bus-off (câble débranché, interférence), TWAI arrête de recevoir mais `twai_running = true` → pas de recovery.

#### Solution

```cpp
// Dans canRxTask, vérifier périodiquement :
static unsigned long lastTwaiCheck = 0;
if (millis() - lastTwaiCheck > 5000) {
    lastTwaiCheck = millis();
    twai_status_info_t status;
    if (twai_get_status_info(&status) == ESP_OK) {
        if (status.state == TWAI_STATE_BUS_OFF) {
            Serial.println("[TWAI] Bus-off detected, recovering...");
            twai_initiate_recovery();
        } else if (status.state == TWAI_STATE_STOPPED) {
            Serial.println("[TWAI] Stopped, restarting...");
            twai_start();
        }
    }
}
```

---

### 11. memcpy simplifié

**Gain : ★☆☆☆☆ | Complexité : ★☆☆☆☆**

#### Problème

```cpp
memcpy(msg.data, data, dlc);
if (dlc < 8) memset(msg.data + dlc, 0, 8 - dlc);
```

Deux appels pour chaque message. Le buffer `data` fait 16 bytes → lecture de 8 bytes est toujours safe.

#### Solution

```cpp
memcpy(msg.data, data, 8);  // Copie toujours 8 bytes (buffer source = 16 bytes)
```

Un seul `memcpy` de taille fixe = optimisable par le compilateur en 2 instructions `ldr`/`str` 32-bit.

---

## Récapitulatif RAM

| Ressource actuelle | Taille | Optimisation |
|-------------------|--------|-------------|
| `espnow_last_sent[2048]` | 8 192 B | Garder (coût faible) |
| `can_queue` (256 × 18B) | 4 608 B | Scinder en 2 queues (#2) |
| `data[16]` (stack, par iteration) | 16 B | OK |
| **Nouvelles structures** | | |
| `CAN_ID_BITFIELD[256]` | 256 B | #4 — remplace binary search |
| `CAN_ID_PRIORITY[2048]` | 2 048 B | #6 — remplace || chain |
| `batchBuf[10]` | 180 B | #1 — batch ESP-NOW |
| `prio_queue` (32 × 18B) | 576 B | #2 — queue prioritaire |
| **Total nouveau** | **~3 060 B** | ESP32-S3 a ~390 KB SRAM libre |

---

## Métriques à mesurer

Avant d'implémenter, ajouter des compteurs pour quantifier les gains potentiels :

```cpp
// Dans setup() — vérifier le tick rate
Serial.printf("[TICK] %d Hz (1 tick = %d ms)\n", configTICK_RATE_HZ, portTICK_PERIOD_MS);

// Stats additionnelles à ajouter à printStatistics() :
Serial.printf("  TWAI invalid: %lu\n", stats_twai_invalid);   // IDs reçus mais pas dans whitelist
Serial.printf("  MCP overflow: %lu\n", stats_mcp_overflow);   // MCP2515 EFLG.RX0OVR/RX1OVR
Serial.printf("  Batch avg:    %.1f\n", stats_batch_avg);     // Messages moyens par batch ESP-NOW
Serial.printf("  Prio drops:   %lu\n", stats_prio_drops);     // Messages prio droppés (queue full)

// Lire le registre d'erreur MCP2515 pour détecter les overflows :
byte eflg = CAN.getError();  // ou CAN.mcp2515_readRegister(MCP_EFLG)
if (eflg & 0x80) stats_mcp_overflow++;  // RX1OVR
if (eflg & 0x40) stats_mcp_overflow++;  // RX0OVR
```

---

## Plan d'implémentation recommandé

| Phase | Actions | Effort |
|-------|---------|--------|
| **1 — Quick wins** | #3 vérifier tick rate, #4 bitfield, #6 priority table, #8 SPI 10 MHz, #11 memcpy | 30 min |
| **2 — Fiabilité** | #2 priority queue, #7 unicast ESP-NOW, #10 TWAI recovery | 1-2h |
| **3 — Débit** | #1 batching ESP-NOW (+ modif Ecran callback) | 2-3h |
| **4 — Mesure** | #9 audit HW filter, #5 TWAI filter, métriques overflow | 1h |

> **Recommandation** : Commencer par la Phase 1 (aucun risque, gains immédiats), puis Phase 2 (fiabilité critique pour turn signals/blindspot), puis Phase 3 (le batching nécessite une modification coordonnée Bridge + Ecran).
