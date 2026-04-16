# ESP32 DevKit V1

Board ID: esp32

## Categories
- 📚 Genel
- 🔧 Kurulum
- ⚙ Ayarlar
- </> Kod
- 🚗 Arac Kontrol

## Main Sections
- 📱 Neden ESP Kullaniyoruz? WiFi kontrol paneli — telefondan uzaktan yonetim
- ? Nasil Calisiyor? ESP8266 NodeMCU V3 — CAN bus enjeksiyon mantigi
- 🔋 BMS — Batarya Yonetim Sistemi Batarya izleme, preconditioning ve CAN diagnostik
- ⚡ Guc Kaynagi Secimi 12V → 5V Buck Converter Onerileri
- ⚙ Araca Montaj & Gizleme Yerlesim, kablo yonlendirme, servis proseduru
- ⚠ Guvenlik & Acil Durum Surerken sorun olursa
- ⚙ Tesla Firmware Uyumlulugu Versiyon kontrolu & guncelleme
- ✓ Hizli Baslangic 10 adimda kurulum ozeti
- $ FSD Abonelik Rehberi Fiyatlandirma, Kanada hesabi, EAP indirimi
- 01 Parca Listesi ESP32 + MCP2515
- 02 Calisma Modlari + OTA Aktif / Stealth + Firmware
- 03 HW Versiyonu Tespiti Legacy / HW3 / HW4
- 04 J1/R1 Terminasyon Direnci 120 Ohm — zorunlu
- 05 Kablo Baglantilari ESP32 ↔ MCP2515 + X179
- 🔌 Enhance Auto Kablo ile Baglanti S3XY Commander kablosu ile CAN + guc — tek konnektor
- ✓ Kurulum Dogrulamasi Araca takmadan once test edin
- 06 Arduino IDE + Kutuphaneler ESP32 board paketi + WebSockets
- 07 Kodu Yukleyin + OTA Kullanimi Ayarlar + OTA proseduru
- 08 Hiz Profilleri Takip mesafesiyle degisir
- 09 Sorun Giderme & Kontrol Listesi
- </> Arduino Kodu CanFeather_ESP32_WiFi.ino
- 🚗 Arac Kontrol Komutlari WiFi panelinden 10 ozellik

## Code IDs
- code-esp32

---

# ESP32 DevKit V1

## Categories
- 📚 Genel
- 🔧 Kurulum
- ⚙ Ayarlar
- </> Kod
- 🚗 Arac Kontrol

## Extracted Content

## ESP32 DevKit V1

MCP2515 + WiFi + OTA

📚 Genel

🔧 Kurulum

⚙ Ayarlar

</> Kod

### 📱 Neden ESP Kullaniyoruz? WiFi kontrol paneli — telefondan uzaktan yonetim

▼

Bu proje herhangi bir mikrodenetleyici ile calisabilir — ama biz ESP8266/ESP32 tabanli kartlari tercih ediyoruz cunku dahili WiFi sayesinde araci acmadan, telefonunuzdan tum sistemi yonetebilirsiniz. Kart araca monte edildikten sonra fiziksel erisim gerekmez.

#### Web Arayuzu Ne Saglar?

Telefonunuzdan veya bilgisayarinizdan "CanFeather" WiFi agina baglanip 192.168.4.1 adresini actiginizda asagidaki kontrol paneline erisirsiniz:

[Image: ESP Web Arayuzu — CanFeather kontrol paneli]

| Ozellik | Aciklama |
| --- | --- |
| Canli Durum Kutusu | CAN baglanti durumu, FSD aktif mi, NAG bastirma, RX/TX sayaclari, takip mesafesi ve hiz offset degerlerini canli gosterir |
| Donanim Secimi | Legacy / HW3 / HW4 — aracinizin Autopilot bilgisayar nesline gore secim yapilir |
| Hiz Profili | FSD'nin surus agresifligini belirler: Chill, Normal, Hurry (HW4'te ek olarak Max ve Sloth). Direksiyondaki takip mesafesi cubuguyla da otomatik degisir |
| FSD Etkin | FSD bit enjeksiyonunu ac/kapat — ana kontrol anahtari |
| Profili Buradan Uygula | Acildiginda CAN'dan gelen profil yok sayilir, web'den sectiginiz profil kullanilir |
| CAN Enjeksiyonu | Kapatildiginda kart tamamen seffaf olur — hicbir CAN mesajini degistirmez (guvenlik icin) |
| Guvenlik Butonu | Tek tusla tum enjeksiyonlari aninda durdurur |
| CAN Canli Gunluk | Islenen CAN mesajlarini gercek zamanli gosterir — debug ve dogrulama icin |
| Trafik Grafigi | Son 60 saniyenin RX trafigini sparkline grafik olarak gosterir — CAN bus aktivitesini anlik izleyin |
| FSD Durum Gecmisi | FSD ne zaman acildi/kapandi zaman cizelgesi — sorun tespiti icin gecmise bakin |
| Otomatik Kapanma | Belirlenen sure sonunda enjeksiyonu otomatik kapatir — guvenlik zamanlayicisi (0 = devre disi) |
| CAN Log Filtresi | Sadece belirli bir CAN ID'yi logla — ornegin sadece 1021 veya 1016 izleyin |
| CAN Mesaj Gonderme | Manuel CAN frame gonderme — ID, DLC ve hex data girerek test/debug yapin |
| Raw CAN Sniffer | Tum CAN tratigini yakalar ve gosterir — sadece bizim islemedigimiz ID'ler dahil, tam gorunurluk |
| EEPROM Kaydi | Ayarlarinizi kalici hafizaya kaydedin — reset/guc kesintisinde bile korunur |
| OTA Guncelleme | WiFi uzerinden firmware yukleyin — araci acmadan, 192.168.4.1/update adresinden .bin dosyasi ile |
| Uptime & Hata Sayaci | Cihaz ne zamandir calisiyor, kac hata oldu — canli durum kutusunda gorunur |

