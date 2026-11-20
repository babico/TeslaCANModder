# Nouvelles données à afficher — Écran rond 360×360

> Brainstorming de fonctionnalités additionnelles pour le dashboard ESP32-S3  
> Tesla Model 3 "50" Standard Range — CAN bus via Bridge ESP-NOW  
> Données déjà affichées : vitesse, SoC, autonomie, gear, puissance kW, température batterie/cabine/extérieure, heure, SoC destination, clignotants, blindspot, feu stop

---

## 1. Vitesse limite et dépassement

### Données CAN disponibles

| Source | CAN ID | Bus | Signal | Bits | Factor | Unité | Description |
|--------|--------|-----|--------|------|--------|-------|-------------|
| **Carte** | `0x3D9` | Chassis | `UI_mppSpeedLimit` | 48\|5 | ×5 | kph | Limite vitesse issue de la carte GPS |
| Carte | `0x3D9` | Chassis | `UI_userSpeedOffset` | 40\|6 | ×1, −30 | kph | Offset configuré par le conducteur |
| **DAS fusionné** | `0x399` | Chassis | `DAS_fusedSpeedLimit` | 8\|5 | ×5 | kph | Limite fusionnée (panneaux + carte) — **la plus fiable** |
| DAS vision | `0x399` | Chassis | `DAS_visionOnlySpeedLimit` | 16\|5 | ×5 | kph | Limite détectée par caméras uniquement |
| UI | `0x334` | Vehicle | `UI_speedLimit` | 16\|8 | ×1, +50 | kph | Limite configurée dans l'UI (50–285 kph) |
| DAS | `0x399` | Chassis | `DAS_suppressSpeedWarning` | 13\|1 | — | bool | L'utilisateur a supprimé l'alerte vitesse |

### Recommandation

**Signal principal** : `DAS_fusedSpeedLimit` (0x399, déjà reçu pour blindspot)
- Résolution : 5 kph (0, 5, 10, … 150)
- Valeur 0 = pas de limite détectée
- Fusionne panneaux de signalisation (vision) + données cartographiques

**Détection de dépassement** :
```
speedLimit  = DAS_fusedSpeedLimit (0x399)
currentSpeed = UI_speed (0x257, déjà affiché)

if (speedLimit > 0 && currentSpeed > speedLimit):
    → Alerter : changement couleur vitesse ou halo rouge
if (currentSpeed > speedLimit + 10):
    → Alerter fort : clignotement ou cercle rouge
```

### Affichage proposé

```
┌─────────────────────────┐
│     Petit panneau rond  │
│     "90" en blanc       │
│     ou rouge si dépassé │
│     position: coin bas  │
│     gauche du dashboard │
└─────────────────────────┘
```

- **Normal** : Petit cercle avec limite en blanc/gris, au coin de l'écran
- **Dépassé ≤10 kph** : Chiffre vitesse passe en orange
- **Dépassé >10 kph** : Chiffre vitesse passe en rouge + halo pulsant
- Si `speedLimit == 0` : Masquer le panneau (pas de limite détectée)

---

## 2. Autopilot / ACC (régulateur adaptatif)

### Données CAN disponibles

| Source | CAN ID | Bus | Signal | Bits | Description |
|--------|--------|-----|--------|------|-------------|
| **DAS** | `0x399` | Chassis | `DAS_autopilotState` | 0\|4 | État Autopilot |
| DAS | `0x399` | Chassis | `DAS_autopilotHandsOnState` | 42\|4 | Demande "mains au volant" |
| DAS | `0x399` | Chassis | `DAS_autoLaneChangeState` | 46\|5 | Changement de voie auto |
| **ACC** | `0x2B9` | Chassis | `DAS_accState` | 12\|4 | État ACC |
| ACC | `0x2B9` | Chassis | `DAS_setSpeed` | 0\|12 | Vitesse consigne ACC (×0.1 kph) |

### Valeurs `DAS_autopilotState`

