# Donanım Varyantları

Desteklenen donanım kombinasyonları ve yetenekleri.

## Arduino Uno R3

Birincil (ve şu anda tek) desteklenen kart.

| Özellik | Detay |
| ------- | ----- |
| MCU | ATmega328P @ 16 MHz |
| Flash | 32 KB (tam özelliklerle firmware ~24 KB kullanır) |
| SRAM | 2 KB |
| EEPROM | 1 KB (ayarları saklar) |
| USB | CH340 veya ATmega16U2 |
| Dijital I/O | 14 pin (6 PWM) |
| SPI | D10-D13 (MCP2515 tarafından kullanılır) |
| Kesmeler | D2 (INT0), D3 (INT1) |

### Pin Tahsisi

| Pin | Fonksiyon | Zorunlu |
| --- | --------- | ------- |
| D2 | MCP2515 #1 INT | Evet |
| D3 | MCP2515 #2 INT | Sadece Vehicle bus |
| D4 | HC-05 RX (SoftwareSerial) | Sadece Bluetooth |
| D5 | HC-05 TX (SoftwareSerial) | Sadece Bluetooth |
| D9 | MCP2515 #2 CS | Sadece Vehicle bus |
| D10 | MCP2515 #1 CS | Evet |
| D11 | SPI MOSI | Evet |
| D12 | SPI MISO | Evet |
| D13 | SPI SCK | Evet |

## MCP2515 CAN Kontrolcü

| Özellik | Detay |
| ------- | ----- |
| Protokol | CAN 2.0A / 2.0B |
| Hız | Maksimum 1 Mbps (500 kbps kullanılır) |
| Arayüz | SPI @ maksimum 10 MHz |
| Alıcı-Verici | TJA1050 (modül üzerinde) |
| Kristal | 8 MHz (kritik — 16 MHz modüller çalışmaz) |
| Tampon | 2 alım, 3 gönderim |
| Filtreler | 6 kabul filtresi, 2 maske |

## HC-05 Bluetooth Modülü

| Özellik | Detay |
| ------- | ----- |
| Bluetooth | Klasik SPP (Seri Port Profili) |
| Baud Hızı | Varsayılan 9600, 115200 olarak yapılandırılır |
| Menzil | ~10 metre |
| Mantık Seviyesi | 3.3V (RX'te gerilim bölücü gerekir) |
| Varsayılan PIN | 1234 |
| Güç | 3.3V–6V |

### HC-05 Yapılandırması

Firmware, HC-05'in 115200 baud için yapılandırıldığını varsayar. Yapılandırmak için:

```
AT+UART=115200,0,0
AT+NAME=TeslaCANMod
AT+PSWD=1234
```

## Firmware Derleme Konfigürasyonları

| Ortam | BT | Flash Boyutu | RAM Kullanımı |
| ----- | --- | ---------- | ------------- |
| uno | Hayır | ~18 KB | ~800 B |
| uno_bt | Evet | ~22 KB | ~1.1 KB |

Bus etkinleştirme derleme bayrakları ile kontrol edilir: `BUS_VEHICLE_ACTIVE=1`, `BUS_BODY_ACTIVE=1`.

## Uyumluluk Notları

- **Kristal frekansı önemlidir:** Sadece 8 MHz MCP2515 modülleri desteklenir. 16 MHz modüller farklı saat bölücü gerektirir ve başlatılamaz.
- **SPI paylaşımı:** Her iki MCP2515 modülü SPI bus'ını paylaşır. Firmware CS arbitrajını yönetir.
- **Kesme önceliği:** INT0 (D2), INT1 (D3)'ten daha yüksek önceliğe sahiptir. Birincil bus ilk kesmeyi alır.
- **SoftwareSerial sınırlaması:** HC-05, SoftwareSerial (D4/D5) kullanır. Aynı anda sadece bir yazılım seri alabilir.
