# Kurulum Rehberi

TeslaCANModder için donanım kurulumu ve yazılım yapılandırması.

## Hızlı Başlangıç

1. MCP2515'i Arduino Uno'ya bağlayın (Kablolama Rehberi'ne bakın)
2. Flasher sekmesini açın → firmware varyantını seçin → yükleyin
3. Dashboard'u açın → USB Bağlan → boot mesajını doğrulayın
4. Bağlantı çubuğundan varyantınızı seçin (HW4 / HW3 / Legacy)
5. Özellikleri etkinleştirin (FSD, Nag, vb.) — tümü varsayılan olarak KAPALI
6. X179 konektörü ile araca kurun

## Gerekli Donanım

- **Arduino Uno R3** — CH340 veya ATmega16U2 USB chip. Firmware'i çalıştırır.
- **MCP2515 CAN Modülü** — 8 MHz kristal + TJA1050 alıcı-verici. X179 ile VehicleBus'a bağlanır.
- **9V-36V → 5V Buck Dönüştürücü** — Minimum 3A. Arduino'yu araç 12V hattından besler.
- **Tesla X179 Konektörü** — Merkezi ekranın arkasında. 12V güç + CAN bus erişimi sağlar.

### İsteğe Bağlı

- **HC-05 Bluetooth Modülü** — Telefondan kablosuz kontrol. RX pininde gerilim bölücü gerektirir.
- **MCP2515 CAN Modülü #2** — İkinci CAN bus (örn. Powertrain). SPI paylaşır, D9/D3 kullanır.

## Firmware Varyantları

| Varyant | Bluetooth | Çift CAN | Kullanım |
|---------|-----------|----------|----------|
| Sadece USB | Hayır | Hayır | En hafif firmware |
| USB + Bluetooth | Evet | Hayır | HC-05 ile kablosuz kontrol |
| USB + Çift CAN | Hayır | Evet | İki CAN bus izleme |
| Tam | Evet | Evet | Her şey etkin |

Tüm varyantlar tüm FSD/araç özelliklerini destekler. Bluetooth ve Çift CAN sadece I/O kapasitesini etkiler.

## Araç Varyant Seçimi

| Varyant | Araçlar | Özellikler |
|---------|---------|------------|
| HW4 | 2023+ HW4 (FSD v14+) | FSD, Nag, Profil, ISA Zil, Çağırma |
| HW3 | 2019–2023 HW3 | FSD, Nag, Profil, Hız Offset, Çağırma |
| Legacy | Eski araçlar / basit CAN | Sadece FSD, Nag |

Bağlandıktan sonra bağlantı çubuğundan varyantınızı seçin. Firmware bunu EEPROM'a kaydeder.

## Yazılım Kurulumu

### Mobil Uygulama (Android / iOS)
1. Uygulamayı kaynaktan kurun veya Expo ile derleyin
2. Bluetooth (mobil) veya USB (web) ile bağlanın
3. Tüm kontroller Dashboard sekmesinde mevcut

### Web (Chrome / Edge)
1. Chrome veya Edge'de web uygulamasını açın (Web Serial API gerekli)
2. "USB Bağlan"a tıklayın ve Arduino seri portunu seçin
3. Firmware'i tarayıcıdan güncellemek için Flasher sekmesini kullanın

### PlatformIO CLI
```bash
cd hardware
pio run -e uno_usb -t upload
```