| Valeur | Nom | Description | Affichage suggéré |
|--------|-----|-------------|-------------------|
| 0 | `DISABLED` | Autopilot désactivé | — (rien) |
| 1 | `UNAVAILABLE` | Indisponible | Icône grisée |
| 2 | `AVAILABLE` | Disponible (pas engagé) | Icône bleue discrète |
| 3 | `ACTIVE_NOMINAL` | **Autopilot actif** | 🟢 Halo bleu + icône volant |
| 4 | `ACTIVE_RESTRICTED` | Actif restreint | 🟡 Halo bleu clignotant |
| 5 | `ACTIVE_NAV` | **Navigate on Autopilot** | 🟢 Halo bleu + icône route |
| 8 | `ABORTING` | Désengagement en cours | ⚠️ Flash rouge |
| 9 | `ABORTED` | Désengagé | ❌ Flash rouge bref |
| 14 | `FAULT` | Défaut | ❌ Rouge fixe |

### Valeurs `DAS_autopilotHandsOnState`

| Valeur | Nom | Action UI |
|--------|-----|-----------|
| 0 | `NOT_REQD` | Rien (pas de demande) |
| 1 | `REQD_DETECTED` | OK — mains détectées |
| 2 | `REQD_NOT_DETECTED` | ⚠️ Alerte douce — icône mains |
| 3 | `REQD_VISUAL` | Alerte visuelle sur écran |
| 4-5 | `REQD_CHIME_*` | Alerte sonore → halo orange |
| 6 | `REQD_SLOWING` | Véhicule ralentit → halo rouge |
| 7 | `STRUCK_OUT` | Autopilot va se désengager → rouge clignotant |
| 8 | `SUSPENDED` | Suspendu |

### Valeurs `DAS_accState`

| Valeur | Nom | Description |
|--------|-----|-------------|
| 0 | `CANCEL_GENERIC` | ACC annulé |
| 3 | `ACC_HOLD` | Hold (arrêt maintenu) |
| 4 | `ACC_ON` | ACC actif |
| 5-10 | `APC_*` | Autopark en cours |

### Affichage proposé

- **ACC/AP inactif** : Rien affiché (ou petit "A" grisé)
- **ACC actif** : Petit chiffre de vitesse consigne (`DAS_setSpeed`) sous la vitesse principale, en vert
- **Autopilot actif** : Halo bleu sur l'arc supérieur du cercle + vitesse consigne. Icône "volant" au centre-haut
- **Hands-on demandé** : Halo passe de bleu → orange → rouge selon l'urgence
- **Navigation sur AP** : Comme AP + petit indicateur "NAV" à côté de l'icône

---

## 3. Itinéraire en cours (navigation)

### Données CAN disponibles

| Source | CAN ID | Bus | Signal | Bits | Factor | Unité | Description |
|--------|--------|-----|--------|------|--------|-------|-------------|
| **Trip** | `0x082` | Vehicle | `UI_tripPlanningActive` | 0\|1 | — | bool | Navigation active |
| Trip | `0x082` | Vehicle | `UI_navToSupercharger` | 1\|1 | — | bool | Route vers Supercharger |
| **Map** | `0x238` | Chassis | `UI_navRouteActive` | 29\|1 | — | bool | Route active (map) |
| Map | `0x238` | Chassis | `UI_nextBranchDist` | 32\|5 | ×10 | m | Distance prochaine bifurcation (0–300m) |
| Map | `0x238` | Chassis | `UI_nextBranchLeftOffRamp` | 38\|1 | — | bool | Sortie à gauche |
| Map | `0x238` | Chassis | `UI_nextBranchRightOffRamp` | 39\|1 | — | bool | Sortie à droite |
| **DAS** | `0x24A` | Chassis | `DAS_navDistance` | 40\|8 | ×100 | m | Distance restante (0–25.5 km) |
| DAS | `0x24A` | Chassis | `DAS_navAvailable` | 39\|1 | — | bool | Navigation disponible |
| **Énergie** | `0x082` | Vehicle | `UI_energyAtDestination` | 48\|16 | ×0.01 | kWh | Énergie à destination (déjà utilisé pour SoC dest) |
| Énergie | `0x082` | Vehicle | `UI_predictedEnergy` | 16\|16 | ×0.01 | kWh | Énergie prédite pour le trajet |

### Affichage proposé

Actuellement, seul le SoC à destination est affiché. Ajouts possibles :

