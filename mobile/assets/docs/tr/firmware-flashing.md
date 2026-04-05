# Firmware Yükleme

Web uygulaması veya PlatformIO CLI kullanarak Arduino Uno'nuza firmware yükleme.

## Web Flasher (Önerilen)

Firmware yüklemenin en kolay yolu uygulamanın Flasher sekmesidir.

### Gereksinimler
- Chrome veya Edge tarayıcı (Web Serial API)
- USB ile bağlı Arduino Uno
- Açık başka seri monitör olmamalı

### Adımlar
1. Uygulamayı açın ve **Flasher** sekmesine gidin
2. Firmware varyantınızı seçin:
   - **Sadece USB** — en hafif, BT veya çift CAN yok
   - **USB + Bluetooth** — HC-05 desteği ekler
   - **USB + Çift CAN** — ikinci MCP2515 ekler
   - **Tam** — her şey etkin
3. **USB ile Yükle**'ye tıklayın
4. İstendiğinde Arduino seri portunu seçin
5. "Başarıyla yüklendi" mesajını bekleyin
6. Kart yeni firmware ile otomatik yeniden başlar

### Sorun Giderme
- Yükleme başarısız olursa önce diğer seri bağlantıları kapatın
- Yüklemeden önce Arduino reset düğmesine basmayı deneyin
- Yedek olarak PlatformIO CLI kullanın

## PlatformIO CLI

Geliştiriciler için veya web flasher çalışmadığında.

### PlatformIO Kurulumu
```bash
pip install platformio
```

### Yükleme Komutları
```bash
cd hardware

# Sadece USB
pio run -e uno_usb -t upload

# USB + Bluetooth
pio run -e uno_usb_bt -t upload

# USB + Çift CAN
pio run -e uno_usb_mcp2 -t upload

# Tam (USB + BT + Çift CAN)
pio run -e uno_full -t upload
```

### Yüklemeden Derleme
```bash
pio run -e uno_usb
```

### Seri Çıkışı İzleme
```bash
pio device monitor -b 115200
```

## Derleme Bayrakları

Her firmware varyantı `platformio.ini` içindeki derleme bayrakları ile kontrol edilir:

| Bayrak | Varsayılan | Açıklama |
|--------|-----------|----------|
| BOARD_ENABLE_BT | 0 | HC-05 Bluetooth'u etkinleştir |
| BOARD_ENABLE_MCP2515_2 | 0 | İkinci MCP2515'i etkinleştir |

## Firmware Güncelleme Süreci

1. Arduino'yu USB ile bağlayın
2. Yeni firmware'i yükleyin (web veya CLI)
3. Kart otomatik yeniden başlar
4. EEPROM ayarları güncellemeler arasında korunur
5. Konsol'daki boot mesajını kontrol ederek doğrulayın

## EEPROM Sıfırlama

Tüm kayıtlı ayarları fabrika varsayılanlarına sıfırlamak için:
```json
{"cmd":"factory_reset"}
```
Bu, varyant, FSD, nag, profil, offset ve diğer tüm kayıtlı ayarları temizler.
