# PRD — TeslaCam : Streaming Photo Camera → Écran

## 1. Résumé du projet

Système embarqué de caméra frontale pour Tesla composé de deux ESP32-S3 communiquant en Wi-Fi. L'ESP32-S3 **Camera** capture une photo JPEG 360×360 via le capteur OV5640, puis l'envoie à l'ESP32-S3 **Écran** qui l'affiche sur un écran rond ST77916 (360×360 pixels).

---

## 2. Matériel

| Composant | Module | Caractéristiques clés |
|---|---|---|
| **Camera** | Freenove ESP32-S3 WROOM | ESP32-S3, OV5640, 8 MB PSRAM OPI, Wi-Fi 802.11n |
| **Écran** | JC3636W518C | ESP32-S3, ST77916 QSPI, 360×360 rond, 16 MB Flash, 8 MB PSRAM OPI |
| **Bridge** | LilyGo T-2CAN v1.0 | ESP32-S3-WROOM-1U, MCP2515 SPI CAN controller, 16 MHz crystal, bus CAN 500 kbps, 16 MB Flash, 8 MB PSRAM OPI |

---

## 3. Architecture

```
┌───────────────────────┐         Wi-Fi (SoftAP ch.1)   ┌───────────────────────┐
│   ESP32-S3 Camera     │ ────── UDP port 5000 ──────── │   ESP32-S3 Écran      │
│                       │                                │                       │
│  OV5640 (JPEG 360x360)│  Paquets UDP chunked          │  ST77916 360×360 rond │
│  Mode : SoftAP        │  Header 8B + payload ≤1452B   │  Mode : Station       │
│  IP : 192.168.4.1     │                                │  IP : 192.168.4.2     │
│                       │                                │  JPEGDEC + GFX        │
└───────────────────────┘                                └───────────────────────┘
                                                                    ▲
                                                                    │ ESP-NOW
                                                                    │ (channel 1)
                                                         ┌──────────┴──────────┐
                                                         │  ESP32-S3 Bridge    │
                                                         │  (LilyGo T-2CAN)   │
                                                         │  MCP2515 CAN 500kbps│
                                                         │  CAN → ESP-NOW      │
                                                         │  Trames Tesla M3    │
                                                         └─────────────────────┘
```

### Choix de communication : SoftAP + UDP raw

Basé sur l'analyse du Guide.txt, le protocole **SoftAP + UDP raw** est retenu car :

- **Débit** : ~20 Mbps effectifs en UDP sur Wi-Fi 802.11n (vs ~1 Mbps ESP-NOW)
- **Latence** : < 80 ms (vs 200–400 ms en HTTP/MJPEG)
- **Autonomie** : pas de routeur Wi-Fi externe nécessaire
- **Résolution cible** : 360×360 JPEG (~15–20 KB/frame) → largement dans la bande passante

---

## 4. Exigences fonctionnelles

### 4.1 ESP32-S3 Camera (émetteur)