- **Indicateur "NAV"** : Petit badge quand `UI_tripPlanningActive == 1` ou `UI_navRouteActive == 1`
- **Distance restante** : Utiliser `DAS_navDistance` (résolution 100m, max 25.5 km). Au-delà de 25.5 km, estimer via `UI_predictedEnergy` / consommation
- **Prochain virage** : Direction de la prochaine sortie (`nextBranchLeftOffRamp` / `rightOffRamp`) avec distance en mètres — utile comme "aide mémoire" sans regarder l'écran principal
- **Icône Supercharger** : Si `UI_navToSupercharger == 1`, afficher un petit ⚡ à côté de la destination

> ⚠️ **Limite** : `DAS_navDistance` plafonne à 25.5 km. Pour les longs trajets, la distance réelle n'est pas disponible via CAN — Tesla la gère côté MCU/tablette.

---

## 4. Warnings DAS (alertes de conduite)

### Données CAN disponibles

| Source | CAN ID | Bus | Signal | Bits | Description |
|--------|--------|-----|--------|------|-------------|
| **FCW** | `0x399` | Chassis | `DAS_forwardCollisionWarning` | 22\|2 | Alerte collision avant (0=none, 1-3=niveaux) |
| **LDW** | `0x399` | Chassis | `DAS_laneDepartureWarning` | 37\|3 | Sortie de voie (0=none, 1-5=niveaux) |
| **Side** | `0x399` | Chassis | `DAS_sideCollisionWarning` | 34\|2 | Collision latérale (0=none, 1-3) |
| Side | `0x399` | Chassis | `DAS_sideCollisionAvoid` | 32\|2 | Évitement collision latérale |
| **Long** | `0x389` | Chassis | `DAS_longCollisionWarning` | 48\|4 | Collision longitudinale (véhicule, piéton, panneau stop…) |

### Valeurs `DAS_forwardCollisionWarning`

| Valeur | Signification | Action UI |
|--------|-------------- |----------|
| 0 | Pas d'alerte | — |
| 1 | Alerte niveau 1 | Halo rouge rapide sur l'arc avant (haut de l'écran) |
| 2 | Alerte niveau 2 | Halo rouge + flash plein écran |
| 3 | Alerte niveau 3 (imminent) | Flash rouge plein écran intense |

### Valeurs `DAS_laneDepartureWarning`

| Valeur | Nom | Action UI |
|--------|-----|-----------|
| 0 | NONE | — |
| 1 | LEFT | Halo orange côté gauche |
| 2 | RIGHT | Halo orange côté droit |
| 3-5 | Variantes | Halo orange + vibration visuelle |

### Affichage proposé

- **Collision avant** : Flash rouge sur l'arc supérieur du cercle. Intensité proportionnelle au niveau (1→doux, 3→intense)
- **Sortie de voie** : L'arc gauche ou droit du cercle clignote en orange (similaire au blindspot mais plus agressif)
- **Collision latérale** : Arc latéral en rouge clignotant
- **Superposition** : Toutes ces alertes peuvent se superposer aux halos existants (turn, blindspot, brake). Priorité : FCW > side > lane departure > blindspot > turn

> Note : Tous ces signaux sont dans `0x399` (déjà reçu pour blindspot et autopilot). **Aucun nouveau CAN ID nécessaire** — juste décoder plus de bits du même message.

---

## 5. Préchauffage batterie

### Données CAN disponibles

| Source | CAN ID | Bus | Signal | Bits | Description |
|--------|--------|-----|--------|------|-------------|
| **Trip** | `0x082` | Vehicle | `UI_requestActiveBatteryHeating` | 2\|1 | Requête préchauffage actif (route vers SC) |
| **BMS** | `0x212` | Vehicle | `BMS_preconditionAllowed` | 3\|1 | BMS autorise le préconditionnement |
| BMS | `0x212` | Vehicle | `BMS_activeHeatingWorthwhile` | 5\|1 | Chauffage actif bénéfique |
| BMS | `0x212` | Vehicle | `BMS_keepWarmRequest` | 30\|1 | Demande garder batterie chaude |
| **VCFRONT** | `0x3A1` | Vehicle | `VCFRONT_preconditionRequest` | 1\|1 | Requête préconditionnement (départ programmé) |
| **Coolant** | `0x241` | Vehicle | `VCFRONT_coolantFlowBatActual` | 0\|9 | ×0.1 LPM — Débit liquide refroidissement batterie |
| Coolant | `0x241` | Vehicle | `VCFRONT_coolantFlowBatReason` | 18\|4 | Raison du flux (chauffage, refroidissement…) |
| **Thermique** | `0x2E5` | Vehicle | Batt temp min/max | — | Déjà décodé (0x2E5) |

