# Komut Referansı

Firmware tarafından desteklenen komutların tam listesi. Tüm komutlar seri veya Bluetooth üzerinden JSON olarak gönderilir.

## Bağlantı Komutları

- `{"cmd":"ping"}` — Kartı pingla, `{"type":"pong"}` bekler
- `{"cmd":"status"}` — Tam durum raporu iste
- `{"cmd":"stream","on":true}` — CAN frame akışını başlat
- `{"cmd":"stream","on":false}` — CAN frame akışını durdur
- `{"cmd":"variant","val":"hw4"}` — Araç varyantı ayarla (hw4, hw3, legacy)

## FSD & Nag Komutları

- `{"cmd":"fsd","on":true}` — FSD etkinleştir
- `{"cmd":"fsd","on":false}` — FSD devre dışı bırak
- `{"cmd":"nag","on":true}` — Nag bastırmayı etkinleştir
- `{"cmd":"nag","on":false}` — Nag bastırmayı devre dışı bırak

## Hız Profili Komutları

- `{"cmd":"profile","val":0}` — Sakin (0)
- `{"cmd":"profile","val":1}` — Normal (1)
- `{"cmd":"profile","val":2}` — Acele (2)
- `{"cmd":"profile","val":3}` — Maksimum (3)
- `{"cmd":"profile","val":4}` — Yavaş (4)
- `{"cmd":"profile_auto"}` — Otomatik profil seçimi

## Hız Offset Komutları (Sadece HW3)

- `{"cmd":"offset","val":0}` — %0 offset
- `{"cmd":"offset","val":20}` — %20 offset
- `{"cmd":"offset","val":100}` — %100 offset
- `{"cmd":"offset_auto"}` — Otomatik offset

## ISA Hız Zili (Sadece HW4)

- `{"cmd":"isa_chime","on":true}` — ISA zilini bastır
- `{"cmd":"isa_chime","on":false}` — Orijinal zili geri yükle

## Çağırma Komutları

- `{"cmd":"summon_fwd"}` — İleri çağır
- `{"cmd":"summon_rev"}` — Geri çağır
- `{"cmd":"summon_stop"}` — Çağırmayı durdur

## Araç Kontrol Komutları

### Aynalar
- `mirrorFold`, `mirrorUnfold`, `mirrorHeat`, `mirrorAutofold`, `mirrorDip`

### Kilitler & Korna
- `lock`, `unlock`, `lockChild`, `horn`

### Bagaj & Frunk
- `frunkOpen`, `frunkClose`, `trunkOpen`, `trunkClose`, `glovebox`

### Farlar
- `lightFogFront`, `lightFogRear`, `lightHighbeamAuto`, `lightAmbient`
- `lightHome`, `lightDomeOff`, `lightDomeOn`, `lightDomeAuto`

### Silecekler
- `wiperOff`, `wiper1`, `wiper2`, `wiper3`

### Koltuk Isıtma
- `seatFL(seviye)`, `seatFR(seviye)`, `seatRL(seviye)`, `seatRR(seviye)`, `seatRC(seviye)` — Seviye 0–3

### Cam & Nöbetçi
- `ventOpen`, `ventClose`, `sentryOn`, `sentryOff`

### Klima
- `climateKeep`, `climateOff`

### Şarj
- `chargeStart`, `chargeStop`, `chargePort`

### Sürüş Yapılandırması
- Pedal: `pedalStandard`, `pedalChill`, `pedalSport`
- Rejenerasyon: `regenOff`, `regenLow`, `regenStd`, `regenMax`
- Durma Modu: `stopCreep`, `stopRoll`, `stopHold`

### Ekran
- `mainDisplay(parlaklık)` — Parlaklık 0–127

### Güç
- `powerAccOn`, `powerAccOff`, `powerReady`, `powerOff`

## CAN Ham Komutları

- `{"cmd":"raw","id":1234,"data":"AABBCCDD"}` — Ham CAN frame gönder
- `{"cmd":"raw","id":1234,"data":"AABBCCDD","bus":1}` — İkincil bus'ta gönder

## Yanıt Türleri

| Tür | Açıklama |
| --- | -------- |
| boot | Donanım bilgisi ile kart başlatma mesajı |
| status | Tam durum raporu |
| frame | CAN frame verisi |
| ack | Komut onayı |
| error | Hata yanıtı |
| log | Bilgi log mesajı |
| pong | Ping'e yanıt |
