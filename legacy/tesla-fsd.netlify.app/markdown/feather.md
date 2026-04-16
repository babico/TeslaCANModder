# RP2040 Feather + ESP32-C3 Bridge

Board ID: feather

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
- 01 Parca Listesi Adafruit RP2040 CAN Feather #5724
- 02 Calisma Modlari WiFi + Serial kontrol
- 03 HW Versiyonu Tespiti Legacy / HW3 / HW4
- 04 JP1 Terminasyon Direnci 120 Ohm — cikarilmali
- 05 Kablo Baglantilari Feather ↔ ESP32-C3 ↔ Tesla X179
- 🔌 Enhance Auto Kablo ile Kolay Baglanti Tek kablo ile CAN + guc — lehim/splice gerekmez
- 06 Arduino IDE + Kutuphaneler RP2040 board paketi
- ✓ Kurulum Dogrulamasi Araca takmadan once test edin
- 11 Sorun Giderme & Kontrol Listesi
- 07 Kodu Yapilandirin ve Yukleyin HW modu secimi
- 08 V14 Yeni Ozellikler Son guncellemeyle eklenen CAN ozellikleri
- 09 Hiz Profilleri Takip mesafesiyle otomatik degisir
- </> Arduino Kodu CanFeather_RP2040.ino
- 🚗 Arac Kontrol Komutlari WiFi panelinden 10 ozellik

## Code IDs
- code-feather
- code-esp32c3

---

# RP2040 Feather + ESP32-C3 Bridge

## Categories
- 📚 Genel
- 🔧 Kurulum
- ⚙ Ayarlar
- </> Kod
- 🚗 Arac Kontrol

## Extracted Content

## RP2040 Feather + ESP32-C3 Bridge

UART Bridge

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

### 01 Parca Listesi Adafruit RP2040 CAN Feather #5724

▼

> MCP2515 dahili — harici CAN modulu gerekmez. ESP32-C3 Mini UART ile baglanarak WiFi AP + web arayuzu saglar.

#### Temel Parcalar