| ID | Exigence | Détail |
|---|---|---|
| CAM-01 | Capturer des photos JPEG 360×360 | Utiliser la compression JPEG native de l'OV5640. Framesize personnalisé ou crop depuis HVGA. |
| CAM-02 | Créer un Access Point Wi-Fi | SSID : `TeslaCam`, pas de mot de passe (réseau local fermé). IP : `192.168.4.1`. |
| CAM-03 | Envoyer les frames via UDP chunked | Découper chaque JPEG en chunks ≤ 1452 bytes avec header 8 bytes. Port destination : `5000`. |
| CAM-04 | Cadence cible | ≥ 15 fps en continu (quand le streaming est actif). |
| CAM-05 | Miroir horizontal + flip vertical | Image retournée pour montage caméra frontale (configurable). |
| CAM-06 | Task FreeRTOS dédiée | Le streaming tourne sur un core dédié (core 1) pour ne pas bloquer le système. |
| CAM-07 | Mode veille par défaut | Au démarrage, la caméra **n'émet pas**. Elle attend une commande `START` de l'Écran via UDP avant de commencer la capture et l'envoi. Sur réception d'une commande `STOP`, elle cesse la capture et repasse en veille (économie d'énergie et réduction de la chauffe). |

### 4.2 ESP32-S3 Écran (récepteur)

| ID | Exigence | Détail |
|---|---|---|
| ECR-01 | Se connecter au SoftAP Camera | Station Wi-Fi connectée à `TeslaCam`, IP statique `192.168.4.2`. |
| ECR-02 | Recevoir et réassembler les frames UDP | Écouter sur le port `5000`. Réassembler les chunks par `frame_id`. Drop des frames incomplètes ou en retard. |
| ECR-03 | Décoder le JPEG | Utiliser la librairie JPEGDEC pour décoder le JPEG en RGB565. |
| ECR-04 | Afficher sur l'écran ST77916 | Rendu via Arduino_GFX `draw16bitRGBBitmap()`. Pas d'LVGL nécessaire pour l'affichage photo (rendu direct). |
| ECR-05 | Timeout / écran noir | Si aucune frame complète reçue pendant > 500 ms, afficher un écran noir (ne jamais rester sur une image figée). |
| ECR-06 | Triple buffering frames | Trois buffers JPEG en PSRAM (réception / prêt / affichage) avec mutex FreeRTOS, pour éviter les artefacts et la perte de paquets. |
| ECR-07 | Activation/désactivation par toucher | Un appui sur l'écran tactile envoie une commande `START` à la Camera (UDP). Un second appui envoie `STOP`. L'état bascule à chaque toucher (toggle). Quand le streaming est inactif, l'écran affiche un indicateur visuel (ex: icône caméra barrée ou texte "Appuyez pour activer"). |
| ECR-08 | Commande UDP de contrôle | Envoie les commandes `START` / `STOP` à la Camera via UDP port `5001` (port de contrôle distinct du port vidéo `5000`). |
| ECR-09 | Réception ESP-NOW du Bridge | Reçoit les trames CAN transmises par le Bridge via ESP-NOW (broadcast, canal 1). Décode 7 signaux CAN : vitesse (0x257), rapport (0x118), SoC (0x292), range (0x33A), temp batterie (0x312), puissance arrière (0x266), puissance avant (0x2E5). Structure extensible pour ajout futur de signaux. |
| ECR-10 | Dashboard instrument cluster | Affiche un tableau de bord complet inspiré du mockup HTML (`Ecran/mockup/ev_round_display.html`) : arc de vitesse coloré (teal→ambre→rouge, 0-180 km/h), arc SoC batterie, sélecteur de rapport P/R/N/D avec couleurs, vitesse en grand (FreeSansBold 24pt × 3), autonomie en km, température batterie °C, pourcentage SoC, barre de régénération 0-50 kW avec lissage. Bezel statique pré-rendu en PSRAM (ticks, numéros, labels) + éléments dynamiques recalculés à 10 Hz. |
| ECR-11 | Indicateurs de connexion | Deux points colorés en haut de l'écran indiquent l'état de connexion : **gauche** = Bridge (vert connecté, rouge déconnecté, orange clignotant en recherche), **droite** = Camera (vert connecté, rouge déconnecté). Visibles sur le dashboard ET en overlay sur le flux vidéo. |
| ECR-12 | WiFi non-bloquant | La connexion WiFi à la Camera est non-bloquante : le dashboard s'affiche immédiatement au démarrage. La pile réseau (UDP, ESP-NOW) est initialisée quand la connexion aboutit. Reconnexion automatique en arrière-plan. |

### 4.3 ESP32 Bridge (passerelle CAN → ESP-NOW)

| ID | Exigence | Détail |
|---|---|---|
| BRG-01 | Lire le bus CAN Tesla | MCP2515 en mode listen-only, 500 kbps, cristal 16 MHz, filtres logiciels sur les IDs Tesla Model 3 (159 IDs DBC). Pas de pin INT — polling SPI via `checkReceive()`. |
| BRG-02 | Émettre via ESP-NOW | Broadcast des trames CAN sous forme de `ESP_CAN_Message_t` (18 bytes packed) à tous les peers ESP-NOW sur canal 1. Rate limiting : max 1 msg/200ms par CAN ID sauf IDs prioritaires. |
| BRG-03 | Heartbeat | Émet une trame heartbeat (CAN ID 0xFFF, DLC 0) toutes les 1 seconde via ESP-NOW. |
| BRG-04 | AP auto-shutdown | Démarre un AP Wi-Fi ("bridge") pour le debug/dashboard. Si aucun client après 3 min, l'AP est coupé et le Bridge passe en mode ESP-NOW seul (économie CPU). |
| BRG-05 | Canal Wi-Fi | Utilise le canal Wi-Fi 1, partagé avec le SoftAP Camera pour permettre la réception ESP-NOW par l'Écran. |

---

## 5. Protocole UDP

### 5.1 Canal vidéo (port 5000) — Camera → Écran

```
┌─────────────────────────────────────────┐
│  HEADER  (8 bytes)                      │
│  frame_id     : uint16_t  (2B)          │
│  chunk_id     : uint16_t  (2B)          │
│  total_chunks : uint16_t  (2B)          │
│  chunk_size   : uint16_t  (2B)          │
├─────────────────────────────────────────┤
│  PAYLOAD (≤ 1452 bytes)                 │
│  données JPEG brutes                    │
└─────────────────────────────────────────┘
```

- **MTU Wi-Fi** : 1460 bytes utiles en UDP → 8 header + 1452 payload
- **Frame JPEG 360×360** estimée : ~12–20 KB → ~9–14 chunks par frame

### 5.2 Canal de contrôle (port 5001) — Écran → Camera

```
┌─────────────────────────────────────────┐
│  COMMAND (4 bytes ASCII)                │
│  "STRT" = démarrer capture + streaming  │
│  "STOP" = arrêter capture + streaming   │
└─────────────────────────────────────────┘
```

- L'Écran envoie la commande à `192.168.4.1:5001`
- La Camera écoute sur le port `5001` en plus du port vidéo
- Au démarrage, la Camera est en mode veille (pas de capture)
- L'Écran réémet `STRT` périodiquement (toutes les 2s) tant que le streaming est actif, pour gérer les reconnexions

### 5.3 Canal CAN / ESP-NOW — Bridge → Écran

```
┌─────────────────────────────────────────┐
│  ESP_CAN_Message_t (18 bytes packed)    │
│  can_id      : uint32_t  (4B, LE)      │
│  dlc         : uint8_t   (1B)          │
│  data[8]     : uint8_t   (8B)          │
│  timestamp   : uint32_t  (4B, LE)      │
│  is_extended : uint8_t   (1B)          │
└─────────────────────────────────────────┘
```

- **Transport** : ESP-NOW broadcast (FF:FF:FF:FF:FF:FF) sur canal Wi-Fi 1
- **Débit** : ~1000 trames CAN/s (filtré par whitelist DBC)
- **Signaux CAN décodés par l'Écran** :
  - `DI_uiSpeed` (CAN 0x257) : bit 24, 9 bits — vitesse affichée
  - `DI_gear` (CAN 0x118) : bit 21, 3 bits — P=1, R=2, N=3, D=4, SNA=7
  - `SOCUI292` (CAN 0x292) : bit 10, 10 bits, ×0.1 — SoC batterie % (source primaire)
  - `UI_Range` (CAN 0x33A) : bit 0, 10 bits — autonomie en miles (×1.609 → km)
  - ⚠️ `UI_SOC` (CAN 0x33A) : bit 48, 7 bits — **non fiable** (donne 111 au lieu du % réel)
  - `BMSmaxPackTemperature` (CAN 0x312) : bit 53, 9 bits, ×0.25−25 — °C
  - `RearPower266` (CAN 0x266) : bit 0, 11 bits signé, ×0.5 — kW
  - `FrontPower2E5` (CAN 0x2E5) : bit 0, 11 bits signé, ×0.5 — kW
  - Regen = −(rear+front power) quand négatif, lissage exponentiel 0.72/0.28
- **Heartbeat** : CAN ID `0xFFF`, DLC=0, toutes les 1s

---

## 6. Configuration OV5640

```cpp
sensor_t *s = esp_camera_sensor_get();
s->set_framesize(s, FRAMESIZE_HVGA);   // 480×320, puis crop/resize à 360×360
s->set_quality(s, 10);                  // Qualité JPEG (bon compromis taille/qualité)
s->set_pixformat(s, PIXFORMAT_JPEG);
s->set_vflip(s, 1);                    // Flip vertical (caméra frontale)
s->set_hmirror(s, 1);                  // Miroir horizontal
```

> **Note** : L'OV5640 ne supporte pas nativement le 360×360. Stratégie : capturer en HVGA (480×320) ou VGA (640×480) en JPEG, puis laisser le récepteur décoder et afficher centré/croppé sur le 360×360.

---

## 7. Performances attendues

| Métrique | Valeur cible |
|---|---|
| Résolution capture | HVGA 480×320 (crop à 360×360 côté écran) |
| Taille JPEG moyenne | ~15–20 KB |
| FPS | ≥ 15 fps |
| Latence bout-en-bout | < 100 ms |
| Portée Wi-Fi | ~10 m (intérieur véhicule, largement suffisant) |
| Consommation Camera | ~350 mA @ 3.3V |

---

## 8. Stack technique

### Camera (`Camera/`)

| Composant | Choix |
|---|---|
| Framework | Arduino (ESP-IDF sous-jacent) |
| Platform | espressif32 (PlatformIO) |
| Wi-Fi | Mode SoftAP natif ESP-IDF |
| Transport | AsyncUDP (ou `lwip_sendto` raw) |
| Capture | `esp_camera` API |

### Écran (`Ecran/firmware/`)

| Composant | Choix |
|---|---|
| Framework | Arduino (pioarduino ESP32 core) |
| Platform | espressif32 via pioarduino |
| Display driver | Arduino_GFX (ST77916 QSPI) |
| Décodage JPEG | JPEGDEC (bitbank2) |
| Wi-Fi | Mode Station |
| ESP-NOW | Réception trames Bridge (canal 1) |
| Transport | lwip UDP raw (ports 5000, 5001) |

### Bridge (`Bridge/`)

| Composant | Choix |
|---|---|
| Framework | Arduino (espressif32) |
| Platform | espressif32 (PlatformIO) |
| CAN | MCP2515 via mcp_can (SPI) |
| Wi-Fi | AP mode (debug) puis STA mode (ESP-NOW only) |
| ESP-NOW | Broadcast trames CAN filtrées |

### Dépendances à ajouter

**Camera** `platformio.ini` :
```ini
lib_deps =
    ESP Async UDP
```

**Écran** `platformio.ini` :
```ini
lib_deps =
    moononournation/GFX Library for Arduino@^1.5.0
    bitbank2/JPEGDEC@^1.4.2
    ESP Async UDP
```

> LVGL n'est plus nécessaire pour la phase d'affichage photo (rendu direct GFX). Il pourra être réintégré plus tard pour une UI complète.

---

## 9. Plan d'implémentation

### Phase 1 — Communication de base
- [x] **Camera** : Remplacer le mode Station + HTTP par un mode SoftAP + UDP sender
- [x] **Écran** : Remplacer la démo arc gauge par un récepteur UDP + affichage JPEG
- [x] Valider la réception d'une frame JPEG complète (log Serial)

### Phase 2 — Affichage fonctionnel
- [x] **Écran** : Décoder les JPEG reçus via JPEGDEC et afficher via `draw16bitRGBBitmap()`
- [x] **Écran** : Implémenter le crop/centrage de l'image HVGA vers 360×360
- [x] **Écran** : Implémenter le timeout (écran noir après 500 ms sans frame)

### Phase 3 — Optimisation
- [x] Mesurer et optimiser le FPS et la latence
- [x] Triple buffer + task UDP dédiée + screen buffer batch → 12.5 fps
- [ ] Ajuster la qualité JPEG et la résolution pour le meilleur compromis

### Phase 4 — Activation par toucher
- [x] **Écran** : Détecter le toucher sur l'écran tactile (CST816S, I2C addr 0x15, SDA=7, SCL=8)
- [x] **Écran** : Implémenter le toggle START/STOP avec envoi de commande UDP sur port 5001
- [x] **Écran** : Afficher un écran d'attente quand le streaming est inactif ("Appuyez pour activer")
- [x] **Camera** : Écouter le port 5001 pour les commandes de contrôle

### Phase 5 — Intégration Bridge CAN + Dashboard
- [x] **Camera** : Configurer le SoftAP sur canal Wi-Fi 1 (alignement avec Bridge ESP-NOW)
- [x] **Écran** : Initialiser ESP-NOW après connexion Wi-Fi pour recevoir les broadcasts du Bridge
- [x] **Écran** : Décoder 6 signaux CAN depuis les trames ESP-NOW (vitesse, rapport, SoC, range, temp, puissance)
- [x] **Écran** : Implémenter le dashboard complet (arcs, vitesse, rapport, widgets, regen) inspiré du mockup HTML
- [x] **Écran** : Bezel statique pré-rendu en PSRAM + rendu dynamique à 10 Hz sans flicker
- [x] **Écran** : WiFi non-bloquant — dashboard visible immédiatement au démarrage
- [x] **Écran** : Indicateurs de connexion (dots Bridge + Camera) sur dashboard et overlay vidéo
- [x] **Camera** : Ne capturer et émettre que lorsque la commande START a été reçue
- [x] **Camera** : Couper la capture (et réduire la consommation) sur commande STOP

---

## 10. Risques et mitigations

| Risque | Impact | Mitigation |
|---|---|---|
| Perte de paquets UDP | Artefacts visuels, frames incomplètes | Drop des frames incomplètes, toujours afficher la dernière frame valide |
| OV5640 ne fait pas 360×360 natif | Image non carrée | Capturer en HVGA/VGA puis crop côté récepteur |
| Interférences Wi-Fi dans le véhicule | Baisses de FPS | Canal Wi-Fi fixe (1 ou 11), puissance TX maximale |
| Dépassement mémoire PSRAM | Crash | Limiter à 3 buffers, surveiller `heap_caps_get_free_size()` |
| Chauffe du module Camera | Dégradation | Mode veille par défaut ; capture uniquement sur demande via toucher écran |

---

## 11. Critères de validation

- [ ] L'image de la caméra s'affiche sur l'écran rond en < 100 ms de latence
- [ ] Le FPS mesuré est ≥ 15 fps en conditions normales
- [ ] L'écran passe au noir si la caméra est déconnectée (timeout 500 ms)
- [ ] Le système fonctionne sans routeur Wi-Fi externe (SoftAP autonome)
- [ ] Pas de fuite mémoire après 1h de fonctionnement continu
- [ ] Un appui sur l'écran démarre le streaming, un second l'arrête
- [ ] La Camera ne consomme pas de ressources CPU/Wi-Fi en mode veille
- [ ] L'écran affiche un état visuel clair (actif vs inactif)
