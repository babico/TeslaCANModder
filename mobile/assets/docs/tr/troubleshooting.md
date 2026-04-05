# Sorun Giderme

TeslaCANModder için yaygın sorunlar ve çözümleri.

## Bağlantı Sorunları

### "Web Serial desteklenmiyor"
- Chrome veya Edge kullanın (sürüm 89+). Firefox ve Safari Web Serial API'yi desteklemez.
- Mobilde USB yerine Bluetooth bağlantısını kullanın.

### Arduino port listesinde görünmüyor
- USB kablosunu kontrol edin — şarj kablosu değil, veri kablosu kullanın
- Arduino'nuz CH340 chip kullanıyorsa CH340 sürücülerini kurun
- Farklı bir USB portu deneyin
- Windows'ta Aygıt Yöneticisi'nden COM portunu kontrol edin

### Bluetooth bağlanmıyor
- HC-05 modülünün beslendiğinden emin olun (LED yanıp sönmeli)
- Önce telefon Bluetooth ayarlarından HC-05'i eşleştirin (varsayılan PIN: 1234)
- HC-05 veri modunda olmalı, AT komut modunda değil
- RX hattındaki gerilim bölücüyü kontrol edin (5V → 3.3V)

### Bağlandıktan hemen sonra "Bağlantı kesildi"
- Baud hızı uyumsuzluğu — firmware 115200 kullanır
- Başka bir uygulama seri portu kullanıyor olabilir
- Arduino'yu güç döngüsüne almayı deneyin

## CAN Bus Sorunları

### "CAN Bekliyor" — hiç çevrimiçi olmuyor
- MCP2515 kablolamasını kontrol edin (özellikle CS ve INT pinleri)
- X179 konektörünün tam oturduğunu doğrulayın
- Araç "uyanık" olmalı (ekrana dokunun, kapı açın veya şarj edin)
- CAN-H / CAN-L bağlantılarını kontrol edin — ters bağlamayın

### "CAN Bekleme" — bus sessizleşti
- Araç uyku moduna geçtiğinde normaldir
- Araç uyandığında kart otomatik kurtarır
- Sürekli ise 12V güç kaynağını kontrol edin

### Frame görünmüyor
- Önce akışı başlatın ("Akış" düğmesine tıklayın)
- Varyant seçiminin aracınızla eşleştiğini doğrulayın
- MCP2515 kristalini kontrol edin — 8 MHz olmalı

## Özellik Sorunları

### FSD etkinleşmiyor
- Doğru varyantın seçildiğini doğrulayın (HW4 / HW3)
- FSD'nin EEPROM panelinde "AÇIK" göründüğünü kontrol edin
- Bazı özellikler aracın Park veya Sürüş modunda olmasını gerektirir
- Etkinleştirdikten sonra kısa bir süre sürün — FSD hareket halinde etkinleşir

### Profil değişiklikleri etkili olmuyor
- Profil değişiklikleri bir sonraki hızlanma olayında uygulanır
- Sabitlenmiş profil çalışmıyorsa "Otomatik"e geçip yeniden seçin
- Değerin kaydedildiğini doğrulamak için EEPROM panelini kontrol edin

### Hız offset mevcut değil
- Hız offset sadece HW3 özelliğidir
- Aracınız destekliyorsa HW3 varyantına geçin

### ISA zili bastırılmıyor
- ISA zil bastırma sadece HW4 içindir
- Zilin tamamen bastırılması bir sürüş döngüsü sürebilir

## Firmware Sorunları

### Yükleme başarısız
- Doğru firmware varyantının seçildiğinden emin olun
- Porta bağlı diğer seri monitörleri kapatın
- PlatformIO CLI yöntemini deneyin: `cd hardware && pio run -e uno_usb -t upload`
- CH340 kullanıyorsanız en son sürücüleri kurun

### Yüklemeden sonra kart yanıt vermiyor
- Yeniden başlatma için 5 saniye bekleyin
- Arduino üzerindeki reset düğmesine basın
- Sorun devam ederse PlatformIO CLI ile yeniden yükleyin

## Donanım Sorunları

### MCP2515 #2 algılanmıyor
- CS pini (D9) ve INT pini (D3) kablolamasını kontrol edin
- Çift CAN firmware varyantı kullandığınızdan emin olun
- Boot mesajını kontrol edin — `bus2: true` bildirmeli

### Buck dönüştürücü aşırı ısınıyor
- 150mA'da 12V→5V için normaldir — 3A destekli modül kullanın
- Muhafaza içinde uygun havalandırma sağlayın
- Kablolamada kısa devre kontrolü yapın

### Kart rastgele yeniden başlıyor
- Güç kaynağı kararsız — buck dönüştürücüyü kontrol edin
- CAN bus gürültüsü — gerekirse 120Ω sonlandırma direnci ekleyin
- Bellek dolu — akış hızını azaltın veya frame tamponunu temizleyin