### Valeurs `VCFRONT_coolantFlowBatReason` (0x241)

| Valeur | Description |
|--------|-------------|
| 4 | `SERIES_2_drive_batteryNeedsHeat` — en conduite, batterie doit chauffer |
| 5 | `SERIES_3_drive_batteryWantsHeat` — en conduite, batterie veut chauffer |
| 9 | `SERIES_4_charge_batteryNeedsHeat` — en charge, chauffage nécessaire |
| 17 | `SERIES_8_preConditioning_batteryNeedsHeat` — **préchauffage navigation** |
| 25-31 | Scénarios refroidissement |

### Logique de détection

```
bool batteryPreconditioning =
    (UI_requestActiveBatteryHeating == 1)       // Route vers SC/destination
    || (VCFRONT_preconditionRequest == 1)        // Départ programmé
    || (coolantFlowBatReason == 17)              // Pre-conditioning heat
    || (BMS_activeHeatingWorthwhile == 1 && battTempMin < 10);  // Froid + BMS dit oui
```

### Affichage proposé

- **Icône flocon → flamme** : Quand le préchauffage est actif, afficher un petit symbole 🔥 à côté de la température batterie. Le symbole peut être un flocon ❄️ si la batterie est froide mais pas encore en chauffe.
- **Arc thermique** : Un arc fin autour de la température batterie qui montre la progression du chauffage (bleu froid → orange en chauffe → vert optimal 20-35°C)
- **Compatible avec SoC dest** : Quand `UI_requestActiveBatteryHeating == 1`, cela confirme que la nav vers SC est active → le SoC dest est fiable

---

## 6. Alertes véhicule (alertMatrix)

### Données CAN disponibles

| Source | CAN ID | Bus | Signal | Description |
|--------|--------|-----|--------|-------------|
| **Alertes** | `0x123` | Vehicle | `UI_a001_DriverDoorOpen` | Porte conducteur ouverte |
| | `0x123` | Vehicle | `UI_a002_DoorOpen` | Une porte ouverte |
| | `0x123` | Vehicle | `UI_a003_TrunkOpen` | Coffre ouvert |
| | `0x123` | Vehicle | `UI_a004_FrunkOpen` | Frunk ouvert |
| | `0x123` | Vehicle | `UI_a013_TPMSHardWarning` | TPMS alerte sévère |
| | `0x123` | Vehicle | `UI_a014_TPMSSoftWarning` | TPMS alerte douce |
| | `0x123` | Vehicle | `UI_a019_ParkBrakeFault` | Défaut frein parking |
| | `0x123` | Vehicle | `UI_a020_SteeringReduced` | Direction réduite |
| | `0x123` | Vehicle | `UI_a021_RearSeatbeltUnbuckled` | Ceinture AR non bouclée |
| | `0x123` | Vehicle | `UI_a031_BrakeFluidLow` | Liquide frein bas |
| **Ceintures** | `0x3A1` | Vehicle | `VCFRONT_driverUnbuckled` | Conducteur non attaché |
| **Portes** | `0x3A1` | Vehicle | `VCFRONT_driverDoorStatus` | Porte conducteur |

### Affichage proposé

Le round display n'a pas la place pour des textes d'alerte. Utiliser un **système d'icônes d'alerte** minimaliste :

- **Porte ouverte** : Icône porte en rouge au centre, disparaît quand fermée
- **TPMS** : Icône pneu en jaune/rouge
- **Ceinture** : Icône ceinture en rouge (clignotant si conducteur)
- **Freinage** : Icône frein en rouge

> Architecture : un ring de micro-icônes (16×16 px) en bas de l'écran principal, avec un badge compteur si >1 alerte.
> 
> ⚠️ **0x123 n'est PAS dans la whitelist actuelle** (Bridge valid_can_ids.h). Il faudra l'ajouter à `VEHICLE_BUS_IDS[]`.

