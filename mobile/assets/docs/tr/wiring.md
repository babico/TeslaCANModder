# Kablolama Rehberi

Tüm donanım konfigürasyonları için detaylı kablolama şemaları.

## MCP2515 #1 → Arduino Uno (Zorunlu)

| MCP2515 | Arduino | Notlar |
| ------- | ------- | ------ |
| VCC | 5V | |
| GND | GND | |
| CS | D10 | SPI chip select |
| INT | D2 | Kesme (INT0) |
| SCK | D13 | SPI saat |
| MISO | D12 | SPI veri çıkış |
| MOSI | D11 | SPI veri giriş |

CAN-H / CAN-L'yi X179 pin 13/14'e (VehicleBus) bağlayın.

## MCP2515 #2 → Arduino Uno (İsteğe Bağlı Vehicle Bus)

İkinci MCP2515, Vehicle bus'u izlemenize olanak tanır. Firmware başlangıçta otomatik algılar.

| MCP2515 #2 | Arduino | Notlar |
| ---------- | ------- | ------ |
| VCC | 5V | Paylaşımlı hat |
| GND | GND | Paylaşımlı hat |
| CS | D9 | Benzersiz chip select |
| INT | D3 | Kesme (INT1) |
| SCK | D13 | #1 ile paylaşımlı |
| MISO | D12 | #1 ile paylaşımlı |
| MOSI | D11 | #1 ile paylaşımlı |

SPI hatları (SCK/MISO/MOSI) paylaşımlıdır — sadece CS ve INT farklıdır.

**Uyarı:** Arduino Uno'da tam 2 donanım kesmesi vardır: D2 (INT0) ve D3 (INT1). İkinci MCP2515 kullanırken D3'ü başka amaçla kullanmayın.

## HC-05 Bluetooth → Arduino Uno (İsteğe Bağlı)

| HC-05 | Arduino | Notlar |
| ----- | ------- | ------ |
| VCC | 5V | |
| GND | GND | |
| RX | D4 | Gerilim bölücü ile (5V → 3.3V) |
| TX | D5 | Doğrudan bağlantı |

**Uyarı:** HC-05 RX 3.3V mantık seviyesindedir. Arduino D4 → HC-05 RX hattında 1kΩ + 2kΩ gerilim bölücü kullanın.

## X179 Araç Bağlantısı

| X179 Pin | Bağlantı | Amaç |
| -------- | -------- | ---- |
| Pin 1 | Buck dönüştürücü VIN+ | 12V güç |
| Pin 20 | Buck dönüştürücü VIN- | Toprak |
| Pin 13 | MCP2515 #1 CAN-H | VehicleBus yüksek |
| Pin 14 | MCP2515 #1 CAN-L | VehicleBus düşük |

Buck dönüştürücü 5V çıkışı → Arduino USB portu. MCP2515 #2 için CAN-H/CAN-L'yi izlemek istediğiniz ikincil bus konektör çiftine bağlayın.

## Güç Notları

- Araç 12V, araba "uyanık" olduğunda mevcuttur (ekran açık, şarj, sürüş)
- Buck dönüştürücü, gerilim düşüşlerinde bile kararlı 5V sağlar
- Toplam akım tüketimi: ~150mA (Arduino + MCP2515 + HC-05)
- CAN bus sessizleştiğinde kart otomatik beklemeye geçer
