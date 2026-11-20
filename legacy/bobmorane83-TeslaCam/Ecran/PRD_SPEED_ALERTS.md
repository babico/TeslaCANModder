# PRD — Alertes Vitesse & Autopilot (Ecran)

> Firmware : `Ecran/firmware/src/main.cpp` — ESP32-S3 JC3636W518C, ST77916 360×360 rond, LVGL

---

## 1. Résumé

Cinq fonctionnalités visuelles liées à la vitesse et à l'autopilot, toutes dérivées de données CAN déjà reçues via ESP-NOW depuis le Bridge.

---

## 2. Source de données CAN

Toutes les nouvelles données sont dans le message **`0x399` (`CAN_ID_BLIND_SPOT`)**, déjà reçu et partiellement décodé.

| Signal DBC | Octet/bits | Calcul | Valeur spéciale |
|---|---|---|---|
| `DAS_autopilotState` | `data[0]` bits [3:0] | `data[0] & 0x0F` | 3=ACTIVE_NOMINAL, 4=ACTIVE_RESTRICTED, 5=ACTIVE_NAV |
| `DAS_fusedSpeedLimit` | `data[1]` bits [4:0] | `(data[1] & 0x1F) * 5` → km/h | 31=NONE (pas de limite), 0=UNKNOWN |
| `DAS_blindSpotRearLeft` | `data[0]` bits [5:4] | déjà décodé | — |
| `DAS_blindSpotRearRight` | `data[0]` bits [7:6] | déjà décodé | — |

**Modification du parser existant :**
- Passer le check DLC de `>= 1` à `>= 2` pour lire `data[1]`.
- Ajouter deux champs dans la struct `canData` : `uint8_t autopilotState` et `uint8_t fusedSpeedLimitKph`.

---

## 3. Calculs communs

### 3.1 Tolérance de vitesse
```
if (speedLimit < 100)  tolerance = 5          // +5 km/h
else                    tolerance = speedLimit * 0.05f  // +5%
speedLimitHigh = speedLimit + tolerance
```

### 3.2 État de dépassement
```
bool hasLimit     = (canData.fusedSpeedLimitKph != 0 && canData.fusedSpeedLimitKph != 155)
                    // 31 * 5 = 155 → NONE
bool overLimit    = hasLimit && (speed > canData.fusedSpeedLimitKph)
bool overTolerance = hasLimit && (speed > speedLimitHigh)
```

### 3.3 Mapping vitesse → angle LVGL
L'arc de vitesse couvre SPEED_ARC_RANGE=270° pour une plage LVGL 0–180 (= 0–180 km/h).
```
float speedToAngle(int speedKph) {
    float clamped = fminf((float)speedKph, 180.0f);
    return SPEED_ARC_START + (clamped / 180.0f) * SPEED_ARC_RANGE;
    // Exemple : 50 km/h → 135 + 75 = 210°
    // Exemple : 130 km/h → 135 + 195 = 330°
}
```

---

## 4. Fonctionnalités

### F1 — Autopilot : vitesse et arc en bleu

**Trigger :** `canData.autopilotState` ∈ {3, 4, 5}

**Comportement :**
- `lblSpeed` (label numérique) : couleur → `COL_BLUE` (`MKCOL(26, 143, 255)` — déjà défini)
- `arcSpeed` indicateur : couleur → `COL_BLUE`
- Si simultanément `overLimit` : F3 prend la priorité pour la couleur (orange/rouge)
- Si simultanément `overTolerance` : F4 prend la priorité

**Réinitialisation :** Quand `autopilotState` ∉ {3, 4, 5}, revenir à la couleur par défaut (teal).

---

### F2 — Arc de zone de tolérance de vitesse

**Trigger :** `hasLimit` (fusedSpeedLimitKph valide)

**Description :** Arc rouge **sous** l'arc de vitesse principal (`arcSpeed`), plus large (width=10px vs 6px), qui représente visuellement la zone entre la limite et la limite + tolérance.

**Création LVGL :**
- Nouvel objet `arcSpeedLimit`, créé **avant** `arcSpeed` dans `createDashboard()` pour être dessiné en dessous (z-order LVGL = ordre de création).
- Même centre et même rayon extérieur que `arcSpeed` (SPEED_R_OUTER=158, centré).
- `lv_arc_set_bg_angles(arcSpeedLimit, 0, 0)` → fond invisible.
- Rotation et range à configurer via angles absolus LVGL :
  ```
  lv_arc_set_rotation(arcSpeedLimit, 0);
  lv_arc_set_bg_angles(arcSpeedLimit, speedToAngle(speedLimit), speedToAngle(speedLimit));  // fond invisible
  lv_arc_set_fg_angles(arcSpeedLimit, speedToAngle(speedLimit), speedToAngle(speedLimitHigh));
  // ou équivalent avec lv_arc_set_start_angle / lv_arc_set_end_angle si l'API LVGL utilisée l'exige
  ```
  > **Note implémentation :** LVGL 8 n'a pas de `lv_arc_set_fg_angles` direct. Utiliser un arc en mode `LV_ARC_MODE_NORMAL` avec bg_angles couvrant uniquement la zone souhaitée (pas de fond tracé si opa=0 sur LV_PART_MAIN), et indicator covering la zone limit→limit+tolerance.

- Style indicator : couleur `COL_RED`, opacité cover, `arc_rounded = true`.
- Style background (LV_PART_MAIN) : `LV_OPA_TRANSP` (invisible).
- Width : 10px (> SPEED_ARC_WIDTH=6).

