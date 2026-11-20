# Tesla Model 3 — CAN Signal Reference

> Fichier source : `Tesla-Model 3.REF`
> Format : **Racelogic VBOX CAN Data File V1a**
> Unit serial number : `00000000`

## Format des signaux

Chaque signal est défini par :

| Champ | Description |
|-------|-------------|
| **Name** | Nom du signal |
| **CAN ID** | Identifiant CAN (décimal) |
| **Unit** | Unité de mesure |
| **Bit Start** | Position du premier bit dans la trame |
| **Bit Length** | Nombre de bits du signal |
| **Offset** | Offset appliqué à la valeur brute |
| **Factor** | Facteur de conversion (valeur physique = raw × factor + offset) |
| **Max** | Valeur physique maximale |
| **Min** | Valeur physique minimale |
| **Signed** | Signé ou non signé |
| **Byte Order** | Intel (Little-Endian) ou Motorola (Big-Endian) |
| **DLC** | Longueur de la trame CAN (octets) |

---

## Signaux par CAN ID

### CAN ID 257 (0x101) — Yaw Rate

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Yaw_Rate | rad | 0 | 16 | 0 | 0.0001 | -3.2766 | 3.2766 | Signed | Intel |

### CAN ID 264 (0x108) — Moteur

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Engine_Torque | Nm | 27 | 13 | 0 | 0.2 | -819.2 | 819 | Signed | Intel |
| Engine_Speed | rpm | 40 | 16 | 0 | 0.1 | -3276.8 | 3276.7 | Signed | Intel |

### CAN ID 280 (0x118) — Accélérateur / Frein

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Accelerator_Pedal_Position | % | 24 | 8 | 0 | 0.4 | 0 | 100 | Unsigned | Motorola |
| Accelerator_Pedal_Position (v2) | % | 32 | 8 | 0 | 0.4 | 0 | 102 | Unsigned | Intel |
| Brake_Light | — | 19 | 2 | 0 | 1 | 0 | 3 | Unsigned | Intel |

### CAN ID 325 (0x145) — Frein

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Brake_Position | % | 52 | 10 | 0 | 0.125 | 0 | 127.875 | Unsigned | Intel |

### CAN ID 373 (0x175) — Vitesses de roue

#### En km/h (factor 0.025)

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Wheel_Speed_FL_kph | km/h | 0 | 13 | 0 | 0.025 | 0 | 327.64 | Unsigned | Intel |
| Wheel_Speed_FR_kph | km/h | 13 | 13 | 0 | 0.025 | 0 | 163.82 | Unsigned | Intel |
| Wheel_Speed_RL_kph | km/h | 26 | 13 | 0 | 0.025 | 0 | 81.91 | Unsigned | Intel |
| Wheel_Speed_RR_kph | km/h | 39 | 13 | 0 | 0.025 | 0 | 40.955 | Unsigned | Intel |

#### En km/h (factor 0.04)

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Wheel_Speed_FL_kph | km/h | 0 | 13 | 0 | 0.04 | 0 | 327.64 | Unsigned | Intel |
| Wheel_Speed_FR_kph | km/h | 13 | 13 | 0 | 0.04 | 0 | 327.64 | Unsigned | Intel |
| Wheel_Speed_RL_kph | km/h | 26 | 13 | 0 | 0.04 | 0 | 327.64 | Unsigned | Intel |
| Wheel_Speed_RR_kph | km/h | 39 | 13 | 0 | 0.04 | 0 | 327.64 | Unsigned | Intel |

#### En mph (factor 0.015525)

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Wheel_Speed_FL_mph | mph | 0 | 13 | 0 | 0.015525 | 0 | 203.464 | Unsigned | Intel |
| Wheel_Speed_FR_mph | mph | 13 | 13 | 0 | 0.015525 | 0 | 101.732 | Unsigned | Intel |
| Wheel_Speed_RL_mph | mph | 26 | 13 | 0 | 0.015525 | 0 | 50.866 | Unsigned | Intel |
| Wheel_Speed_RR_mph | mph | 39 | 13 | 0 | 0.015525 | 0 | 25.433 | Unsigned | Intel |