> Nasil Baglanilir: Telefonunuzun WiFi ayarlarindan "CanFeather" agini bulun (sifre: tesla1234 ). Baglantiktan sonra tarayicida http://192.168.4.1 adresini acin. Internet baglantiniz kesilir — bu normaldir, cihaz yerel ag olusturur.

> RP2040 Feather notu: RP2040'da dahili WiFi yoktur. WiFi icin yanina ESP32-C3 Mini baglanir (UART kopru). Ayni web arayuzu ESP32-C3 uzerinden sunulur.

### ? Nasil Calisiyor? ESP8266 NodeMCU V3 — CAN bus enjeksiyon mantigi

▼

1

#### CAN Mesajlarini Dinle

Arac surulurken Autopilot ECU surekli olarak CAN bus uzerinden mesaj yayinlar. Kod iki kritik mesaji dinler:

```text
CAN ID 1021 // AP_CONTROL — Autopilot kontrol komutlari
CAN ID 1016 // AP_FOLLOW_DIST — Takip mesafesi verisi
```

Her mesaj 8 byte (64 bit) veri tasir. Kod bunlari okuyup analiz eder.

2

#### Mux ID Kontrolu

Her AP_CONTROL mesajinin ilk byte'inin alt 3 biti bir "mux ID" tasir (0, 1 veya 2). Her mux ID farkli bir veri grubunu temsil eder. Kod her mux'a gore ayri islem yapar:

##### Mux 0 — FSD Bitini Ac

```text
// Byte 4, bit 6 → ekranda FSD secili mi?
fsdActive = isFSDSelectedInUI(f);

// Byte 5, bit 6 → FSD ENABLE bitini 1 yap
setBit(f, 46, true);

// Byte 6, bit 1-2 → hiz profilini yaz
writeSpeedProfile(f, speedProfile);
```

Bit 46 kritik bit. 8 byte'lik verinin 5. byte'indaki 6. bit'e denk gelir (46 / 8 = byte 5, 46 % 8 = bit 6). Tesla'nin Autopilot yazilimi bu bit'i kontrol eder — eger 1 ise FSD ozelliklerini aktif eder, 0 ise sadece temel Autopilot calisir. Normal sartlarda bu bit yalnizca FSD aboneligi olan araclarda 1 olur. Kod bunu zorla 1 yapar .

##### Mux 1 — NAG Bastirma

```text
// Byte 2, bit 3 → NAG SUPPRESS bitini 0 yap
setBit(f, 19, false);
```

"Direksiyonu tut" uyarisini (nag) kontrol eden bit. false yaparak uyariyi bastirir — boylece surekli direksiyon tutma hatirlatmasi gelmez.

##### Mux 2 — Hiz Offset

```text
// speedOffset (0-100) byte 0 ve byte 1'e bolunerek yazilir
f.data[0] = (f.data[0] & ~0xC0) | ((speedOffset & 0x03) << 6);
f.data[1] = (f.data[1] & ~0x3F) | (speedOffset >> 2);
```

FSD'nin hiz limitini ayarlar. speedOffset degeri (0-100 arasi) iki byte'a bolunerek yazilir. Kullanici scroll tekerlegiyle hizi ayarladiginda bu deger guncellenir.

3

#### Degistirilmis Mesaji Gonder

```text
canSend(&f); // Modifiye edilmis frame'i CAN bus'a geri gonder
```

canSend icinde iki guvenlik kontrolu var: cihaz MODE_ACTIVE modunda mi ve fsdEnabled acik mi? Ikisi de dogruysa mesaj gonderilir.

★

#### Hiz Profili (Bonus)

CAN ID 1016 (AP_FOLLOW_DIST) mesajindan takip mesafesi okunur ve buna gore hiz profili ( Chill / Normal / Hurry / Max ) belirlenir. Bu profil Mux 0'daki writeSpeedProfile ile AP_CONTROL mesajina yazilir.

> Ozet: Kod, Autopilot ECU'nun gonderdigi kontrol mesajindaki 46. bit'i (FSD enable flag) zorla 1'e cevirerek, Tesla'nin yazilimini "bu aracta FSD aktif" diye kandiriyor. Ayni anda 19. bit'i 0'a cekerek direksiyon uyarisini da bastiriyor.

#### HW4 — Ek Bit Farkliliklari

> HW4, HW3 ile ayni CAN ID'leri kullanir (0x3FD, 0x3F8). Fark ID'lerde degil, bit seviyesindedir:

##### Mux 0 — HW4 Ek Bitleri

```text
setBit(f, 46, true); // FSD enable (HW3 ile ayni)
setBit(f, 60, true); // FSD V14 flag (HW4'e ozel)
setBit(f, 59, true); // Acil arac algilama (HW4'e ozel)
```

##### Mux 1 — HW4 Ek Biti

```text
setBit(f, 19, false); // NAG suppress (HW3 ile ayni)
setBit(f, 47, true); // HW4 ek kontrol biti (HW4'e ozel)
```

##### Mux 2 — Speed Profile (Farkli Konum)

```text
// HW3: Byte 0-1'e split offset (6-bit, 0-100)
// HW4: Byte 7, bit 4-6'ya 3-bit profil (0-4)
f.data[7] = (f.data[7] & ~0x70) | ((speedProfile & 0x07) << 4);
```

> HW4 FSD v14: Firmware 2026.2.9+ gerektirir. 2026.8.X dali hala FSD v13'tedir — v13 kullaniyorsaniz (2026.8.X veya 2026.2.9 oncesi) HW4 donanimi olsa bile HW3 modunu secin.

#### CAN Mesaj Detay Tablolari

Asagidaki tablolar her HW versiyonu icin degistirilen CAN mesajlarinin sinyal isimlerini ve bit pozisyonlarini gosterir.