**Mise à jour dans `updateDashboard()` :** Recalculer les angles à chaque appel si `fusedSpeedLimitKph` a changé.

**Masquage :** Si `!hasLimit`, cacher l'arc (`lv_obj_add_flag(arcSpeedLimit, LV_OBJ_FLAG_HIDDEN)`).

---

### F3 — Couleur dynamique du cercle de vitesse

Remplace la logique actuelle de `lvSpeedColor()` et l'application de la couleur dans `updateDashboard()`.

**Logique de couleur (priorité décroissante) :**

| Condition | Couleur `lblSpeed` | Couleur indicateur `arcSpeed` |
|---|---|---|
| `overTolerance` (speed > limit + tolerance) | `COL_RED` | `COL_RED` |
| `overLimit` (speed > limit, dans tolérance) | `COL_ORANGE` | `COL_ORANGE` |
| `autopilotActive` (F1) | `COL_BLUE` | `COL_BLUE` |
| Défaut | `COL_WHITE` (actuel) | `COL_TEAL` (actuel) |

**Nouvelle constante à ajouter :**
```cpp
static const lv_color_t COL_ORANGE = MKCOL(255, 120, 0);
```

---

### F4 — Halo rouge "dépassement tolérance" en haut de l'écran

**Trigger :** `overTolerance` (speed > fusedSpeedLimitKph + tolerance)

**Description :** Halo rouge vif en haut de l'écran, symétrique, utilisant le même mécanisme de canvas pre-rendered que les halos de clignotants. S'allume immédiatement, reste actif tout le temps du dépassement, disparaît immédiatement en dessous.

**Nouveau canvas :** `speedHaloCanvasBuf` (similaire à `brakeCanvasBuf`) — arc de ~180° en haut de l'écran, gradient depuis le bord vers l'intérieur (profondeur `TURN_HALO_WIDTH=110`).

**Couleur :** `COL_HALO_RED` (`MKCOL(255, 0, 0)`) — déjà défini.

**Position :** Arc supérieur. En termes d'angles LVGL (clockwise from 3 o'clock) : ~270° à ~90° (passant par 0°/360°), soit le demi-cercle du haut. Le halo couvre la moitié supérieure de l'écran en arc depuis le bord extérieur.

**Logique :**
```cpp
// Dans la boucle principale (comme pour brake/turn halo)
if (overTolerance) {
    showSpeedHalo(COL_HALO_RED);
} else {
    hideSpeedHalo();
}
```

**Implémentation :**
- `prerenderSpeedHalo()` : identique à `prerenderTurnHalo()` mais sur un arc supérieur de 180°.
- Appelé une fois à l'init, couleur fixe (rouge), pas de recoloriage nécessaire.
- Le canvas est affiché/masqué avec `lv_obj_add/clear_flag(LV_OBJ_FLAG_HIDDEN)` ou `lv_obj_set_style_opa()`.

---

### F5 — Angle mort : orange vif au lieu d'ambre

**Changement unique :**
```cpp
// Avant
static const lv_color_t COL_HALO_AMBER = MKCOL(255, 210, 0);
// Après
static const lv_color_t COL_HALO_AMBER = MKCOL(255, 120, 0);  // orange vif
```
> Cette couleur est la même que `COL_ORANGE` (F3). Peut être unifiée en une seule constante `COL_ORANGE_VIVID`.

Aucune autre modification : le mécanisme de halo d'angle mort reste identique.

---

## 5. Modifications fichiers

| Fichier | Changements |
|---|---|
| `Ecran/firmware/src/main.cpp` | Toutes les modifications (struct, parser 0x399, nouvelles constantes, nouveaux widgets LVGL, logique updateDashboard, halo overspeed) |
| `Bridge/src/valid_can_ids.h` | Aucun (0x399 déjà présent) |
| `Bridge/src/main.cpp` | Aucun (0x399 déjà forwardé tel quel) |

---

## 6. Ordre d'implémentation recommandé

- [ ] F5 — Couleur angle mort (1 ligne, risque zéro)
- [ ] F3 — Ajout constante `COL_ORANGE` + logique couleur `updateDashboard()`
- [ ] F1 — Parser `DAS_autopilotState` dans 0x399 + champ struct + couleur bleue
- [ ] F2 — Parser `DAS_fusedSpeedLimit` + champ struct + nouvel arc `arcSpeedLimit`
- [ ] F4 — Halo rouge `speedHaloCanvas` + logique affichage/masquage

---

## 7. Points d'attention

- **Unités `DAS_fusedSpeedLimit`** : le DBC note "kph/mph". Pour une Model 3 en France configurée en km/h, la valeur est en km/h. Si l'unité est mph, multiplier par 1.609. Un signal `UI_mapSpeedLimitUnits` existe en 0x3D9 (bit 46) mais n'est pas actuellement reçu. Pour v1 : supposer km/h et noter dans le code.
- **Tolérance < 100 km/h** : +5 km/h absolus. Tolérance ≥ 100 km/h : +5% (arrondissement à l'entier).
- **DLC check** : Le case `CAN_ID_BLIND_SPOT` doit passer à `m->dlc >= 2` pour lire `data[1]`.
- **Z-order LVGL** : `arcSpeedLimit` doit être créé avant `arcSpeed` (et donc avant `arcSpeedTrack` qui est le fond de l'arc de vitesse) dans `createDashboard()`.
- **`DAS_fusedSpeedLimit` raw value 0** : signifie UNKNOWN (pas connu), traiter comme "pas de limite".
