# Arduino Uno R3 / Nano (klasik 5V)

Board ID: uno

## Categories
- 📚 Genel
- 🔧 Kurulum
- ⚙ Komutlar
- </> Kod

## Main Sections
- 💡 Neden Arduino Uno / Nano? En basit, en ucuz, evde duruyor
- 01 Parca Listesi Uno / Nano + MCP2515 — klasik 5V kombinasyon
- 02 Donanim Baglanti Uno / Nano + MCP2515 modulu — SPI uzerinden
- 03 Kutuphane Kurulumu Tek kutuphane gerek — mcp2515 by autowp
- 04 Ilk Yukleme Kodu ac, ayarla, derle, yukle
- ⚙ Serial Komut Listesi Runtime ayar degisimi — 9600 baud
- </> Kaynak Kod CanFeather_ArduinoUno.ino

## Code IDs
- code-uno

---

# Arduino Uno R3 / Nano (klasik 5V)

## Categories
- 📚 Genel
- 🔧 Kurulum
- ⚙ Komutlar
- </> Kod

## Extracted Content

## Arduino Uno R3 / Nano (klasik 5V)

ATmega328P · MCP2515 + Serial Monitor (WiFi'siz, lean)

📚 Genel
🔧 Kurulum
⚙ Komutlar
</> Kod

### 💡 Neden Arduino Uno / Nano? En basit, en ucuz, evde duruyor

▼

Arduino Uno R3 ve Arduino Nano (klasik 5V), CanFeather'in en minimal donanim hedefidir. Ikisi de ayni cipi (ATmega328P) ve ayni pin numaralarini kullanir — sadece form factor farkli (Uno tam boy, Nano breadboard dostu kucuk). Tek kod, iki kart , hicbir degisiklik gerekmez. Cogu hobicide zaten masada duran bir karttir — yeni donanim almaya gerek yok.

> Bu varyant kimin icin? Elinde Uno veya Nano duran, web arayuzu istemeyen, sadece "FSD bypass + speed profile" calismasi yetsin diyen kullanicilar icin. Daha fazla ozellik istiyorsaniz (web UI, OTA, vehicle control komutlari) ESP8266 veya ESP32 sekmesini kullanin.

> ⚠ DIKKAT — Hangi Nano? Bu kod sadece klasik 5V Arduino Nano (ATmega328P) icin calisir. Asagidakiler desteklenmiyor (farkli mimari, kod derlenmez):
> - Arduino Nano 33 BLE — 3.3V, nRF52840 ARM
> - Arduino Nano 33 IoT — 3.3V, SAMD21 ARM
> - Arduino Nano RP2040 Connect — 3.3V, RP2040 ARM
> - Arduino Nano Every — 5V ama ATmega4809, farkli pin layout, test edilmemis
> Almadan once urun aciklamasinda "ATmega328P" ve "5V" yazdigindan emin olun. CH340 USB chip'li Cin klonlari da klasik Nano sayilir, calisir.

#### Neler Var?

| Ozellik | Durum |
| --- | --- |
| HW3 / HW4 / Legacy FSD bypass | ✅ Var (handler kodu birebir ayni) |
| Speed profile (Chill .. Sloth) | ✅ Var (compile-time veya Serial) |
| ISA Speed Override (gercek hiz) | ✅ Var |
| ISA Speed Chime suppress | ✅ Var |
| Emergency vehicle detection | ✅ Var |
| Bypass "Trafik Isigi" gerekligi | ✅ Var |
| EEPROM persistence (boot ayari) | ✅ Var (1 KB EEPROM kullanilir) |
| Serial komut interface (runtime) | ✅ Var (9600 baud, '?' yardim) |
| Web arayuzu | ❌ Yok (WiFi yok) |
| Vehicle control (ayna/kilit/klima) | ❌ Yok (RAM yetmiyor) |
| OTA guncelleme | ❌ Yok (USB ile flash) |
| FSD history / log buffer | ❌ Yok (RAM yetmiyor) |

> Sinirlamalar: Uno'nun 2 KB RAM ve 32 KB Flash sinirlari sebebiyle bu varyant yalin tutulmustur. ATmega328P single-core 16 MHz — Tesla VehicleBus'in ~500 fps frame rate'i sinirina yakin calisir, frame loss riski ESP'lerden biraz daha yuksektir. Sahada beta olarak test edilmesi onerilir.

### 01 Parca Listesi Uno / Nano + MCP2515 — klasik 5V kombinasyon

▼

> MCP2515 VCC → 5V! TJA1050 transceiver 5V gerektirir. Uno ve Nano'nun 5V pinine baglayin (3.3V calismaz). 3.3V Nano varyantlari (BLE, IoT, RP2040 Connect) bu kod ile derlemez — sadece klasik ATmega328P 5V destekleniyor.

#### Temel Parcalar (Uno seceneği)

| # | Parca | Model | Fiyat | Link |
| --- | --- | --- | --- | --- |
| 1 | Mikrodenetleyici | Arduino Uno R3 klon (CH340 USB chip, kablo dahil) | ~150-200 TL | [Robotistan](https://www.robotistan.com/arduino-uno-r3-klon-usb-kablo-hediyeli-usb-chip-ch340) |
| 2 | CAN Controller | MCP2515 + TJA1050 Modulu | ~40-80 TL | [Robotistan](https://www.robotistan.com/mcp2515-canbus-spi-haberlesme-modulu) · [Direnc.net](https://www.direnc.net/mcp2515-modul) |
| 3 | Jumper Kablo | Disi-Disi, 10cm | ~10 TL | [Robotistan](https://www.robotistan.com/40-pin-ayrilabilen-disi-disi-f-f-jumper-kablo-200-mm) |
| 4 | JST-XH 2-pin | CAN hatti hizli sokum | ~5 TL | [Robotistan](https://www.robotistan.com/i-ds1066-scw002-2-pin-jst-terminalsiz-fis) |
| 5 | Proje Kutusu | 80x50x25mm (Uno tam boy) | ~20-30 TL | [Robotistan](https://www.robotistan.com/54-x-84-x-32-el-tipi-kutu-5921) |

#### Temel Parcalar (Nano seceneği — daha kucuk + ucuz)

| # | Parca | Model | Fiyat | Link |
| --- | --- | --- | --- | --- |
| 1 | Mikrodenetleyici | Arduino Nano (ATmega328, USB kablolu) | ~80-150 TL | [Robotistan](https://www.robotistan.com/arduino-nano-328-usb-kablolu) |
| 2 | CAN Controller | MCP2515 + TJA1050 Modulu | ~40-80 TL | [Robotistan](https://www.robotistan.com/mcp2515-canbus-spi-haberlesme-modulu) · [Direnc.net](https://www.direnc.net/mcp2515-modul) |
| 3 | Jumper Kablo | Disi-Disi, 10cm | ~10 TL | [Robotistan](https://www.robotistan.com/40-pin-ayrilabilen-disi-disi-f-f-jumper-kablo-200-mm) |
| 4 | JST-XH 2-pin | CAN hatti hizli sokum | ~5 TL | [Robotistan](https://www.robotistan.com/i-ds1066-scw002-2-pin-jst-terminalsiz-fis) |
| 5 | Proje Kutusu | Kucuk boy (Nano breadboard dostu) | ~15-25 TL | [Robotistan](https://www.robotistan.com/54-x-84-x-32-el-tipi-kutu-5921) |

#### Guc Kaynagi (her iki secenek icin ayni)

| # | Parca | Model / Not | Fiyat | Link |
| --- | --- | --- | --- | --- |
| 10 | Buck Converter | Mini560 (MP2315S) sabit 5V — giris 4.5-20V | ~15-30 TL | [Robotistan](https://www.robotistan.com/mini560-5v-dc-dc-voltaj-dusurucu) |
| 11 | Sigorta Tutucu + 3A | Arac hatti korumasi | ~10 TL | — |
| 12 | Disi JST-XH 2-pin (x2) | 12V giris + 5V cikis | ~5 TL | [Robotistan](https://www.robotistan.com/i-ds1066-scw002-2-pin-jst-terminalsiz-fis) |

> Tasarruf notu: Nano + MCP2515 + Mini560 toplam ~150-250 TL civarinda kalir — CanFeather'in en ucuz donanim seceneği. ESP8266 veya ESP32 ile karsilastirildiginda ~50-100 TL daha ucuz, ama web arayuzu olmadigi icin ayar degisimi Serial Monitor uzerinden yapilir.

### 02 Donanim Baglanti Uno / Nano + MCP2515 modulu — SPI uzerinden

▼

| MCP2515 Modul | Uno / Nano | Not |
| --- | --- | --- |
| VCC | 5V | Modul 5V tolerant |
| GND | GND | |
| CS | D10 (SS) | Chip Select |
| MOSI | D11 | SPI sabit pin |
| MISO | D12 | SPI sabit pin |
| SCK | D13 | SPI sabit pin (LED ile cakisir, normal) |
| INT | D2 | Kullanilmiyor (polling) |

#### Tesla X179 Konnektor

| MCP2515 | X179 Pin |
| --- | --- |
| CAN-H | Pin 13 |
| CAN-L | Pin 14 |

> Uno mu Nano mu? Pin numaralari ve baglantilari ayni (D10/D11/D12/D13). Tek farkli sey Arduino IDE'de board secimi:
> - Uno R3: Tools → Board → Arduino Uno
> - Nano (orijinal yeni): Tools → Board → Arduino Nano + Processor → ATmega328P
> - Nano (eski / Cin klonu): Tools → Board → Arduino Nano + Processor → ATmega328P (Old Bootloader)
> Eger Nano'da yukleme " avrdude: stk500_recv(): programmer is not responding " hatasi verirse Old Bootloader secenegini dene. Cogu CH340 USB chip'li klon Old Bootloader ister.

> ⚠ MCP2515 modulundeki J1 jumper'ini CIKARIN. Bu jumper 120Ω terminasyon direncini devreye sokuyor. Tesla'nin CAN bus'i zaten kendi terminasyonuna sahip — ikincisi paralel olunca bus empedansi 60Ω yerine 40Ω olur, frame error patlamasi yaratir. Module'u kullanmadan once kucuk PCB jumper'ini (genelde modulun bir kosesinde "J1" etiketli) cikar veya kes .

> ⚠ 3.3V Nano varyantlari calismaz. Nano 33 BLE, Nano 33 IoT, Nano RP2040 Connect — bunlarin hicbiri ATmega328P degil, ARM tabanli ve 3.3V calisir. Bu kodu derlemezler. Ayrica Tesla'nin 5V CAN sinyaline dogrudan baglanmaniz icin transceiver level shift'i farkli olur. Sadece klasik 5V Arduino Nano (ATmega328P) destekleniyor — satin alirken urun sayfasinda "ATmega328P" yazdigindan emin olun.

> MCP2515 kristal frekansi: Cogu modul 8 MHz veya 16 MHz kristal ile gelir. Modulu ters cevirip kristalin uzerindeki yaziya bakin. Kod icindeki #define CAN_CLOCK_MHZ degerini ona gore ayarlayin (8 veya 16). Yanlis ayar → bus'a hic frame okuyamaz.

### 03 Kutuphane Kurulumu Tek kutuphane gerek — mcp2515 by autowp

▼

- Arduino IDE ac (1.8.x veya 2.x)

- Tools → Manage Libraries (Library Manager)

- Arama kutusuna mcp2515 yaz

- "mcp2515 by autowp" kutuphanesini kur (genelde ilk sirada gelir)

- Tools → Board → Arduino Uno sec

- Tools → Port → Uno'nun bagli oldugu portu sec

> SPI ve EEPROM kutuphaneleri Arduino IDE ile birlikte gelir, ayrica kurmaya gerek yok.

### 04 Ilk Yukleme Kodu ac, ayarla, derle, yukle

▼

- "Kod" sekmesinden CanFeather_ArduinoUno.ino dosyasini indir veya kopyala

- Arduino IDE'de ac

- Dosyanin uzerindeki AYARLAR bolumunde compile-time defaultlari kontrol et:

#define DEFAULT_HW — aracin (0=Legacy, 1=HW3, 2=HW4)

- #define DEFAULT_PROFILE — baslangic profili (0..4)

- #define CAN_CLOCK_MHZ — MCP2515 kristali (8 veya 16)

- Verify (✓) ile derle — hata almamali

- Upload (→) ile Uno'ya yukle

- Tools → Serial Monitor → 9600 baud sec

- Boot mesajini gormeli: === CanFeather Uno v2.8-uno ===

- Status icin s yaz, komut listesi icin ? yaz

### ⚙ Serial Komut Listesi Runtime ayar degisimi — 9600 baud

▼

Web arayuzu olmadigi icin tum ayarlar Serial Monitor uzerinden yapilir. Komutu yazip Enter'a bas. EEPROM'a kalici kaydetmek icin sonunda save komutunu unutma.

| Komut | Aciklama | Ornek |
| --- | --- | --- |
| ? | Yardim — komut listesi | ? |
| s | Status — anlik durum (HW, profile, FSD, RX/TX/ERR) | s |
| hw 0\\|1\\|2 | HW seciminin degistir (0=Legacy 1=HW3 2=HW4) | hw 2 |
| p 0..4 | Speed profile (0=Chill 1=Normal 2=Hurry 3=Max 4=Sloth) | p 4 |
| fsd on\\|off | FSD enjeksiyonu | fsd on |
| inj on\\|off | Tum CAN enjeksiyonu (FSD dahil) — "guvenlik kapama" | inj off |
| isa on\\|off | ISA hiz uyari sesi suppress (HW3/HW4) | isa on |
| ev on\\|off | Acil arac algilama (HW3/HW4 bit59) | ev on |
| tlssc on\\|off | Bypass "Trafik Isigi" gerekligi (UI ayarsiz FSD) | tlssc on |
| ovr on\\|off | Profile override (CAN auto-mapping yerine sabit) | ovr on |
| isaovr on\\|off | ISA Speed Override (gercek hizi nav hiz limitine ezdirme) | isaovr on |
| isamul N | ISA hiz carpan (1-15, default 7) | isamul 9 |
| save | Mevcut ayarlari EEPROM'a yaz (kalici) | save |
| load | EEPROM'dan ayarlari geri yukle | load |
| reset | EEPROM'u temizle — default ayarlara don | reset |

#### Ornek Akis

> ?
// komut listesi

> s
// status goster

FW=v2.8-uno HW=2 P=2 FSD=ON INJ=ON UI=OFF FD=0 OFF=0 RX=4521 TX=890 ERR=0
> hw 2
// HW4 secimi

hw=2
> p 4
// Sloth profili

p=4
> tlssc on
// Trafik Isigi gerekligi bypass

tlssc=ON
> save
// EEPROM'a yaz

[ee] kaydedildi
> s
// dogrula

FW=v2.8-uno HW=2 P=4 FSD=ON INJ=ON UI=OFF FD=0 OFF=0 RX=8932 TX=1782 ERR=0

> Bir kere save ettikten sonra kabloyu cikarip bagladiginda Uno EEPROM'dan ayarlari otomatik yukler — Serial Monitor acmaya gerek yok, "tak ve calistir".

### Kaynak Kod CanFeather_ArduinoUno.ino

▼

CanFeather_ArduinoUno.ino
Arduino Uno R3 — lean (WiFi'siz)

Kopyala
