# ESP32-S3 TWAI (Waveshare)

Board ID: esp32s3

## Categories
- (none)

## Main Sections
- 01 Neden Bu Kart? Tek kart, 2 kablo, bitti
- 02 Baglanti CAN + Enerji — hepsi bu
- 🔌 Enhance Auto Kablo ile Kolay Baglanti Tek kablo ile CAN + guc — lehim/splice gerekmez
- 03 Arduino IDE Ayarlari Board + kutuphane
- 04 Sorun Giderme
- </> Arduino Kodu CanFeather_ESP32S3_TWAI.ino

## Code IDs
- code-esp32s3

---

# ESP32-S3 TWAI (Waveshare)

## Extracted Content

## ESP32-S3 TWAI (Waveshare)

Dahili CAN + WiFi

### 01 Neden Bu Kart? Tek kart, 2 kablo, bitti

▼

> En kolay kurulum: Bu kart CAN bus, WiFi ve DIN rail kutusunu tek pakette sunar. Tesla 12V dogrudan baglanir (7-36V destekler), USB-C ile de calisir. Harici modul veya ek parca gerekmez.

[Image: Waveshare ESP32-S3-RS485-CAN]

Waveshare ESP32-S3-RS485-CAN

[Image: ESP32-S3-RS485-CAN Sema]

Interface & Pin Diyagrami

| Ozellik | Diger Kartlar | ESP32-S3 TWAI |
| --- | --- | --- |
| CAN Controller | Harici MCP2515 (SPI kablo gerekir) | Dahili TWAI (kablo yok) |
| CAN Transceiver | Harici TJA1050 | Dahili TJA1051T/3 (izole) |
| WiFi | ESP8266/ESP32 ile dahili | Dahili (ESP32-S3) |
| Guc Girisi | Harici buck converter gerekir | 12V direkt veya USB-C (7-36V destekler) |
| Kutu | Ayri proje kutusu | DIN rail kutu dahil |
| Toplam Parca | 5-7 parca + kablolar | 1 kart + 2 kablo |
| Izolasyon | Yok | Galvanik izolasyon |

### 02 Baglanti CAN + Enerji — hepsi bu

▼

#### Onerilen: Enhance Auto Gen 2 Cable

> Tek kablo, her sey dahil. Enhance Auto Tesla Gen 2 Cable X179 konnektorune takilir ve hem CAN bus hem 12V guc saglar. Lehim, kesme, splice gerekmez — tik sesi ile yerine oturur.