---

## 7. Idées créatives supplémentaires

### 7.1 Power Flow (flux d'énergie)

**Données** : `rearPowerKw` (0x266) + `frontPowerKw` (0x2E5) — **déjà reçues**

Visualisation d'un flux d'énergie sur l'écran rond :
- **Accélération** : Flèche/flux vert du centre vers l'extérieur (batterie → roues)
- **Régénération** : Flèche/flux bleu de l'extérieur vers le centre (freinage → batterie)
- **Intensité** : Épaisseur/vitesse proportionnelle à la puissance
- L'arc de puissance actuel (kW) pourrait devenir un anneau dynamique coloré

### 7.2 G-mètre (accélérations)

**Données CAN** :

| CAN ID | Bus | Signal | Factor | Unité | Description |
|--------|-----|--------|--------|-------|-------------|
| `0x2D3` | Chassis | `ESP_yawRate` | 0.0625 | deg/s | Vitesse lacet |
| `0x155` | Chassis | `ESP_lateralAccel` | 0.01 | m/s² | Accélération latérale |
| `0x155` | Chassis | `ESP_longitudAccel` | 0.01 | m/s² | Accélération longitudinale |

Affichage : Crosshair au centre de l'écran rond avec un point qui bouge selon les G. Très intéressant en mode track.

> ⚠️ **0x155 et 0x2D3 sont déjà dans la whitelist ChassisBus**.

### 7.3 Pression pneus (TPMS)

**Données CAN** :

| CAN ID | Bus | Signal | Description |
|--------|-----|--------|-------------|
| `0x352` | Vehicle | `TPMS_pressureFL/FR/RL/RR` | Pression 4 roues |
| `0x352` | Vehicle | `TPMS_temperatureFL/FR/RL/RR` | Température 4 roues |

Affichage : 4 "quarts" de cercle, chacun avec la pression en couleur (vert=ok, jaune=bas, rouge=critique).

> ⚠️ Déjà dans la whitelist VehicleBus.

### 7.4 Efficacité temps réel (Wh/km)

**Données** : `totalPower` (déjà calculée) + `uiSpeed` (déjà reçue)

```
efficiency_wh_km = (totalPower * 1000) / max(uiSpeed, 1)
```

Affichage : Arc d'efficacité autour du cercle, coloré (vert <150 Wh/km, jaune <200, orange <250, rouge >250).

### 7.5 Capteurs de recul (Parking Sensors)

**Données CAN** :

| CAN ID | Bus | Signal | Description |
|--------|-----|--------|-------------|
| `0x3F2` | Vehicle | `PARK_sdiSensor1-6RawDistData` | 6 capteurs avant (0-511 cm) |
| `0x3F2` | Vehicle | `PARK_sdiSensor7-12RawDistData` | 6 capteurs arrière |

Affichage en gear R : Vue radar semi-circulaire avec les distances colorées (vert→jaune→rouge→clignotant).

> ⚠️ **0x3F2 est dans la whitelist.** 

### 7.6 Mode Track / Lap Timer

**Données** : GPS via `0x04F` (lat/long ChassisBus) + `UI_gpsVehicleSpeed` (0x3D9)

Fonctionnalité :
- Détection de start/finish line par géofence GPS
- Chrono par tour, meilleur temps
- G-mètre intégré

> ⚠️ **0x04F est dans la whitelist ChassisBus. 0x3D9 aussi.**

### 7.7 Voltage 12V

**Données CAN** :

| CAN ID | Bus | Signal | Description |
|--------|-----|--------|-------------|
| `0x3A1` | Vehicle | `VCFRONT_pcs12vVoltageTarget` | Tension 12V cible (×0.01 V) |

Utile pour surveiller la batterie 12V (alerter si <11.5V).

> ⚠️ **0x3A1 est dans la whitelist.**

---

## 8. Récapitulatif des CAN IDs nécessaires

### Déjà reçus (aucune modification Bridge nécessaire)