#### En mph (factor 0.02484)

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Wheel_Speed_FL_mph | mph | 0 | 13 | 0 | 0.02484 | 0 | 203.464 | Unsigned | Intel |
| Wheel_Speed_FR_mph | mph | 13 | 13 | 0 | 0.02484 | 0 | 203.464 | Unsigned | Intel |
| Wheel_Speed_RL_mph | mph | 26 | 13 | 0 | 0.02484 | 0 | 203.464 | Unsigned | Intel |
| Wheel_Speed_RR_mph | mph | 39 | 13 | 0 | 0.02484 | 0 | 203.464 | Unsigned | Intel |

### CAN ID 389 (0x185) — Frein (position)

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Brake_Position | % | 36 | 12 | 0 | 0.08 | 0 | 100 | Unsigned | Intel |

### CAN ID 792 (0x318) — Portes

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Door_Open_-_Front_Left | — | 12 | 2 | 0 | 1 | 0 | 3 | Unsigned | Intel |
| Door_Open_-_Front_Right | — | 14 | 2 | 0 | 1 | 0 | 3 | Unsigned | Intel |

### CAN ID 799 (0x31F) — Pression et température des pneus (TPMS)

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Tyre_Pressure_FL | bar | 0 | 8 | 0 | 0.025 | 0 | 6.375 | Unsigned | Intel |
| Tyre_Temperature_FL | °C | 8 | 8 | -40 | 1 | -40 | 215 | Unsigned | Intel |
| Tyre_Pressure_FR | bar | 16 | 8 | 0 | 0.025 | 0 | 6.375 | Unsigned | Intel |
| Tyre_Temperature_FR | °C | 24 | 8 | -40 | 1 | -40 | 215 | Unsigned | Intel |
| Tyre_Pressure_RL | bar | 32 | 8 | 0 | 0.025 | 0 | 6.375 | Unsigned | Intel |
| Tyre_Temperature_RL | °C | 40 | 8 | -40 | 1 | -40 | 215 | Unsigned | Intel |
| Tyre_Pressure_RR | bar | 48 | 8 | 0 | 0.025 | 0 | 6.375 | Unsigned | Intel |
| Tyre_Temperature_RR | °C | 56 | 8 | -40 | 1 | -40 | 215 | Unsigned | Intel |

### CAN ID 801 (0x321) — Température extérieure

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Air_Temperature | °C | 40 | 8 | -40 | 0.5 | -40 | 87.5 | Unsigned | Intel |

### CAN ID 1160 (0x488) — Direction

| Signal | Unité | Bit Start | Bit Len | Offset | Factor | Min | Max | Signé | Byte Order |
|--------|-------|-----------|---------|--------|--------|-----|-----|-------|------------|
| Steering_Angle | ° | 48 | 16 | -1640 | 0.1 | -720 | 720 | Unsigned | Motorola |

---

## Résumé des CAN IDs

| CAN ID (dec) | CAN ID (hex) | Catégorie | Signaux |
|:---:|:---:|---|---|
| 257 | 0x101 | Dynamique véhicule | Yaw Rate |
| 264 | 0x108 | Moteur | Couple, Régime |
| 280 | 0x118 | Pédalier | Accélérateur, Témoin frein |
| 325 | 0x145 | Freinage | Position frein |
| 373 | 0x175 | Roues | Vitesse FL/FR/RL/RR (km/h & mph) |
| 389 | 0x185 | Freinage | Position frein (alt.) |
| 792 | 0x318 | Carrosserie | Portes AV gauche/droite |
| 799 | 0x31F | TPMS | Pression & température 4 pneus |
| 801 | 0x321 | Environnement | Température extérieure |
| 1160 | 0x488 | Direction | Angle volant |

---

## Notes

- Les vitesses de roue existent en **4 variantes** sur le même CAN ID 373, avec des facteurs différents selon l'unité (km/h ou mph) et la précision souhaitée.
- Le signal `Accelerator_Pedal_Position` apparaît **2 fois** sur le CAN ID 280 : une version Motorola (bit 24) et une version Intel (bit 32), avec des max légèrement différents (100 vs 102%).
- Le signal `Brake_Position` apparaît sur **2 CAN IDs différents** : 389 (12 bits, factor 0.08) et 325 (10 bits, factor 0.125).
- Toutes les trames ont un DLC de **8 octets**.
- La majorité des signaux utilisent l'ordre **Intel (Little-Endian)**, sauf `Steering_Angle` et une variante de `Accelerator_Pedal_Position` qui sont en **Motorola (Big-Endian)**.
- Le bloc initial `00000000` correspond au numéro de série de l'unité Racelogic VBOX.