##### Legacy (Pre-AP / AP1)

| CAN ID | Sinyal Adi | Bit | Deger | Aciklama |
| --- | --- | --- | --- | --- |
| 0x399 | UI_autopilotStatus | 0-3 | 0x5 | FSD aktif goster |
| 0x399 | UI_autopilotHands | 4-5 | 0x0 | Nag bastir |
| 0x257 | DAS_steeringControl | 0-15 | Passthru | Direksiyon tork override |
| 0x118 | DI_speedMPS | 0-11 | Profil | Hiz limiti (5 profil) |
| 0x241 | DAS_longControl | 0-7 | Passthru | Boylamsal kontrol |

##### HW3 (FSD Computer)

| CAN ID | Sinyal Adi | Bit | Deger | Aciklama |
| --- | --- | --- | --- | --- |
| 0x33F | UI_applyEceR79 | 46 | 0→1 | FSD etkinlestir |
| 0x33F | UI_hardCoreSummon | 47 | 0→1 | Smart Summon AB kilidi ac |
| 0x33F | UI_autopilotHands | 4-5 | 0x0 | Nag bastir |
| 0x257 | DAS_steeringControl | 0-15 | Passthru | Direksiyon tork |
| 0x118 | DI_speedMPS | 0-11 | Profil | Hiz profili (5 seviye) |
| 0x241 | DAS_longControl | 0-7 | Passthru | ACC kontrol |
| 0x2B9 | DAS_bodyControls | 24-31 | Passthru | Govde kontrol sinyalleri |

##### HW4 (AI4)

| CAN ID | Sinyal Adi | Bit | Deger | Aciklama |
| --- | --- | --- | --- | --- |
| 0x33F | UI_applyEceR79 | 46 | 0→1 | FSD etkinlestir |
| 0x33F | UI_hardCoreSummon | 47 | 0→1 | Smart Summon kilidi ac |
| 0x33F | UI_autopilotHands | 4-5 | 0x0 | Nag bastir |
| 0x257 | DAS_steeringControl | 0-15 | Passthru | Direksiyon tork |
| 0x118 | DI_speedMPS | 0-11 | Profil | Hiz profili (5 seviye) |
| 0x241 | DAS_longControl | 0-7 | Passthru | ACC kontrol |
| 0x2B9 | DAS_bodyControls | 24-31 | Passthru | Govde kontrol |
| 0x352 | DAS_suppressSpeedWarning | 0 | 0→1 | ISA hiz uyarisi bastir |
| 0x352 | DAS_emergencyVehicle | 1-2 | 0x1 | Acil arac algilama aktif |
| 0x3A1 | UI_firmwareSync | 0-7 | 0xFF | Firmware uyumluluk bayragi |

### 🔋 BMS — Batarya Yonetim Sistemi Batarya izleme, preconditioning ve CAN diagnostik

▼

#### Batarya Izleme (BMS Monitoring)

Firmware, asagidaki CAN mesajlarini dinleyerek gercek zamanli batarya verisi toplar ve web arayuzunde gosterir.

- Voltaj, Akim & Guc — HV batarya durumu CAN ID 0x132 uzerinden okunur

- Sarj Durumu (SoC %) — Anlik sarj yuzdesi CAN ID 0x292 ile izlenir

- Sicaklik Min/Max — Batarya termal verileri CAN ID 0x312 uzerinden takip edilir

- Enerji Tuketimi (Wh/km) — Anlik tuketim degeri CAN ID 0x33A ile hesaplanir

- Web Arayuzu — SVG tabanli SoC ring gostergesi ve batarya hero karti ile goruntuleme

#### Batarya Preconditioning

Supercharger oncesi batarya isitma komutu CAN bus uzerinden gonderilir.

- Preconditioning Komutu — CAN ID 0x082 (UI_tripPlanning) mesaji ile batarya isitma baslatilir

- Web Kontrolu — Arayuzden Precond ON / OFF butonlari ile tek tikla kontrol

- Otomatik Gonderim — Aktifken 500 ms aralikla CAN mesaji tekrarlanir

#### CAN Bus Diagnostik

CAN bus saglik durumu web arayuzunden izlenebilir.

- Bus Durumu — RUNNING / BUS_OFF / RECOVERING durum gosterimi

- Hata Sayaclari — RX/TX errors, bus errors ve missed frames sayaclari

- Diagnostik Karti — Web arayuzunde canli CAN diagnostik ozet karti

#### BMS CAN ID Tablosu

| CAN ID | Decimal | Mesaj | Tip | Aciklama |
| --- | --- | --- | --- | --- |
| 0x132 | 306 | BMS_hvBusStatus | READ | Voltaj / Akim / Guc |
| 0x292 | 658 | BMS_socStatus | READ | Sarj % |
| 0x312 | 786 | BMS_thermalStatus | READ | Sicaklik min/max |
| 0x33A | 826 | BMS_rangeStatus | READ | Wh/km tuketim |
| 0x082 | 130 | UI_tripPlanning | WRITE | Preconditioning |

### ⚡ Guc Kaynagi Secimi 12V → 5V Buck Converter Onerileri

▼

> Tesla Model 3/Y 12V bataryasi kucuktur (~6.3Ah Li-ion). Cihaz daima acik kalacaksa dusuk bekleme akimi (quiescent current) kritik oneme sahiptir. Sarj sirasinda voltaj ~16V'a cikabilir — buck converter giris araliginin bunu karsilamasi gerekir.

#### ⭐ Onerilen: Mini560 (MP2315S)

