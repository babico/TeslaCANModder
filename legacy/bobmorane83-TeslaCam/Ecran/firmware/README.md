# JC3636W518C - Écran rond ESP32-S3

## Module

| Caractéristique | Valeur |
|---|---|
| MCU | ESP32-S3 (QFN56) rev v0.2 |
| Flash | 16 MB QIO |
| PSRAM | 8 MB OPI |
| Écran | ST77916, QSPI, 360×360 pixels, rond |
| USB | USB-Serial/JTAG (`/dev/cu.usbmodem101`) |

## Pinout QSPI

| Signal | GPIO |
|--------|------|
| CS | 10 |
| SCK | 9 |
| D0 | 11 |
| D1 | 12 |
| D2 | 13 |
| D3 | 14 |
| RST | 47 |
| Backlight | 15 |

## Firmware

### Stack technique

- **Framework** : Arduino (ESP32 core 3.x via pioarduino)
- **Librairie écran** : [Arduino_GFX](https://github.com/moononournation/Arduino_GFX) v1.6.5
- **Librairie PNG** : [PNGdec](https://github.com/bitbank2/PNGdec) v1.1.6
- **Filesystem** : SPIFFS (12 MB partition)

### Configuration importante

Le module JC3636W518**C** nécessite la séquence d'initialisation `st77916_150_init_operations` (et non la séquence par défaut `st77916_180`).

```cpp
Arduino_GFX *gfx = new Arduino_ST77916(
    bus, TFT_RST, 0, true, 360, 360,
    0, 0, 0, 0,
    st77916_150_init_operations, sizeof(st77916_150_init_operations));
```

Pour le rendu PNG, utiliser le format **Little Endian** :

```cpp
png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_LITTLE_ENDIAN, 0xffffffff);
gfx->draw16bitRGBBitmap(0, pDraw->y, lineBuffer, pDraw->iWidth, 1);
```

### Partitionnement (16 MB Flash)

| Partition | Type | Taille |
|-----------|------|--------|
| nvs | data | 20 KB |
| otadata | data | 8 KB |
| app0 | app | 4 MB |
| spiffs | data | ~12 MB |

## Commandes PlatformIO

```bash
# Compiler
pio run

# Flasher le firmware
pio run -t upload

# Uploader les fichiers SPIFFS (dossier data/)
pio run -t uploadfs

# Moniteur série
pio device monitor
```

## Structure du projet

```
firmware/
├── platformio.ini         # Config PlatformIO
├── partitions.csv         # Table de partitions 16MB
├── data/
│   └── photo.png          # Image 360×360 (stockée en SPIFFS)
└── src/
    └── main.cpp           # Firmware principal
```
