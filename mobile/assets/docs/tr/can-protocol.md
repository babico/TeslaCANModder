# CAN Protokolü

TeslaCANModder tarafından kullanılan CAN bus iletişim protokolünün teknik detayları.

## Genel Bakış

Firmware, Tesla VehicleBus CAN ağıyla 500 kbps hızında iletişim kurar. Standart CAN 2.0A frame'leri okur ve yazar (11-bit ID, 8 bayta kadar veri).

## Frame Formatı

Firmware tarafından alınan her CAN frame, seri/BT arayüzüne JSON olarak iletilir:

```json
{
  "type": "frame",
  "id": 1160,
  "dlc": 8,
  "data": "0102030405060708",
  "dir": "rx",
  "seq": 42,
  "bus": 0
}
```

| Alan | Tür | Açıklama |
|------|-----|----------|
| type | string | Her zaman "frame" |
| id | number | CAN ID (ondalık, 11-bit) |
| dlc | number | Veri uzunluk kodu (0–8) |
| data | string | Hex kodlu veri baytları |
| dir | string | "rx" (alınan) veya "tx" (gönderilen) |
| seq | number | Sıra sayacı |
| bus | number | 0 = birincil MCP2515, 1 = ikincil |

## Bilinen CAN ID'leri

| CAN ID | Hex | Açıklama |
|--------|-----|----------|
| 1160 | 0x488 | VehicleBus durumu |
| 881 | 0x371 | FSD kontrol |
| 962 | 0x3C2 | Nag bastırma |
| 599 | 0x257 | Hız profili |
| 1001 | 0x3E9 | Hız offset |
| 785 | 0x311 | ISA hız zili |
| 644 | 0x284 | Çağırma kontrol |
| 1200 | 0x4B0 | Kapı/kilit durumu |
| 801 | 0x321 | Klima kontrol |
| 1024 | 0x400 | Şarj durumu |
| 513 | 0x201 | Far kontrol |
| 770 | 0x302 | Koltuk ısıtma |

## Bus Hızı & Zamanlama

- **Baud Hızı:** 500 kbps (VehicleBus standardı)
- **Kristal:** MCP2515 modülünde 8 MHz
- **SPI Saat:** 8 MHz (Arduino SPI varsayılan)
- **Kesme Modu:** INT pininde düşen kenar

## Çoklu CAN Bus

İkinci MCP2515 bağlandığında:
- Bus 0 (FSD): CS=D10, INT=D2 — FSD Bus
- Bus 1 (Vehicle): CS=D9, INT=D3 — Vehicle Bus
- Her iki bus bağımsız olarak 500 kbps'de çalışır
- Frame JSON'daki `bus` alanı kaynağı belirtir

## CAN Frame Çözümleme

Uygulama, bilinen ID'leri okunabilir açıklamalara eşleyen yerleşik CAN frame çözücü içerir:

- ID etiket araması (örn. 0x488 → "VehicleBus Durumu")
- Bilinen frame'ler için bayt düzeyinde alan çıkarma
- İzleme için bit-fark vurgulama

## Seri Protokol

Kart ve uygulama arasındaki tüm iletişim JSON satırları kullanır (satır başına bir JSON nesnesi, `\n` ile sonlandırılır).

### Kart → Uygulama Mesajları
- `{"type":"boot","hw":"uno","driver":"mcp2515","variant":"hw4",...}`
- `{"type":"status","fsd":true,"nag":false,"profile":1,...}`
- `{"type":"frame","id":1160,"dlc":8,"data":"...","dir":"rx"}`
- `{"type":"ack","cmd":"fsd","ok":true}`
- `{"type":"error","msg":"bilinmeyen komut"}`
- `{"type":"log","msg":"CAN bus kurtarıldı"}`
- `{"type":"pong"}`

### Uygulama → Kart Komutları
- `{"cmd":"ping"}`
- `{"cmd":"fsd","on":true}`
- `{"cmd":"stream","on":true}`
- vb.