| CAN ID | Bus | Signaux disponibles | Utilisé pour |
|--------|-----|-------------------|--------------|
| `0x399` | Chassis | `DAS_autopilotState`, `DAS_fusedSpeedLimit`, `DAS_forwardCollisionWarning`, `DAS_laneDepartureWarning`, `DAS_sideCollisionWarning`, `DAS_autopilotHandsOnState`, `DAS_autoLaneChangeState` | **Autopilot + Vitesse limite + Warnings** |
| `0x082` | Vehicle | `UI_tripPlanningActive`, `UI_requestActiveBatteryHeating`, `UI_navToSupercharger` | **Navigation + Préchauffage** |
| `0x2E5` | Vehicle | Batt temp min/max | Déjà affiché |
| `0x266` | Vehicle | Rear power | Déjà affiché |
| `0x257` | Vehicle | Speed | Déjà affiché |

### À ajouter comme priority IDs dans le Bridge

| CAN ID | Bus | Signaux | Usage |
|--------|-----|---------|-------|
| `0x2B9` | **Chassis** | `DAS_accState`, `DAS_setSpeed` | Vitesse consigne ACC |
| `0x238` | **Chassis** | `UI_navRouteActive`, `UI_nextBranchDist`, `UI_mapSpeedLimit` | Navigation + limite carte |
| `0x24A` | **Chassis** | `DAS_navDistance` | Distance restante |
| `0x212` | Vehicle | `BMS_preconditionAllowed`, `BMS_activeHeatingWorthwhile` | Préchauffage batterie |
| `0x241` | Vehicle | `VCFRONT_coolantFlowBatReason/Actual` | Détail préchauffage |
| `0x3A1` | Vehicle | `VCFRONT_preconditionRequest`, `VCFRONT_driverDoorStatus` | Préconditionnement + portes |

### À ajouter à la whitelist Bridge

| CAN ID | Bus | Raison |
|--------|-----|--------|
| `0x123` | Vehicle | Alert matrix (portes, TPMS, ceintures…) — **62 alertes en 1 message** |

> Les IDs 0x2B9, 0x238, 0x24A sont **déjà dans la whitelist ChassisBus**.  
> Les IDs 0x212, 0x241, 0x3A1 sont **déjà dans la whitelist VehicleBus**.  
> Seul **0x123 doit être ajouté** (il est déjà dans `VEHICLE_BUS_IDS[]` dans valid_can_ids.h ✅).

### IDs à ajouter en priority dans le Bridge

```cpp
// Dans espnowTxTask, ajouter aux priority IDs :
msg.can_id == 0x2B9    // DAS_control (ACC state, vitesse consigne)
msg.can_id == 0x123    // Alert matrix (portes, TPMS)
```

---

## 9. Priorisation recommandée

| Priorité | Fonctionnalité | Complexité | Valeur ajoutée | CAN IDs |
|----------|---------------|------------|----------------|---------|
| **P1** | Vitesse limite + dépassement | ★★☆ | ★★★★★ | 0x399 (existant) |
| **P1** | Autopilot / ACC état | ★★★ | ★★★★★ | 0x399 + 0x2B9 |
| **P2** | Warnings DAS (FCW/LDW) | ★★☆ | ★★★★☆ | 0x399 (existant) |
| **P2** | Préchauffage batterie | ★★☆ | ★★★☆☆ | 0x082 (existant) + 0x212 |
| **P3** | Navigation (distance, direction) | ★★★ | ★★★☆☆ | 0x238 + 0x24A |
| **P3** | Alertes véhicule (portes, TPMS) | ★★☆ | ★★☆☆☆ | 0x123 |
| **P4** | G-mètre | ★★★ | ★★☆☆☆ | 0x155 + 0x2D3 |
| **P4** | Power flow animé | ★★★★ | ★★☆☆☆ | Existant |
| **P5** | Parking sensors | ★★★★ | ★★☆☆☆ | 0x3F2 |
| **P5** | Efficacité Wh/km | ★☆☆ | ★★☆☆☆ | Existant |
| **P5** | TPMS 4 roues | ★★☆ | ★☆☆☆☆ | 0x352 |

> **Quick wins** (P1) : Vitesse limite et Autopilot state utilisent principalement 0x399, déjà reçu. L'implémentation nécessite uniquement des changements côté Ecran (parsing + affichage).