| # | Parca | Not | Link |
| --- | --- | --- | --- |
| 1 | Enhance Auto Tesla Gen 2 Cable | X179’a takilir, CAN + 12V saglar | [enhauto.com](https://enhauto.com) |

#### Kablo Pinout (Enhance Auto)

> Kablonun bir ucu X179’a takilir. Diger ucunda serbest kablolar vardir:

| Kablo Rengi | Sinyal | Nereye Baglanir |
| --- | --- | --- |
| Kirmizi | 12V+ | Board 7-36V terminal (+) — converter gerekmez! |
| Siyah | GND | Board 7-36V terminal (−) |
| Siyah cizgili | CAN-H (Body Bus) | Board CAN terminal H |
| Siyah duz | CAN-L (Body Bus) | Board CAN terminal L |
| Diger siyah cift | Other Bus | Kullanilmiyor — bos birakin |

> ESP32-S3 avantaji: Kart 7-36V destekledigi icin Enhance Auto kablosunun 12V cikisi dogrudan guc terminaline baglanir. DC/DC converter gerekmez. Diger kartlarda (ESP8266, ESP32) 12V’u 5V’a dusurmek icin converter sart.

#### X179 Konnektorune Erisim

> X179 konnektoru yolcu tarafi ayak boslugunda , sag panelin arkasindadir. Paneli hafifce cekerek cikarin (alet gerekmez). Konnektor kumesini bulun — X179 asagida isaretlenmistir.

> Legacy Model 3 (2020 ve oncesi): X179 bulunmayabilir. Bu durumda X652 konnektorunu kullanin — CAN-H pin 1, CAN-L pin 2.

#### Alternatif: Dogrudan X179 Baglanti

> Enhance Auto kablo olmadan dogrudan X179’a baglanabilirsiniz, ancak sadece CAN erisilebilir — 12V icin ayri kaynak gerekir.

| X179 Pin | Sinyal | Baglanti |
| --- | --- | --- |
| 13 | CAN-H | Board CAN terminal H |
| 14 | CAN-L | Board CAN terminal L |

> Guc icin: X930 Pin 1 (merkez konsol alti, daima acik 12V) veya torpido sigorta kutusu (ACC hatti) kullanin. Test icin USB-C de yeterlidir.

#### Pin Haritasi (Referans)

| GPIO | Fonksiyon | Not |
| --- | --- | --- |
| GPIO 15 | CAN TX (TWAI) | Dahili — baglamaya gerek yok |
| GPIO 16 | CAN RX (TWAI) | Dahili — baglamaya gerek yok |

#### X179 Konnektor Pin Detayi

| Pin | Sinyal | Kullanim |
| --- | --- | --- |
| 13 | CAN-H | Board CAN H terminali |
| 14 | CAN-L | Board CAN L terminali |
| 20 | GND | Board 7-36V terminal (−) |

#### Baglanti Semasi

12V+

GND

CAN-H

CAN-L

Board

Enhance Auto Cable

#### Tesla X179 Konnektor Detayi

> Tesla Part: 1849225-03-B (KSE K30M31014) — 20 kaviteli gri konnektor. 13 pin bos, sadece 7 pin kullanilir. Pin 20 = GND (0.50mm²), diger sinyaller 0.35mm².

[Image: X179 Konnektor Konumu]

X179 Arac Uzerindeki Konumu

[Image: X179 Face View]

X179 Konnektor Yuz Gorunumu (Face View)

[Image: X179 Pinout Tablosu]

| Pin | Sinyal | Kablo Rengi | Hedef |
| --- | --- | --- | --- |
| 2 | Sinyal | Sari/Acik Mavi | X051:13 |
| 3 | Sinyal | Sari | X051:29 |
| 9 | Sinyal | Kahve/Beyaz | X050:56 |
| 10 | Sinyal | Kahve | X050:55 |
| 13 | CAN-H | Koyu Yesil/Beyaz | X908M:13 |
| 14 | CAN-L | Yesil | X908M:14 |
| 20 | GND | Siyah (0.50mm²) | G032:8 |

#### Tesla X652 Konnektor (2020 oncesi Legacy Model 3)

> Tesla Part: 1015115-00-A (AMP 936119-1) — 4 pinli siyah konnektor. Tumu X052 gateway'e gider.

[Image: X652 Konnektor Konumu]

X652 Arac Uzerindeki Konumu

[Image: X652 Face View]

X652 Konnektor Yuz Gorunumu (Face View)

[Image: X652 Pinout Tablosu]

| Pin | Kablo Rengi | Hedef |
| --- | --- | --- |
| 1 | Sari | X052:45 |
| 2 | Yesil | X052:44 |
| 3 | Sari | X052:22 |
| 4 | Mavi | X052:20 |

### 🔌 Enhance Auto Kablo ile Kolay Baglanti Tek kablo ile CAN + guc — lehim/splice gerekmez

▼

> En kolay yontem: [Enhance Auto Tesla Gen 2 Cable](https://www.enhauto.com/products/tesla-gen-2-cable?variant=41214470094923) — S3XY Commander icin kullanilan ayni kablo. Tek konnektor ile hem CAN bus verisi hem 12V guc saglar. Lehim, splice veya jumper kablo gerekmez.

[Image: Enhance Auto Gen 2 Cable]

#### Kablo Yapisi

Kablo bir pass-through/splitter (gecis kablosu). Bir ucu Tesla X179'a takilir, diger ucu aracin orijinal kablo demetine geri baglanir (aracin fonksiyonlari calismaya devam eder). Alt ucunda S3XY Commander'a giden serbest konnektorler vardir.

#### Gerekli Parcalar

| Parca | Not |
| --- | --- |
| Enhance Auto Tesla Gen 2 Cable | [enhauto.com](https://www.enhauto.com/products/tesla-gen-2-cable?variant=41214470094923) |
| 12V/24V → 5V DC/DC Converter | USB-C veya Micro-USB — kartiniza gore |

#### Adim 1 — X179 Konnektore Erisin

X179 konnektoru yolcu tarafi ayak boslugunda , sag panelin arkasindadir.

- Sag ayak boslugu panelini cikartin — alet gerekmez, hafifce cekerek cikar

- Konnektor grubunu bulun. X179 asagida isaretlenen konnektordur

[Image: X179 konnektor konumu]

> Konnektoru bulmak ve kabloyu takmak icin [Enhance Auto kurulum videosunu](https://youtube.com/watch?v=ifwJNZgykVI) izleyin — en iyi gorsel referanstir.

> Legacy Model 3 (2020 ve oncesi): X179 konnektoru olmayabilir. Bu durumda [X652 konnektorunu](https://service.tesla.com/docs/Model3/ElectricalReference/prog-187/connector/x652/) kullanin — Pin 1 = CAN-H, Pin 2 = CAN-L.

#### Adim 2 — Kablo Pinout

Enhance Auto Gen 2 Cable'in bir ucu X179'a takilir, diger ucunda serbest kablolar vardir:

[Image: Enhance Auto kablo pinout]

| Kablo | Sinyal | Baglanti |
| --- | --- | --- |
| Kirmizi | 12V+ | DC/DC Converter IN+ |
| Siyah | GND | DC/DC Converter IN− |
| Siyah cizgili | CAN-H (Body Bus) | Feather CANH |
| Siyah duz | CAN-L (Body Bus) | Feather CANL |
| Kalan siyah cift | Diger Bus | Kullanilmaz — bos birakin |

[Image: X179 Konnektor Pinout ve Commander Kablo Renkleri]

X179 Pinout + Commander Kablo Renkleri

[Image: X179 Konnektor Face View — CAN-H Pin 13, CAN-L Pin 14]

X179 Face View — Pin 13 CAN-H, Pin 14 CAN-L

#### X179 Konnektor Pinout (Arac Tarafi)

| X179 Pini | Sinyal | Aciklama |
| --- | --- | --- |
| Pin 13 | Chassis CAN_P (CAN High) | Feather CANH ← bizim kullandigimiz |
| Pin 14 | Chassis CAN_N (CAN Low) | Feather CANL ← bizim kullandigimiz |
| Pin 1 | +12V (VCC) | Buck Converter IN+ ← guc kaynagi |
| Pin 20 | Ground (GND) | Buck Converter IN− & Feather GND |
| Pin 9 | Body CAN_P (CAN High) | Kullanilmaz — bos birakin |
| Pin 10 | Body CAN_N (CAN Low) | Kullanilmaz — bos birakin |

#### Commander Taraf Kablo Renkleri

> Commander konnektorundeki kablo renklerini asagidaki tabloyla eslestirebilirsiniz. Bazi Commander versiyonlarinda guc (VCC) icin ayri kablo kullanilir.

| Kablo Rengi | Sinyal | Baglanti |
| --- | --- | --- |
| Yesil | Chassis CAN_P (Pin 13) | Feather CANH |
| Sari | Chassis CAN_N (Pin 14) | Feather CANL |
| Kirmizi | +12V VCC (Pin 1) | Buck Converter IN+ |
| Siyah | GND (Pin 20) | Buck Converter IN− & Feather GND |
| Mavi | Body CAN_P (Pin 9) | Kullanilmaz |
| Diger siyah | Body CAN_N (Pin 10) | Kullanilmaz |

#### Baglanti Semasi

CAN-H

CAN-L

12V / 5V Guc

GND

UART TX

UART RX

Enhance Auto / Feather

#### Kablolari Tespit Etme

> Commander konnektorunu kesin veya pin-out'unu multimetre ile cikartin. Enhance Auto bu pinout'u resmi olarak dokumante etmemistir.

- Kirmizi kablo = 12V+ (multimetre ile dogrulayin)

- Siyah kablo = GND

- Kalan siyah kablolar = CAN cifti. CAN-H idle'da ~2.5V olur

- 2 CAN cifti vardir — Chassis CAN (Pin 13/14) ve Body CAN (Pin 9/10)

#### Adim 3 — Guc ve CAN Baglantisi

- CAN-H (siyah cizgili) ve CAN-L (siyah duz) kablolarini Feather'in CAN pinlerine baglayin

- 12V+ (kirmizi) ve GND (siyah) kablolarini DC/DC converter girisine baglayin

- DC/DC converter cikisini (USB-C veya Micro-USB) Feather'a baglayin

[Image: Baglanti gorunumu]

#### Adim 3.5 — ESP32-C3 WiFi Bridge Baglantisi

> RP2040 Feather'da dahili WiFi yoktur. Web arayuzu (192.168.4.1) icin ESP32-C3 Mini'yi UART ile baglamaniz gerekir. Feather CAN islemlerini yapar, ESP32-C3 sadece WiFi koprusu gorevindedir.

| RP2040 Feather | ESP32-C3 Mini | Not |
| --- | --- | --- |
| TX (GPIO0) | RX (GPIO20) | Feather → ESP32 |
| RX (GPIO1) | TX (GPIO21) | ESP32 → Feather |
| 3.3V | 3V3 | ESP32-C3 besleme |
| GND | GND | Ortak toprak |

> TX ↔ RX capraz baglanir. Ikisi de 3.3V logic — level shifter gerekmez. ESP32-C3'e ayrica ESP32C3_WiFiBridge.ino yuklenmelidir.

#### Adim 4 — Araca Takin

Enhance Auto kablosunu X179 konnektorune takin — klik sesiyle yerine oturur. Lehim, splice veya jumper gerekmez. Kablo tek konnektor ile hem CAN verisi hem 12V guc saglar — arac uyandigi anda kart acilir.

> Terminasyon direnci: Feather uzerindeki JP1 jumper'i cikartin veya R10 SMD direncini kaldirin. Aracin CAN bus'i zaten sonlandirilmis — ikinci 120Ω direnc iletisim hatalarina neden olur.

#### Enhance Auto Kablo Olmadan Dogrudan Baglanti

Enhance Auto kablo kullanmak istemiyorsaniz, [X179 konnektorune](https://service.tesla.com/docs/Model3/ElectricalReference/prog-233/connector/x179/) dogrudan baglanti yapabilirsiniz:

| Pin | Sinyal |
| --- | --- |
| 13 | CAN-H |
| 14 | CAN-L |

> Dogrudan baglantida gucu ayri saglamaniz gerekir (12V → Buck Converter → 5V). Enhance Auto kablo ise tek konnektorden hem CAN hem guc verir.

### 03 Arduino IDE Ayarlari Board + kutuphane

▼

Board Manager URL:

```text
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

| Ayar | Deger |
| --- | --- |
| Board | ESP32S3 Dev Module |
| USB CDC On Boot | Enabled |
| Flash Size | 16MB (128Mb) |
| PSRAM | OPI PSRAM |
| Kutuphane | ArduinoJson v6 (v7 degil!) |

> USB CDC On Boot: Enabled secilmezse Serial Monitor calismaz!

> Yuklemek icin: BOOT basili tut → RESET bas-birak → BOOT birak → Upload

> MCP2515 kutuphanesi GEREKMEZ. CAN bus ESP32-S3 dahili TWAI ile calisir.

### 04 Sorun Giderme

▼

| Hata | Cozum |
| --- | --- |
| Board gorulmuyor | USB CDC On Boot: Enabled. BOOT+RESET ile download moduna gir |
| TWAI baslatilamadi | ESP32S3 Dev Module secili mi? esp32 board paketi v3.0+? |
| CAN veri gelmiyor | Terminal vidalarini sikin. X179 Pin 13/14 kontrol |
| FSD aktif olmuyor | FSD aboneligi + Trafik Isigi Kontrolu acik olmali |
| WiFi gorunmuyor | Guc geliyor mu? USB-C veya 7-36V terminal kontrol |

- Board: ESP32S3 Dev Module + USB CDC On Boot: Enabled

- CAN-H → X179 Pin 13, CAN-L → X179 Pin 14

- Serial: "[can] TWAI hazir @ 500 kbps" goruldu

- WiFi: "CanFeather" → 192.168.4.1

### Arduino Kodu CanFeather_ESP32S3_TWAI.ino

▼

CanFeather_ESP32S3_TWAI.ino
ESP32-S3 TWAI — dahili CAN + WiFi

Kopyala