| Ozellik | Deger |
| --- | --- |
| Chip | MP2315S (MPS) |
| Giris Aralik | 4.5V – 20V (16V Tesla spike'i karsilar) |
| Cikis | Sabit 5V (ayar gerektirmez) |
| Maks. Akim | 3A (sogutucusuz ~2A) |
| Boyut | 22 x 17 x 8 mm — cok kucuk |
| Bekleme Akimi | ~0.1 – 0.3 mA (mukemmel) |
| Verimlilik | ~%90 |
| Fiyat | ~$0.50 – $1.50 (AliExpress) / ~15 – 30 TL |

> Neden Mini560? 22x17mm boyutuyla proje kutusuna rahatca sigar, sabit 5V cikis (trimpot yok, kayma riski yok), cok dusuk bekleme akimi ile Tesla 12V bataryasini bosaltmaz, sarj spike'larini (16V) karsilar.

#### Alternatifler & Karsilastirma

| Modul | Chip | Giris | Cikis | Boyut | Iq | Fiyat | Yorum |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Mini560 | MP2315S | 4.5–20V | Sabit 5V | 22x17mm | ~0.2mA | ~15 TL | ⭐ En iyi secim |
| MP1584EN | MP1584EN | 4.5–28V | Ayarli | 22x17mm | ~0.8mA | ~10 TL | Ucuz ama trimpot kayabilir |
| Pololu D24V25F5 | TPS54360 | 4.5–38V | Sabit 5V | 18x13mm | ~0.1mA | ~250 TL | Premium kalite, enable pin |
| LM2596 | LM2596 | 4.5–40V | Ayarli | 43x21mm | ~8mA | ~15 TL | ❌ Cok buyuk, yuksek Iq |

> LM2596 kullanmayin! 43x21mm boyutuyla proje kutusuna sigmaz, ~8mA bekleme akimi Tesla 12V bataryasini bosaltir ve cogu ucuz modulde sahte chip vardir. Bench test icin uygundur, araca kalici montaj icin degildir.

#### Koruma Devreleri (Onerilen)

| Bilesen | Nerede | Neden |
| --- | --- | --- |
| 3A Sigorta + Tutucu | 12V giris hatti | Kisa devre korumasi — zorunlu |
| TVS Diyot (SMBJ18A) | Buck girisinde | Voltaj spike / load dump korumasi |
| Schottky Diyot veya MOSFET | Buck girisinde | Ters polarite korumasi |
| 100µF elektrolitik + 100nF seramik | Giris ve cikis | Ripple filtreleme (cogu modulde zaten var) |

#### Tesla 12V Kaynaklari

| Kaynak | Konum | Surekli? | Not |
| --- | --- | --- | --- |
| ⭐ X930 Pin 1 | Merkez konsol alti (teshis konnektoru) | Evet — daima acik | En ideal. Stealth mod batarya bosalmasini onler |
| ⭐ Torpido sigorta kutusu | Sol on kose (torpido alti) | Acc hatti — kontakla acilir | Kolay erisim, sigortali |
| X930 Pin 15 | Merkez konsol alti | USB besleme hatti | Kullanilabilir ama sinirli akim |
| 12V cakmak soketi | On konsol | Acc ile aktif | Sadece gecici test icin |

> Ozet: Mini560 (MP2315S, sabit 5V) + 3A sigorta + X930 Pin 1 veya torpido sigorta kutusu. Toplam maliyet: ~25-40 TL. Bu kombinasyon kucuk, verimli, guvenli ve Tesla 12V sistemiyle tam uyumlu.

### ⚙ Araca Montaj & Gizleme Yerlesim, kablo yonlendirme, servis proseduru

▼

#### X179 Konnektorune Erisim

> X179 konnektoru merkez konsol altinda , on koltuklar arasindaki panelin icinde bulunur. Erisim icin: konsol ust kapaklarini cikarin → yan panelleri sokerek kablo demetine ulasin. X179, mavi kablo demetinde 20 pinli gri konnektordur.

#### Onerilen Montaj Konumlari

| Konum | Avantaj | Dezavantaj |
| --- | --- | --- |
| Merkez konsol ici | X179'a en yakin, kisa kablo | Erisim icin sokum gerekir |
| Torpido arkasi (yolcu tarafi) | Genis alan, sigorta kutusuna yakin | Uzun CAN kablosu gerekir |
| A-sutunu kaplama arkasi | Tamamen gizli | Isi birikebilir, dar alan |

#### Kablo Yonlendirme Ipuclari

- CAN kablolarini (H/L) birbirine burun — twisted pair gurultu azaltir

- Guc kablolarini CAN kablolarindan ayri yonlendirin

- Mevcut kablo kanallarini ve klipsleri kullanin

- Keskin kenarlardan gecen kablolara spiral sargili koruyucu takin

- Fazla kabloyu dustuk dugumle toplayip cit ile sabitleyin

#### Tesla Servise Gitmeden Once

> Servis ziyareti oncesi cihazi devre disi birakin! Tesla servisi CAN bus taramasi yapar — aktif enjeksiyon tespit edilebilir.

| Adim | Islem |
| --- | --- |
| 1 | Web arayuzunden FSD enjeksiyonunu kapat |
| 2 | JST konnektorunu cekerek CAN hattini fiziksel olarak ayin |
| 3 | Guc kablosunu cikararak cihazi tamamen kapat |
| 4 | Servis sonrasi: ters sirada yeniden bagla ve test et |

### ⚠ Guvenlik & Acil Durum Surerken sorun olursa

▼

> Bu cihaz deneyseldir. CAN bus mesajlarini degistirmek arac davranisini etkiler. Her zaman direksiyona hakim olun ve acil duruma hazir olun.

#### Surerken Sorun Olursa

| Durum | Arac Davranisi | Yapmaniz Gereken |
| --- | --- | --- |
| FSD aniden devre disi | Arac normal suruse doner, fren/gaz sizde | Panik yapmayin — direksiyon ve fren calismaya devam eder |
| Ekranda hata mesaji | Autopilot gecici devre disi kalabilir | Araci guvenli sekilde kenara cekin, cihazi kontrol edin |
| CAN bus hatasi | Gostergede uyari isiklari yanabilir | JST konnektorunu cikararak cihazi aninda devre disi birakin |

#### Hizli Devre Disi Birakma

> En hizli yontem: JST konnektorunu cekin — CAN hatti aninda kesilir, cihaz pasif olur. Arac normal calismaya devam eder. Web arayuzunden de "Tum enjeksiyonlari devre disi birak" butonunu kullanabilirsiniz.

#### Yapilmamasi Gerekenler

- Cihaz aktifken firmware guncellemeyin

- Surerken web arayuzunde ayar degistirmeyin

- CAN kablolarini arac calisirken takip/sokmeyin

- Birden fazla CAN mod cihazini ayni anda kullanmayin

### ⚙ Tesla Firmware Uyumlulugu Versiyon kontrolu & guncelleme

▼

> Tesla OTA guncelleme ile CAN mesaj yapisini degistirebilir. Guncelleme sonrasi cihazin duzgun calistigini dogrulayin.

| HW | Minimum Firmware | Not |
| --- | --- | --- |
| Legacy (HW3 retrofit) | Tum versiyonlar | Model S/X, CAN ID 1006 |
| HW3 | Tum versiyonlar | Model 3/Y, CAN ID 1016/1021 |
| HW4 (FSD v14) | 2026.2.9+ | 2026.8.X dali = FSD v13, HW3 modu kullanin |

#### Tesla Guncellemesi Geldiginde

| Adim | Islem |
| --- | --- |
| 1 | Guncelleme oncesi web arayuzunden FSD'yi kapatin |
| 2 | Guncellemeyi yukleyin (cihaz stealth modda kalir) |
| 3 | Guncelleme sonrasi firmware versiyonunu kontrol edin |
| 4 | Kisa bir test surusu yapin — FSD'nin normal calistigini dogrulayin |
| 5 | Sorun varsa: HW modunu kontrol edin, topluluk forumlarini inceleyin |

### ✓ Hizli Baslangic 10 adimda kurulum ozeti

▼

| # | Adim | Detay |
| --- | --- | --- |
| 1 | HW versiyonunu belirle | Kontroller → Yazilim → Ek Arac Bilgisi → Autopilot Computer |
| 2 | Kart sec | ESP8266 (ucuz) / ESP32 (OTA) / RP2040+ESP32-C3 (en temiz) |
| 3 | Parcalari temin et | MCU + MCP2515 (veya Feather) + Mini560 buck + 3A sigorta + JST |
| 4 | Terminasyon direncini cikar | J1/R1/JP1 — multimetreyle CAN-H↔CAN-L arasi OL dogrula |
| 5 | Kablolama yap | SPI + CAN + Guc baglantilari — semaya bak |
| 6 | Arduino IDE kur | Board paketi + mcp2515 + ArduinoJson v6 kutuphaneleri |
| 7 | Kodu yukle | Kristal frekansini (8/16 MHz) ve HW modunu ayarla, yukle |
| 8 | Test et (masaustunde) | Serial Monitor'da "[ok] Hazir." gorulmeli, WiFi agina baglan |
| 9 | Araca monte et | 12V → Buck → 5V, CAN-H/L → X179 Pin 13/14, kutuyu sabitle |
| 10 | Surusten dogrula | WiFi'ya baglan, FSD'yi aktif et, test surusu yap |

> FSD aboneligi olmadan cihaz calismaz. Bolgenizde FSD yoksa, FSD sunulan bir ulkede (orn. Kanada) Tesla hesabi acip araci aktararak abonelik alabilirsiniz.

### $ FSD Abonelik Rehberi Fiyatlandirma, Kanada hesabi, EAP indirimi

▼

> Bu cihaz tek basina FSD'yi aktif etmez! Aracta aktif bir FSD aboneligi veya satin alimi olmalidir. Cihaz, mevcut FSD'nin bolge kisitlamasini asarak Turkiye/AB gibi FSD sunulmayan ulkelerde calismasini saglar.

#### FSD Nasil Alinir?

> Subat 2026 itibariyle Tesla, FSD satin alma secenegini kaldirdi. Artik sadece aylik abonelik ile kullanilabilir. Abonelik su bolgelerde mevcut: ABD, Kanada, Meksika, Avustralya, Yeni Zelanda . Avrupa'da henuz aktif degil (2026 ortasi bekleniyor).

#### Turkiye'den FSD Aboneligi Alma (Kanada Yontemi)

> Bolgenizde FSD aboneligi mevcut degilse, FSD sunulan bir ulkede (orn. Kanada) Tesla hesabi olusturup aracinizi o hesaba aktararak abonelik alabilirsiniz. Bu sayede aylik 99 CAD (~60 EUR) ile FSD kullanabilirsiniz.

##### Adim 1 — Kanada Tesla Hesabi Olusturun

| # | Islem |
| --- | --- |
| 1 | [tesla.com](https://www.tesla.com) adresine gidin |
| 2 | Sag ust koseden bolgeyi Canada / English olarak secin |
| 3 | Sag ustteki hesap ikonuna tiklayin |
| 4 | Create Account (Hesap Olustur) secenegine tiklayin |
| 5 | Bilgilerinizi doldurun ve hesabi tamamlayin. Kanada adresi girin (Google Maps'ten gercek bir adres secebilirsiniz) |

##### Adim 2 — Aracinizi Yeni Hesaba Aktarin

| # | Islem |
| --- | --- |
| 1 | Tesla uygulamasini acin (mevcut/eski hesabinizla giris yapmis olun) |
| 2 | Sag ustteki uc cizgi menusune dokunun |
| 3 | My Products (Urunlerim) secenegine gidin |
| 4 | Aktarmak istediginiz araci secin |
| 5 | Remove or Transfer Ownership (Sahipligi Kaldir veya Aktar) secenegine dokunun |
| 6 | Get Started (Basla) tusuna basin |
| 7 | "Sold the vehicle to an individual" (Araci bir bireye sattim) secenegini isaretleyin |
| 8 | Next (Ileri) tusuna basin |
| 9 | Yeni olusturdugunuz Kanada hesabinin bilgilerini girin |
| 10 | Dogrulama islemini tamamlayin |

> Dogrulama sonrasi araciniz yeni Kanada Tesla hesabinizda gorunmelidir.

##### Adim 3 — FSD Aboneligini Baslatin

| # | Islem |
| --- | --- |
| 1 | Tesla uygulamasini acin ve Kanada hesabinizla giris yapin |
| 2 | Upgrades (Yukseltmeler) bolumune gidin |
| 3 | Full Self-Driving (Supervised) aboneligini baslatin |

> Araciniz artik gecerli bir FSD yetkisine sahip. CAN mod karti, FSD'yi CAN bus seviyesinde etkinlestirebilir.

> Onemli: Bu resmi olarak desteklenen bir yontem degildir. Tesla politikalarini degistirebilir. Arac fiziksel olarak Kanada disinda olsa bile abonelik calisir, ancak bazi ozellikler bolgesel yazilim yapisiyla sinirli olabilir.

#### Fiyatlandirma

| Durum | Aylik Ucret | Yillik Maliyet | Not |
| --- | --- | --- | --- |
| Standart FSD Aboneligi | $99 CAD/ay | ~$1,188 CAD | ~72 USD / ~66 EUR |
| EAP Sahipleri (indirimli) | $49 CAD/ay | ~$588 CAD | %50 indirim! ~36 USD / ~33 EUR |

> EAP (Enhanced Autopilot) indirimi: Aracinda daha once EAP satin alinmissa, FSD aboneligi yarim fiyatina ($49 CAD/ay) gelir. EAP zaten Navigate on Autopilot, Auto Lane Change ve Autopark icerir — FSD sadece sehir ici surusu ekler. EAP'in olup olmadigini kontrol etmek icin: Tesla Uygulama → Yukseltmeler → Yazilim Yukseltmeleri.

#### FSD Supervised vs. Unsupervised

| Ozellik | FSD Supervised (Mevcut) | FSD Unsupervised (Gelecek) |
| --- | --- | --- |
| Surucu dikkat | Surekli gerekli, eller direksiyonda | Gerekli degil |
| Sorumluluk | Surucude | Tesla'da |
| Sehir ici surusu | ✓ Aktif | ✓ Aktif |
| Otoyol navigasyonu | ✓ Aktif | ✓ Aktif |
| Otonom park | ✓ Aktif | ✓ Aktif |
| Smart Summon | ✓ Aktif (AB kisitlamasi cihazla asilir) | ✓ Aktif |
| Tamamen surucusuz | ❌ Hayir | ✓ Evet |
| Mevcut mu? | Evet (ABD, Kanada, MX, AU, NZ) | 2026 sonu bekleniyor |

#### FSD Abonelik Detaylari

- Abonelik istedigin zaman iptal edilebilir — donem sonuna kadar aktif kalir

- Kismi ay icin iade veya orantili ucret yok

- Abonelik hesaba bagli, araca degil — arac satilirsa yeni sahip kendi aboneligini almali

- Satin alma secenegi Subat 2026'da kaldirildi — sadece abonelik mevcut

- Elon Musk FSD fiyatinin FSD yetenekleri gelistikce artacagini uyardi

- Avrupa'da FSD Hollanda'dan baslayarak 2026 ortasi onay bekleniyor

> Ozet: Turkiye'den FSD kullanmak icin: (1) Kanada Tesla hesabi ac, (2) araci aktar, (3) $99 CAD/ay abonelik al (EAP varsa $49), (4) bu cihazi monte et. Cihaz FSD'nin bolge kisitlamasini asar, ama abonelik olmadan hicbir sey yapmaz.

### 01 Parca Listesi ESP32 + MCP2515

▼

> X930 Pin 1 arac kapaliyken dahi 12V verir — otomatik stealth modunu aktif birakin.

> MCP2515 VCC → 5V (VIN)! TJA1050 5V gerektirir. 3.3V baglamayin. ESP32'de VIN pini genelde 5V verir. Calismiyorsa 5V pinini dogrudan kullanin.

#### Temel Parcalar

| # | Parca | Model | Fiyat | Link |
| --- | --- | --- | --- | --- |
| 1 | Mikrodenetleyici | ESP32 DevKit V1 (30-pin, ESP-WROOM-32) | ~60-100 TL | [Robotistan](https://www.robotistan.com/esp32-esp-32s-wifi-bluetooth-dual-mode-gelistirme-karti) · [Direnc.net](https://www.direnc.net/esp32-wroom-32d-wifi-bluetooth-gelistirme-board) |
| 2 | CAN Controller | MCP2515 + TJA1050 Modulu (8MHz) | ~40-80 TL | [Robotistan](https://www.robotistan.com/mcp2515-canbus-spi-haberlesme-modulu) · [Direnc.net](https://www.direnc.net/mcp2515-modul) |
| 3 | Jumper Kablo | Disi-Disi, 10cm | ~10 TL | [Robotistan](https://www.robotistan.com/40-pin-ayrilabilen-disi-disi-f-f-jumper-kablo-200-mm) |
| 4 | JST-XH 2-pin | CAN hizli sokum | ~5 TL | [Robotistan](https://www.robotistan.com/i-ds1066-scw002-2-pin-jst-terminalsiz-fis) |
| 5 | Proje Kutusu | 80x50x25mm | ~20 TL | [Robotistan](https://www.robotistan.com/54-x-84-x-32-el-tipi-kutu-5921) |

#### Guc Kaynagi

| # | Parca | Model / Not | Fiyat | Link |
| --- | --- | --- | --- | --- |
| 10 | Buck Converter | Mini560 (MP2315S) sabit 5V — giris 4.5-20V, 22x17mm | ~15-30 TL | [Robotistan](https://www.robotistan.com/mini560-5v-dc-dc-voltaj-dusurucu) · [AliExpress](https://tr.aliexpress.com/w/wholesale-mini560-5v.html) |
| 11 | Sigorta Tutucu + 3A | Arac hatti korumasi | ~10 TL | — |
| 12 | Disi JST-XH 2-pin (x2) | 12V giris + 5V cikis | ~5 TL | [Robotistan](https://www.robotistan.com/i-ds1066-scw002-2-pin-jst-terminalsiz-fis) |

#### Guc Kaynaklari

| Kaynak | Konum | Surekli? | Uygunluk |
| --- | --- | --- | --- |
| Torpido sigorta kutusu | Sol on kose | Evet (Acc) | ⭐ Onerilen |
| X930 Pin 1 | Merkez konsol alti | Evet (daima) | ⭐ Ideal |
| X930 Pin 15 | Merkez konsol alti | USB besleme | Kullanilabilir |
| 12V cakmak | On konsol | Acc ile aktif | Gecici test |

### 02 Calisma Modlari + OTA Aktif / Stealth + Firmware

▼

> OTA Guncelleme (sadece ESP32): http://192.168.4.1/update adresinden .bin dosyasi yukleyin. Cihazi sokmeden firmware guncelleyebilirsiniz.

### 03 HW Versiyonu Tespiti Legacy / HW3 / HW4

▼

> Kontroller → Yazilim → Ek Arac Bilgisi

### 04 J1/R1 Terminasyon Direnci 120 Ohm — zorunlu

▼

> Tesla CAN bus zaten sonlandirilmis. Ek 120 Ohm direnci eklerseniz sinyal bozulur.

### 05 Kablo Baglantilari ESP32 ↔ MCP2515 + X179

▼

| MCP2515 | ESP32 | Tesla X179 | Not |
| --- | --- | --- | --- |
| VCC | VIN (5V) | — | 5V zorunlu! VIN calismiyorsa VU pinini deneyin |
| GND | GND | — | Toprak |
| CS | GPIO5 | — | SPI CS |
| SCK | GPIO18 | — | SPI Clock |
| SO/MISO | GPIO19 | — | SPI MISO |
| SI/MOSI | GPIO23 | — | SPI MOSI |
| CAN-H | — | Pin 13 | JST kirmizi |
| CAN-L | — | Pin 14 | JST mavi |

#### 12V → 5V Guc Baglantisi

> Arac 12V → Buck Converter → 5V → ESP32 VIN. Dogrudan 12V baglamayin, kart yanar! Buck converter cikisini multimetreyle 5.0V olarak ayarlayin.

| Kaynak | Hedef | Not |
| --- | --- | --- |
| Tesla 12V → Sigorta (3A) | Buck IN+ | Torpido sigorta kutusu veya X930 Pin 1 |
| Tesla GND | Buck IN− | Arac sasisi |
| Buck OUT+ ( 5V ) | ESP32 VIN | Multimetreyle 5.0V ayarla |
| Buck OUT− | ESP32 GND | |

> X930 Pin 1 arac kapaliyken de 12V verir (daima acik). Torpido sigorta kutusu ise ACC hatti — kontak acikken aktif. Stealth mod sayesinde X930 Pin 1 kullanilsa bile batarya bosalmaz.

#### Baglanti Semasi

5V Guc

GND

SPI (Veri)

CAN-H

CAN-L

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

### 🔌 Enhance Auto Kablo ile Baglanti S3XY Commander kablosu ile CAN + guc — tek konnektor

▼

> [Enhance Auto Tesla Gen 2 Cable](https://www.enhauto.com/products/tesla-gen-2-cable?variant=41214470094923) — S3XY Commander icin tasarlanmis bu kablo, Tesla X179 konnektorune takilarak hem CAN bus verisi hem 12V guc saglar.

[Image: Enhance Auto Gen 2 Cable]

#### Kablo Yapisi

Kablo bir pass-through/splitter . Bir ucu X179'a takilir, diger ucu aracin kablo demetine geri baglanir. Serbest ucundaki Commander konnektorunu keserek veya problaarak kablolari tespit edin.

#### Pinout

[Image: X179 Konnektor Pinout ve Commander Kablo Renkleri]

X179 Pinout + Commander Kablo Renkleri

[Image: X179 Konnektor Face View — CAN-H Pin 13, CAN-L Pin 14]

X179 Face View — Pin 13 CAN-H, Pin 14 CAN-L

#### X179 Konnektor Pinout (Arac Tarafi)

| X179 Pini | Sinyal | Aciklama |
| --- | --- | --- |
| Pin 13 | Chassis CAN_P (CAN High) | MCP2515 CAN-H ← bizim kullandigimiz |
| Pin 14 | Chassis CAN_N (CAN Low) | MCP2515 CAN-L ← bizim kullandigimiz |
| Pin 1 | +12V (VCC) | Buck Converter IN+ ← guc kaynagi |
| Pin 20 | Ground (GND) | Buck Converter IN− & ESP32 GND |
| Pin 9 | Body CAN_P (CAN High) | Kullanilmaz |
| Pin 10 | Body CAN_N (CAN Low) | Kullanilmaz |

#### Commander Taraf Kablo Renkleri

| Kablo Rengi | Sinyal | Baglanti |
| --- | --- | --- |
| Yesil | Chassis CAN_P (Pin 13) | MCP2515 CAN-H |
| Sari | Chassis CAN_N (Pin 14) | MCP2515 CAN-L |
| Kirmizi | +12V VCC (Pin 1) | Buck Converter IN+ |
| Siyah | GND (Pin 20) | Buck Converter IN− & ESP32 GND |
| Mavi | Body CAN_P (Pin 9) | Kullanilmaz |
| Diger siyah | Body CAN_N (Pin 10) | Kullanilmaz |

#### Baglanti Semasi

CAN-H

CAN-L

12V / 5V Guc

GND

SPI

Enhance Auto Kablo

#### Kablolari Tespit Etme

- Kirmizi = 12V, Siyah = GND (multimetre ile dogrulayin)

- Kalan siyah kablolar = CAN cifti. CAN-H idle'da ~2.5V

- 2 CAN cifti var — Chassis CAN (Pin 13/14) ve Body CAN (Pin 9/10)

> Terminasyon: MCP2515 J1 jumper'i cikartin. Tesla CAN bus zaten sonlandirilmis.

### ✓ Kurulum Dogrulamasi Araca takmadan once test edin

▼

> Araca monte etmeden once masaustunde dogrulama yapin. Sorunlari tespit etmek icin karta USB ile baglanip Serial Monitor izleyin.

| # | Islem | Beklenen Sonuc |
| --- | --- | --- |
| 1 | Karti USB ile laptopa baglayin ve Serial Monitor acin (115200 baud) | [can] MCP2515 hazir @ 500 kbps gorunur |
| 2 | WiFi: "CanFeather" agina baglanin, 192.168.4.1 acin | Web arayuzu yuklenmelidir |
| 3 | Aracin Autopilot ayarlarindan "Trafik Isigi ve Dur Isareti Kontrolu" acin | Bu ayar FSD tetikleyicisidir — acik olmalidir |
| 4 | Direksiyondaki takip mesafesi cubuguyla mesafeyi degistirin | Serial'de CAN mesajlari gorunur, profil degeri degisir |
| 5 | Her sey calisiyorsa: USB'yi cikartin, DC/DC converter ile guc verin | Cihaz arac 12V ile calismaya baslar |

> Onemli: "Trafik Isigi ve Dur Isareti Kontrolu" ayari FSD'nin tetikleyicisidir. Bu ayar acik olmalidir — kod bu ayarin CAN bus'taki karsiligini (Byte 4, Bit 6) okuyarak FSD enjeksiyonunu baslatir.

### 06 Arduino IDE + Kutuphaneler ESP32 board paketi + WebSockets

▼

> ArduinoJson v6.x secin. WebSockets: "WebSockets by Markus Sattler" v2.x kurun.

| Kutuphane | Yazar | Versiyon |
| --- | --- | --- |
| mcp2515 | autowp | son surum |
| ArduinoJson | Blanchon | v6.x ! |
| WebSockets | Markus Sattler | v2.x |
| WiFi / WebServer / EEPROM / Update | Espressif | dahili |

Board Manager URL:

```text
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

### 07 Kodu Yukleyin + OTA Kullanimi Ayarlar + OTA proseduru

▼

```text
// Kristal frekansi
#define CAN_CRYSTAL MCP_8MHZ // veya MCP_16MHZ

// WiFi
#define WIFI_SSID "TeslaCANMod"
#define WIFI_PASS "teslamod2026"
```

Basarili yukleme ciktisi:

```text
=== CanFeather ESP32 v3.1 ===
[eeprom] hw=1 p=2 fsd=1
[wifi] TeslaCANMod 192.168.4.1
[web] http://192.168.4.1 | OTA: http://192.168.4.1/update
[ws] WebSocket :81 basladi
[can] MCP2515 hazir @ 500 kbps
[mode] AKTIF
[ok] Hazir.
```

### 08 Hiz Profilleri Takip mesafesiyle degisir

▼

| Takip Mesafesi | HW3 Profili | HW4 Profili |
| --- | --- | --- |
| 2 | ⚡ Hurry | 🔥 Max |
| 3 | 🟢 Normal | ⚡ Hurry |
| 4 | ❄️ Chill | 🟢 Normal |
| 5 | — | ❄️ Chill |
| 6 | — | 🐢 Sloth |

### 09 Sorun Giderme & Kontrol Listesi

▼

| Sorun | Cozum |
| --- | --- |
| setBitrate basarisiz | MCP_8MHZ / MCP_16MHZ degistir · VCC → 5V kontrol |
| WebSocket baglanamıyor | Tarayicida ws:// destegi · Sayfayi yenile |
| OTA yukleme basarisiz | Dogru .bin dosyasi? Yeterli flash? |
| ArduinoJson hatasi | v6.x kur |
| FSD aktif olmuyor | Aracta aktif FSD aboneligi gerekli |

#### Kontrol Listesi

- J1/R1 terminasyon direnci cikarildi — OL dogrulandi

- MCP2515 VCC → VIN (5V)'e baglandi

- GPIO5→CS · GPIO18→SCK · GPIO19→MISO · GPIO23→MOSI

- CAN-H → X179 Pin 13 · CAN-L → X179 Pin 14

- Kristal frekansi dogrulandi

- ESP32 Dev Module board secildi (240MHz, 4MB)

- mcp2515 + ArduinoJson v6 + WebSockets v2 kuruldu

- Serial Monitor'da "[ok] Hazir." goruldu

- 192.168.4.1 acildi, kirmizi LED yaniyor

- Ayarlar kaydedildi (EEPROM)

- FSD aboneligi aktif · Trafik Isigi Kontrolu acik

### Arduino Kodu CanFeather_ESP32_WiFi.ino

▼

CanFeather_ESP32_WiFi.ino

Kopyala

### 🚗 Arac Kontrol Komutlari WiFi panelinden 10 ozellik

▼

> Bu icerik ESP8266 sekmesindeki "Arac Kontrol" ile aynidir. Tum sinyal detaylari, komut listesi ve WiFi sifre degistirme rehberi icin ESP8266 → Arac Kontrol sekmesine bakin. Firmware komutlari ve CAN ID'ler tum platformlarda ortaktir.

> ESP32 farki: Ayni ozellikler, ayni komutlar. 192.168.4.1/cmd?c=mirror_fold seklinde kullanilir. WiFi ayarlari /wifi endpoint'inden degistirilir.