| # | Parca | Model | Fiyat | Link |
| --- | --- | --- | --- | --- |
| 1 | Mikrodenetleyici + CAN | Adafruit RP2040 CAN Bus Feather #5724 | ~$20 | [e-Komponent](https://www.e-komponent.com/adafruit-5724-rp2040-can-bus-feather) |
| 2 | JST-XH 2-pin | CAN hatti hizli sokum | ~5 TL | [Robotistan](https://www.robotistan.com/i-ds1066-scw002-2-pin-jst-terminalsiz-fis) |
| 3 | Proje Kutusu | 80x50x25mm | ~20 TL | [Robotistan](https://www.robotistan.com/54-x-84-x-32-el-tipi-kutu-5921) |

#### WiFi Modulu

| # | Parca | Model | Fiyat | Link |
| --- | --- | --- | --- | --- |
| 4 | WiFi Modulu | ESP32-C3 Super Mini (Lolin / WeAct) | ~30–50 TL | [Direnc.net](https://www.direnc.net/esp32-c3-mini-wifi-bluetooth-board) |
| 5 | Jumper Kablo (4x) | Disi-disi dupont 10cm | ~5 TL | [Robotistan](https://www.robotistan.com/40-pin-ayrilabilen-disi-disi-f-f-jumper-kablo-200-mm) |

#### Guc Kaynagi

| # | Parca | Model | Fiyat | Link |
| --- | --- | --- | --- | --- |
| 6 | Buck Converter | Mini560 (MP2315S) sabit 5V — 22x17mm | ~15-30 TL | [Robotistan](https://www.robotistan.com/mini560-5v-dc-dc-voltaj-dusurucu) · [AliExpress](https://tr.aliexpress.com/w/wholesale-mini560-5v.html) |
| 7 | Sigorta Tutucu + 3A | Arac hatti korumasi | ~10 TL | — |

### 02 Calisma Modlari WiFi + Serial kontrol

▼

> ESP32-C3 Mini ekleyerek WiFi AP + web arayuzu (192.168.4.1) saglanir. ESP32-C3 olmadan Serial Monitor (115200 baud) ile de kontrol edilebilir.

| Komut | Aciklama |
| --- | --- |
| hw0 / hw1 / hw2 | Legacy / HW3 / HW4 modu sec |
| p0 – p4 | Hiz profili sec (0=Chill, 4=Max) |
| on / off | FSD enjeksiyonu ac/kapat |
| status | Mevcut ayarlari goster |
| log | Son 24 CAN mesajini goster |

### 03 HW Versiyonu Tespiti Legacy / HW3 / HW4

▼

> Kontroller → Yazilim → Ek Arac Bilgisi

> HW4 FSD v14: Approaching Emergency Vehicle Detection. Firmware 2026.2.9+ gerekir. 2026.8.X dali veya 2026.2.9 oncesi surumlerde HW3 secin .

| Versiyon | Arac | CAN ID | Kod |
| --- | --- | --- | --- |
| Legacy | Model S/X — dikey ekran, HW3 retrofit | 1006, 69 | #define HW 0 |
| HW3 | Model 3/Y — yatay ekran, HW3 chip | 1016, 1021 | #define HW 1 |
| HW4 | Yeni Model 3/Y — HW4 chip | 1016, 1021, 921 | #define HW 2 |

#### HW4 vs HW3 — CAN Mimarisi Karsilastirmasi

> HW4, HW3 ile ayni CAN ID'leri kullanir (0x3FD = 1021, 0x3F8 = 1016). Fark ID'lerde degil, bit seviyesindedir . Donanim baglantisi (X179 Pin 13/14) her iki nesil icin aynidir.

| Ozellik | HW3 | HW4 |
| --- | --- | --- |
| CAN ID'ler | 1016, 1021 | 1016, 1021 + 921 (ISA) |
| Mux 0 — Bit 46 | FSD enable | FSD enable (ayni) |
| Mux 0 — Bit 60 | — | FSD V14 flag |
| Mux 0 — Bit 59 | — | Acil arac algilama |
| Mux 1 — Bit 19 | NAG suppress | NAG suppress (ayni) |
| Mux 1 — Bit 47 | — | HW4 ek kontrol biti |
| Mux 2 — Speed Profile | Byte 0-1 split (6-bit offset) | Byte 7, bit 4-6 (3-bit profil) |
| ISA Chime Suppress | — | CAN ID 921 (opsiyonel) |
| Min. Firmware | Tum surumler | 2026.2.9+ (FSD v14). 2026.8.X = v13, HW3 kullanin |

> Neden ayni CAN ID? Tesla'nin Autopilot ECU'su nesiller arasi ayni mesaj yapisini (AP_CONTROL = 0x3FD, AP_FOLLOW_DIST = 0x3F8) korur. HW4'te mevcut bit alanlarina yeni flag'ler (bit 59, 60, 47) eklenmis ve speed profile daha verimli bir alana tasınmıstır. Bu tasarım geriye uyumlulugu saglar — eski firmware HW4'te bilinmeyen bitleri yok sayar.

### 04 JP1 Terminasyon Direnci 120 Ohm — cikarilmali

▼

> JP1 jumper sokulerek cikar veya R10 SMD direncini kaldir. Feather uzerinde 120 Ohm terminasyon var. Tesla CAN bus zaten sonlandirilmis. Multimetreyle CAN-H ↔ CAN-L arasi OL okumali.

### 05 Kablo Baglantilari Feather ↔ ESP32-C3 ↔ Tesla X179

▼

> MCP2515 Feather'a dahili — CAN icin 2 kablo. WiFi icin ESP32-C3 Mini UART ile baglanir.

#### CAN Baglantisi

| Feather | Tesla X179 | Not |
| --- | --- | --- |
| CANH | Pin 13 | CAN High |
| CANL | Pin 14 | CAN Low |

#### Guc Baglantisi

| Kaynak | Hedef | Not |
| --- | --- | --- |
| Tesla 12V → Sigorta (3A) | Buck IN+ | Torpido veya X930 Pin 1 |
| Tesla GND | Buck IN− | |
| Buck OUT+ (5V) | Feather USB pini | Multimetreyle 5.0V ayarla |
| Buck OUT− | Feather GND | |

#### UART Baglantisi (ESP32-C3 WiFi)

| RP2040 Feather | ESP32-C3 Mini | Not |
| --- | --- | --- |
| TX (GPIO0) | RX (GPIO20) | Feather → ESP32 veri |
| RX (GPIO1) | TX (GPIO21) | ESP32 → Feather veri |
| 3.3V | 3V3 | ESP32-C3 besleme |
| GND | GND | Ortak toprak |

> Capraz baglanti: TX ↔ RX, RX ↔ TX. Ikisi de 3.3V logic — level shifter gerekmez.

#### Baglanti Semasi

5V Guc / 12V

GND

CAN-H

CAN-L

Feather gövde

UART TX

UART RX

[X179 Pin Diyagrami →](https://service.tesla.com/docs/Model3/ElectricalReference/prog-233/connector/x179/)

[X652 (2020 oncesi Legacy Model 3) →](https://service.tesla.com/docs/Model3/ElectricalReference/prog-187/connector/x652/)

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

### 06 Arduino IDE + Kutuphaneler RP2040 board paketi

▼

Board Manager URL:

```text
https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
```

| Ayar | Deger |
| --- | --- |
| Board | Adafruit Feather RP2040 |
| Kutuphane 1 | mcp2515 by autowp |
| Kutuphane 2 | ArduinoJson v6 (v7 degil) |

> ESP32-C3 icin: Arduino IDE'ye ESP32 board paketini ekleyin. Board: ESP32C3 Dev Module . Ayri firmware yuklenir (ESP32C3_WiFiBridge.ino).

### ✓ Kurulum Dogrulamasi Araca takmadan once test edin

▼

> Araca monte etmeden once masaustunde dogrulama yapin. Sorunlari tespit etmek icin karta USB ile baglanip Serial Monitor izleyin.

| # | Islem | Beklenen Sonuc |
| --- | --- | --- |
| 1 | DC/DC converter'i cikartin ve Feather'a USB-C ile laptopunuzu baglayin | Board acilir, LED yanar |
| 2 | Arduino IDE'de Serial Monitor acin (115200 baud) | [can] MCP2515 hazir @ 500 kbps mesaji gorunur |
| 3 | Serial'e status yazin | JSON cikti: {"hw":1,"profile":2,"fsd":true,...} |
| 4 | Aracin Autopilot ayarlarindan "Trafik Isigi ve Dur Isareti Kontrolu" secenegini acin | Bu ayar FSD'nin tetiklenmesi icin gereklidir |
| 5 | Direksiyondaki takip mesafesi cubuguyla mesafeyi degistirin | Serial'de HW3 id=1016 fd=X gorunur, profil degeri degisir |
| 6 | WiFi: "CanFeather" agina baglanin, 192.168.4.1 acin | Web arayuzu yuklenmelidir (ESP32-C3 bagli olmalidir) |
| 7 | Her sey calisiyorsa: USB'yi cikartin, DC/DC converter'i geri takin | Cihaz arac 12V ile calismaya baslar |

> Onemli: "Trafik Isigi ve Dur Isareti Kontrolu" ayari FSD'nin tetikleyicisidir. Bu ayar acik olmalidir — kod bu ayarin CAN bus'taki karsiligini (Byte 4, Bit 6) okuyarak FSD enjeksiyonunu baslatir.

> Serial Monitor uzerinden canli log izleyebilirsiniz. logon / logoff komutlariyla loglama acilip kapatilabilir. CAN trafigi gorunuyorsa ve FSD bitleri yaziliyorsa kurulum basarilidir.

### 11 Sorun Giderme & Kontrol Listesi

▼

| Hata | Cozum |
| --- | --- |
| setBitrate basarisiz | CAN_CRYSTAL MCP_16MHZ kontrol et |
| SPI1 derleme hatasi | Adafruit Feather RP2040 sec (generic degil) |
| Board gorunmuyor | USB-C kabloyu degistir (data kablo olmali) |
| CAN veri gelmiyor | JP1/R10 cikar — terminasyon |
| FSD aktif olmuyor | FSD aboneligi + Trafik Isigi Kontrolu ac |
| WiFi agi gorunmuyor | ESP32-C3 UART + guc baglantisini kontrol et |
| Web arayuzu yanit vermiyor | TX/RX capraz mi? RP2040 TX → ESP32 RX olmali |

#### Kontrol Listesi

- JP1 jumper veya R10 cikarildi (terminasyon acik)

- CANH → X179 Pin 13, CANL → X179 Pin 14

- 12V → 5V buck → Feather USB pini

- Board: Adafruit Feather RP2040 secildi

- mcp2515 (autowp) + ArduinoJson v6 kuruldu

- Serial: "[can] MCP2515 hazir @ 500 kbps" goruldu

- status komutu: hw=1 dogrulandi

- FSD aboneligi aktif, Trafik Isigi Kontrolu acik

- ESP32-C3: TX↔RX capraz baglanti, 3.3V + GND

- WiFi: "CanFeather" agina baglanildi, 192.168.4.1 acildi

### 07 Kodu Yapilandirin ve Yukleyin HW modu secimi

▼

> Upload sonrasi Serial Monitor'da [can] MCP2515 hazir @ 500 kbps gorunmeli.

| Satir | Deger | Ne icin |
| --- | --- | --- |
| uint8_t hw = 1; | 0 / 1 / 2 | Legacy / HW3 / HW4 |
| uint8_t speedProfile = 2; | 0–4 | Baslangic profili |
| bool fsdEnabled = true; | true / false | FSD varsayilan |

#### HW4 Ek Derleme Opsiyonlari

> Bu ayarlar sadece HW4 modunda gecerlidir. Kodun basindaki #define satirlarini degistirin.

| Define | Varsayilan | Aciklama |
| --- | --- | --- |
| ENABLE_APPROACHING_EMERGENCY_VEHICLE_DETECTION | true | Yaklasan acil durum araci algilama (bit 59). FSD surerken ambulans/polis/itfaiye algilandiginda arac otomatik yol verir. |
| ENABLE_ISA_SPEED_CHIME_SUPPRESS | false | ISA hiz limiti uyari sesini bastirir (CAN ID 921). Dikkat: Aktifken hiz limiti isareti ekranda bos gorunur. |

### 08 V14 Yeni Ozellikler Son guncellemeyle eklenen CAN ozellikleri

▼

#### 1. Legacy: Follow-Distance Stalk ile Profil Secimi (CAN ID 69)

> Legacy araclarda (Model S/X, dikey ekran) hiz profili artik CAN ID 69 (STW_ACTN_RQ — direksiyon kolu) uzerinden okunuyor. Onceki surumde HW3 tarzinda offset hesaplamasiyla yapiliyordu, bu Legacy donanimla uyumsuzdu.

| Stalk Pozisyonu (data[1] >> 5) | Profil |
| --- | --- |
| 0 veya 1 | ⚡ Hurry (2) |
| 2 | 🟢 Normal (1) |
| 3+ | ❄️ Chill (0) |

```text
// CAN ID 69 — STW_ACTN_RQ: Follow-distance stalk pozisyonu
// byte[1] ust 3 bit: 0x00=Pos1, 0x21=Pos2, 0x42=Pos3...
uint8_t pos = f.data[1] >> 5;
if (pos <= 1) cfg.speedProfile = 2; // Hurry
else if (pos == 2) cfg.speedProfile = 1; // Normal
else cfg.speedProfile = 0; // Chill
```

#### 2. HW4: Acil Durum Araci Algilama (Bit 59)

> HW4 araclarda FSD surerken yaklasan ambulans, polis veya itfaiye araclarini algilayarak otomatik yol verme ozelligini aktif eder. CAN ID 1021, mux 0'da bit 59 set edilir.

```text
// CAN ID 1021, Mux 0 — Emergency Vehicle Detection
setBit(f, 46, true); // FSD enable
setBit(f, 60, true); // HW4 FSD extended flag
setBit(f, 59, true); // Approaching Emergency Vehicle Detection
```

> ENABLE_APPROACHING_EMERGENCY_VEHICLE_DETECTION varsayilan true . Kapatmak icin kodda false yapin.

#### 3. HW4: ISA Hiz Uyari Sesi Bastirma (CAN ID 921)

> Bazi bolgelerde zorunlu olan ISA (Intelligent Speed Assistance) hiz limiti asildiginda bir uyari sesi calar. Bu ozellik CAN ID 921 mesajini modifiye ederek bu sesi bastirir.

```text
// CAN ID 921 — ISA Speed Chime Suppress
f.data[1] |= 0x20; // suppress bitini set et
// checksum yeniden hesapla (byte 0-6 + CAN ID)
uint8_t sum = 0;
for (int i = 0; i < 7; i++) sum += f.data[i];
sum += (921 & 0xFF) + (921 >> 8);
f.data[7] = sum & 0xFF; // yeni checksum
```

> Dikkat: Bu ozellik aktifken ekrandaki hiz limiti isareti bos gorunur . Varsayilan false — bilinçli olarak acin. Bazi ulkelerde ISA'yi devre disi birakmak yasalara aykiri olabilir.

#### 4. FSD State Tracking (Tum Handler'lar)

> Onceki surumde FSD UI durumu her CAN frame'de tekrar okunuyordu. Simdi fsdSelectedInUI state olarak saklanarak, mux 0 'dan alinan FSD durumu diger mux index'lerinde de dogru sekilde kullanilir. Bu ozellikle HW3'un mux 2 (speed offset) isleminde kritiktir.

```text
// Mux 0'da FSD durumunu oku ve sakla
if (idx == 0) cfg.fsdSelectedInUI = fsdInUI(f);
bool fsdOn = cfg.fsdSelectedInUI && cfg.fsdEnabled;

// Artik mux 1, mux 2'de de dogru FSD durumu kullanilir
```

### 09 Hiz Profilleri Takip mesafesiyle otomatik degisir

▼

#### HW3 / HW4 — Takip Mesafesine Gore

| Takip Mesafesi | HW3 Profili | HW4 Profili |
| --- | --- | --- |
| 2 | ⚡ Hurry | 🔥 Max |
| 3 | 🟢 Normal | ⚡ Hurry |
| 4 | ❄️ Chill | 🟢 Normal |
| 5 | — | ❄️ Chill |
| 6 | — | 🐢 Sloth |

#### Legacy — Follow-Distance Stalk Pozisyonuna Gore (CAN ID 69)

| Stalk Pozisyonu (data[1] >> 5) | Profil |
| --- | --- |
| 0–1 | ⚡ Hurry (2) |
| 2 | 🟢 Normal (1) |
| 3+ | ❄️ Chill (0) |

### Arduino Kodu CanFeather_RP2040.ino

▼

CanFeather_RP2040.ino
RP2040 Feather — CAN + UART bridge

Kopyala

ESP32C3_WiFiBridge.ino
ESP32-C3 — WiFi AP + web arayuzu

Kopyala

### 🚗 Arac Kontrol Komutlari WiFi panelinden 10 ozellik

▼

> Bu icerik ESP8266 sekmesindeki "Arac Kontrol" ile aynidir. Tum sinyal detaylari, komut listesi ve WiFi sifre degistirme rehberi icin ESP8266 → Arac Kontrol sekmesine bakin.

> RP2040 farki: Komutlar ESP32-C3 WiFi bridge uzerinden UART ile RP2040'a iletilir. Ayni endpoint: 192.168.4.1/cmd?c=mirror_fold
