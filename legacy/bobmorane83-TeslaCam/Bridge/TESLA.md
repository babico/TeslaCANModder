# Tesla Model 3 — Documentation du bus CAN

Documentation complète des messages et signaux CAN du Tesla Model 3 (2022).
Chaque signal est accompagné d'une description lisible en français.
Source : `Can/tesla_can.dbc`.

## Table des matières

- [Bus principal du véhicule (Vehicle Bus)](#bus-principal-du-véhicule-vehicle-bus) — 135 messages
- [Bus châssis (Chassis Bus)](#bus-châssis-chassis-bus) — 24 messages

---

## Bus principal du véhicule (Vehicle Bus)

### 0x00C — UI_status

**État de l'interface utilisateur : écran tactile, connexions (WiFi, Bluetooth, cellulaire, GPS), audio, système**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_audioActive` | — | Audio en cours de lecture |
| `UI_autopilotTrial` | — | État de l'essai Autopilot. Valeurs : NONE (0), START (1), STOP (2), ACTIVE (3) |
| `UI_bluetoothActive` | — | Bluetooth activé |
| `UI_cameraActive` | — | Caméra de recul active |
| `UI_cellActive` | — | Module cellulaire (4G/LTE) activé |
| `UI_cellConnected` | — | Connexion cellulaire établie |
| `UI_cellNetworkTechnology` | — | Type de réseau cellulaire utilisé. Valeurs possibles : CELL_NETWORK_NONE (0), CELL_NETWORK_GPRS (1), CELL_NETWORK_EDGE (2), CELL_NETWORK_UMTS (3), CELL_NETWORK_HSDPA (4), CELL_NETWORK_HSUPA (5), CELL_NETWORK_HSPA (6), CELL_NETWORK_LTE (7), CELL_NETWORK_GSM (8), CELL_NETWORK_CDMA (9), CELL_NETWORK_WCDMA (10), CELL_NETWORK_SNA (15) |
| `UI_cellReceiverPower` | dB | Puissance du signal cellulaire reçu |
| `UI_cellSignalBars` | — | Nombre de barres de signal cellulaire. Valeurs : ZERO (0), ONE (1), TWO (2), THREE (3), FOUR (4), FIVE (5), SNA (7) |
| `UI_cpuTemperature` | C | Température du processeur de l'écran tactile |
| `UI_developmentCar` | — | Véhicule de développement/test |
| `UI_displayOn` | — | Écran tactile allumé |
| `UI_displayReady` | — | Écran tactile prêt à l'utilisation |
| `UI_factoryReset` | — | Mode de réinitialisation usine. Valeurs : NONE_SNA (0), DEVELOPER (1), DIAGNOSTIC (2), CUSTOMER (3) |
| `UI_falseTouchCounter` | 1 | Compteur de faux appuis sur l'écran tactile |
| `UI_gpsActive` | — | Module GPS activé |
| `UI_pcbTemperature` | C | Température de la carte électronique de l'écran |
| `UI_radioActive` | — | Radio FM/AM active |
| `UI_readyForDrive` | — | Véhicule prêt à rouler |
| `UI_screenshotActive` | — | Capture d'écran en cours |
| `UI_systemActive` | — | Système d'infodivertissement actif |
| `UI_touchActive` | — | Écran tactile réactif aux appuis |
| `UI_vpnActive` | — | Tunnel VPN Tesla actif |
| `UI_wifiActive` | — | Module WiFi activé |
| `UI_wifiConnected` | — | WiFi connecté à un réseau |

### 0x016 — I_bmsRequest

**Requêtes de l'onduleur vers le BMS : ouverture des contacteurs haute tension, version du protocole**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DI_bmsOpenContactorsRequest` | — | Demande d'ouverture des contacteurs haute tension |
| `DI_bmsRequestInterfaceVersion` | — | Version du protocole de communication BMS |

### 0x082 — UI_tripPlanning

**Planification de trajet : énergie estimée à destination, préchauffage batterie, navigation Superchargeur**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_energyAtDestination` | kWh | Énergie estimée restante à l'arrivée. Valeurs : TRIP_TOO_LONG (32767), SNA (32768) |
| `UI_hindsightEnergy` | kWh | Énergie réellement consommée (mesurée a posteriori). Valeurs : TRIP_TOO_LONG (32767), SNA (32768) |
| `UI_navToSupercharger` | — | Navigation active vers un Superchargeur |
| `UI_predictedEnergy` | kWh | Énergie prédite pour le trajet. Valeurs : TRIP_TOO_LONG (32767), SNA (32768) |
| `UI_requestActiveBatteryHeating` | — | Demande de préchauffage de la batterie avant la charge |
| `UI_tripPlanningActive` | — | Planification d'itinéraire active |

### 0x102 — VCLEFT_doorStatus

**Portières côté gauche : loquet, poignée, interrupteur intérieur, rétroviseur (pliage, chauffage, inclinaison)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCLEFT_frontHandlePWM` | % | Signal de commande de la poignée avant gauche |
| `VCLEFT_frontHandlePulled` | — | Poignée avant gauche tirée |
| `VCLEFT_frontHandlePulledPersist` | — | Poignée avant gauche tirée (état mémorisé) |
| `VCLEFT_frontIntSwitchPressed` | — | Bouton intérieur de la portière avant gauche appuyé |
| `VCLEFT_frontLatchStatus` | — | État du loquet de la portière avant gauche. Valeurs possibles : LATCH_SNA (0), LATCH_OPENED (1), LATCH_CLOSED (2), LATCH_CLOSING (3), LATCH_OPENING (4), LATCH_AJAR (5), LATCH_TIMEOUT (6), LATCH_DEFAULT (7), LATCH_FAULT (8) |
| `VCLEFT_frontLatchSwitch` | — | Capteur du loquet de la portière avant gauche |
| `VCLEFT_frontRelActuatorSwitch` | — | Capteur de l'actionneur de déverrouillage avant gauche |
| `VCLEFT_mirrorDipped` | — | Rétroviseur gauche en position basse (manœuvre) |
| `VCLEFT_mirrorFoldState` | — | État de pliage du rétroviseur gauche. Valeurs : MIRROR_FOLD_STATE_UNKNOWN (0), MIRROR_FOLD_STATE_FOLDED (1), MIRROR_FOLD_STATE_UNFOLDED (2), MIRROR_FOLD_STATE_FOLDING (3), MIRROR_FOLD_STATE_UNFOLDING (4) |
| `VCLEFT_mirrorHeatState` | — | État du chauffage du rétroviseur gauche. Valeurs : HEATER_STATE_SNA (0), HEATER_STATE_ON (1), HEATER_STATE_OFF (2), HEATER_STATE_OFF_UNAVAILABLE (3), HEATER_STATE_FAULT (4) |
| `VCLEFT_mirrorRecallState` | — | Rappel de position mémorisée du rétroviseur gauche. Valeurs : MIRROR_RECALL_STATE_INIT (0), MIRROR_RECALL_STATE_RECALLING_AXIS_1 (1), MIRROR_RECALL_STATE_RECALLING_AXIS_2 (2), MIRROR_RECALL_STATE_RECALLING_COMPLETE (3), MIRROR_RECALL_STATE_RECALLING_FAILED (4), MIRROR_RECALL_STATE_RECALLING_STOPPED (5) |
| `VCLEFT_mirrorState` | — | État de fonctionnement du rétroviseur gauche. Valeurs : MIRROR_STATE_IDLE (0), MIRROR_STATE_TILT_X (1), MIRROR_STATE_TILT_Y (2), MIRROR_STATE_FOLD_UNFOLD (3), MIRROR_STATE_RECALL (4) |
| `VCLEFT_mirrorTiltXPosition` | V | Inclinaison horizontale du rétroviseur gauche |
| `VCLEFT_mirrorTiltYPosition` | V | Inclinaison verticale du rétroviseur gauche |
| `VCLEFT_rearHandlePWM` | % | Signal de commande de la poignée arrière gauche |
| `VCLEFT_rearHandlePulled` | — | Poignée arrière gauche tirée |
| `VCLEFT_rearIntSwitchPressed` | — | Bouton intérieur de la portière arrière gauche appuyé |
| `VCLEFT_rearLatchStatus` | — | État du loquet de la portière arrière gauche. Valeurs possibles : LATCH_SNA (0), LATCH_OPENED (1), LATCH_CLOSED (2), LATCH_CLOSING (3), LATCH_OPENING (4), LATCH_AJAR (5), LATCH_TIMEOUT (6), LATCH_DEFAULT (7), LATCH_FAULT (8) |
| `VCLEFT_rearLatchSwitch` | — | Capteur du loquet de la portière arrière gauche |
| `VCLEFT_rearRelActuatorSwitch` | — | Capteur de l'actionneur de déverrouillage arrière gauche |

### 0x103 — VCRIGHT_doorStatus

**Portières côté droit et coffre : loquet, poignée, interrupteur intérieur, rétroviseur (pliage, chauffage, inclinaison)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCRIGHT_frontHandlePWM` | % | Signal de commande de la poignée avant droite |
| `VCRIGHT_frontHandlePulled` | — | Poignée avant droite tirée |
| `VCRIGHT_frontHandlePulledPersist` | — | Poignée avant droite tirée (état mémorisé) |
| `VCRIGHT_frontIntSwitchPressed` | — | Bouton intérieur de la portière avant droite appuyé |
| `VCRIGHT_frontLatchStatus` | — | État du loquet de la portière avant droite. Valeurs possibles : LATCH_SNA (0), LATCH_OPENED (1), LATCH_CLOSED (2), LATCH_CLOSING (3), LATCH_OPENING (4), LATCH_AJAR (5), LATCH_TIMEOUT (6), LATCH_DEFAULT (7), LATCH_FAULT (8) |
| `VCRIGHT_frontLatchSwitch` | — | Capteur du loquet de la portière avant droite |
| `VCRIGHT_frontRelActuatorSwitch` | — | Capteur de l'actionneur de déverrouillage avant droite |
| `VCRIGHT_mirrorDipped` | — | Rétroviseur droit en position basse (manœuvre) |
| `VCRIGHT_mirrorFoldState` | — | État de pliage du rétroviseur droit. Valeurs : MIRROR_FOLD_STATE_UNKNOWN (0), MIRROR_FOLD_STATE_FOLDED (1), MIRROR_FOLD_STATE_UNFOLDED (2), MIRROR_FOLD_STATE_FOLDING (3), MIRROR_FOLD_STATE_UNFOLDING (4) |
| `VCRIGHT_mirrorRecallState` | — | Rappel de position mémorisée du rétroviseur droit. Valeurs : MIRROR_RECALL_STATE_INIT (0), MIRROR_RECALL_STATE_RECALLING_AXIS_1 (1), MIRROR_RECALL_STATE_RECALLING_AXIS_2 (2), MIRROR_RECALL_STATE_RECALLING_COMPLETE (3), MIRROR_RECALL_STATE_RECALLING_FAILED (4), MIRROR_RECALL_STATE_RECALLING_STOPPED (5) |
| `VCRIGHT_mirrorState` | — | État de fonctionnement du rétroviseur droit. Valeurs : MIRROR_STATE_IDLE (0), MIRROR_STATE_TILT_X (1), MIRROR_STATE_TILT_Y (2), MIRROR_STATE_FOLD_UNFOLD (3), MIRROR_STATE_RECALL (4) |
| `VCRIGHT_mirrorTiltXPosition` | V | Inclinaison horizontale du rétroviseur droit |
| `VCRIGHT_mirrorTiltYPosition` | V | Inclinaison verticale du rétroviseur droit |
| `VCRIGHT_rearHandlePWM` | % | Signal de commande de la poignée arrière droite |
| `VCRIGHT_rearHandlePulled` | — | Poignée arrière droite tirée |
| `VCRIGHT_rearIntSwitchPressed` | — | Bouton intérieur de la portière arrière droite appuyé |
| `VCRIGHT_rearLatchStatus` | — | État du loquet de la portière arrière droite. Valeurs possibles : LATCH_SNA (0), LATCH_OPENED (1), LATCH_CLOSED (2), LATCH_CLOSING (3), LATCH_OPENING (4), LATCH_AJAR (5), LATCH_TIMEOUT (6), LATCH_DEFAULT (7), LATCH_FAULT (8) |
| `VCRIGHT_rearLatchSwitch` | — | Capteur du loquet de la portière arrière droite |
| `VCRIGHT_rearRelActuatorSwitch` | — | Capteur de l'actionneur de déverrouillage arrière droite |
| `VCRIGHT_reservedForBackCompat` | — | Réservé pour la rétrocompatibilité |
| `VCRIGHT_trunkLatchStatus` | — | État du loquet du coffre. Valeurs possibles : LATCH_SNA (0), LATCH_OPENED (1), LATCH_CLOSED (2), LATCH_CLOSING (3), LATCH_OPENING (4), LATCH_AJAR (5), LATCH_TIMEOUT (6), LATCH_DEFAULT (7), LATCH_FAULT (8) |

### 0x108 — IR_torque

**Onduleur arrière : couple demandé, couple mesuré, vitesse de l'essieu, position pédale** | Cycle : 10 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `DIR_axleSpeed` | RPM | Vitesse de l'essieu arrière. Valeurs : SNA (32768) |
| `DIR_slavePedalPos` | % | Position de la pédale (secondaire, arrière). Valeurs : SNA (255) |
| `DIR_torqueActual` | Nm | Couple réel mesuré sur l'essieu arrière. Valeurs : SNA (4096) |
| `DIR_torqueChecksum` | — | Somme de contrôle du message couple arrière |
| `DIR_torqueCommand` | Nm | Couple demandé à l'essieu arrière. Valeurs : SNA (4096) |
| `DIR_torqueCounter` | — | Compteur de séquence du message couple arrière |
| `DIR_axleSpeedQF` | — | Indicateur de qualité de la vitesse essieu arrière |

### 0x113 — GTW_bmpDebug

**Débogage de la passerelle (Gateway) : états du microprocesseur de gestion d'énergie**

| Signal | Unité | Description |
|--------|-------|-------------|
| `GTW_BMP_AWAKE_PIN` | — | Broche de réveil du processeur de la passerelle |
| `GTW_BMP_GTW_PMIC_ERROR` | — | Erreur du circuit d'alimentation de la passerelle |
| `GTW_BMP_GTW_PMIC_ON` | — | Circuit d'alimentation de la passerelle : sous tension |
| `GTW_BMP_GTW_PMIC_THERMTRIP` | — | Disjonction thermique du circuit d'alimentation de la passerelle |
| `GTW_BMP_GTW_SOC_PWROK` | — | Alimentation du processeur de la passerelle : OK |
| `GTW_BMP_GTW_nSUSPWR` | — | Signal de maintien en veille de la passerelle |
| `GTW_BMP_PERIPH_nRST_3V3_PIN` | — | Broche de réinitialisation des périphériques 3,3 V |
| `GTW_BMP_PGOOD_PIN` | — | Signal « alimentation stabilisée » de la passerelle |
| `GTW_BMP_PMIC_PWR_ON` | — | Mise sous tension du circuit d'alimentation de la passerelle |
| `GTW_bmpState` | — | État du processeur de gestion d'énergie de la passerelle. Valeurs : BMP_STATE_OFF (0), BMP_STATE_ON (1), BMP_STATE_ASLEEP (2), BMP_STATE_MIA (3), BMP_STATE_RESET (4), BMP_STATE_POWER_CYCLE (5), DUMMY (255) |

### 0x118 — DriveSystemStatus

**Système de traction : rapport engagé (P/R/N/D), couple, vitesse, état de l'onduleur, pédale** | Cycle : 10 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `DI_accelPedalPos` | % | Onduleur — Accélération pédale pos. Valeurs : SNA (255) |
| `DI_brakePedalState` | — | État de la pédale de frein (appuyée ou non). Valeurs : OFF (0), ON (1), INVALID (2) |
| `DI_driveBlocked` | — | Onduleur — Traction bloqué. Valeurs : DRIVE_BLOCKED_NONE (0), DRIVE_BLOCKED_FRUNK (1), DRIVE_BLOCKED_PROX (2) |
| `DI_epbRequest` | — | Onduleur — Frein de stationnement électrique (EPB) demande. Valeurs : DI_EPBREQUEST_NO_REQUEST (0), DI_EPBREQUEST_PARK (1), DI_EPBREQUEST_UNPARK (2) |
| `DI_gear` | — | Rapport engagé (P = Parking, R = Recul, N = Neutre, D = Marche). Valeurs : DI_GEAR_INVALID (0), DI_GEAR_P (1), DI_GEAR_R (2), DI_GEAR_N (3), DI_GEAR_D (4), DI_GEAR_SNA (7) |
| `DI_immobilizerState` | — | Onduleur — Immobilizer état. Valeurs : DI_IMM_STATE_INIT_SNA (0), DI_IMM_STATE_REQUEST (1), DI_IMM_STATE_AUTHENTICATING (2), DI_IMM_STATE_DISARMED (3), DI_IMM_STATE_IDLE (4), DI_IMM_STATE_RESET (5), DI_IMM_STATE_FAULT (6) |
| `DI_keepDrivePowerStateRequest` | — | Demande de maintien de l'alimentation en mode traction. Valeurs : NO_REQUEST (0), KEEP_ALIVE (1) |
| `DI_proximity` | — | Onduleur — Proximité (signal PP) |
| `DI_regenLight` | — | Témoin de freinage régénératif allumé |
| `DI_systemState` | — | Onduleur — Système état. Valeurs : DI_SYS_UNAVAILABLE (0), DI_SYS_IDLE (1), DI_SYS_STANDBY (2), DI_SYS_FAULT (3), DI_SYS_ABORT (4), DI_SYS_ENABLE (5) |
| `DI_systemStatusChecksum` | — | Somme de contrôle de l'état système de l'onduleur |
| `DI_systemStatusCounter` | — | Onduleur — Système état compteur |
| `DI_trackModeState` | — | Onduleur — Circuit mode état. Valeurs : TRACK_MODE_UNAVAILABLE (0), TRACK_MODE_AVAILABLE (1), TRACK_MODE_ON (2) |
| `DI_tractionControlMode` | — | Onduleur — Traction contrôle mode. Valeurs : Standard (0), Slip Start (1), Dev1 (2), Dev2 (3), Rolls Mode (4), Dyno Mode (5), Offroad Assist (6) |

### 0x119 — VCSEC_windowRequests

**Commandes de lève-vitres : ouverture/fermeture de chaque vitre, pourcentage**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCSEC_hvacRunScreenProtectOnly` | — | Climatisation limitée à la protection de l'écran |
| `VCSEC_windowRequestLF` | — | Commande de la vitre avant gauche |
| `VCSEC_windowRequestLR` | — | Commande de la vitre arrière gauche |
| `VCSEC_windowRequestPercent` | — | Pourcentage d'ouverture de la vitre commandée. Valeurs : SNA (127) |
| `VCSEC_windowRequestRF` | — | Commande de la vitre avant droite |
| `VCSEC_windowRequestRR` | — | Commande de la vitre arrière droite |
| `VCSEC_windowRequestType` | — | Type de commande de vitre (ouverture, fermeture, arrêt). Valeurs : WINDOW_REQUEST_IDLE (0), WINDOW_REQUEST_GOTO_PERCENT (1), WINDOW_REQUEST_GOTO_CRACKED (2), WINDOW_REQUEST_GOTO_CLOSED (3) |

### 0x122 — VCLEFT_doorStatus2

**Portières côté gauche (suite) : informations complémentaires sur l'état des portières**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCLEFT_frontDoorState` | — | Côté gauche — Avant portière état. Valeurs : DOOR_STATE_UNKNOWN (0), DOOR_STATE_CLOSED (1), DOOR_STATE_WAIT_FOR_SHORT_DROP (2), DOOR_STATE_RELEASING_LATCH (3), DOOR_STATE_OPEN_OR_AJAR (4) |
| `VCLEFT_frontHandle5vEnable` | — | Alimentation 5 V de la poignée avant gauche activée |
| `VCLEFT_frontHandleDebounceStatus` | — | État anti-rebond de la poignée avant gauche. Valeurs : EXTERIOR_HANDLE_STATUS_SNA (0), EXTERIOR_HANDLE_STATUS_INDETERMINATE (1), EXTERIOR_HANDLE_STATUS_NOT_ACTIVE (2), EXTERIOR_HANDLE_STATUS_ACTIVE (3), EXTERIOR_HANDLE_STATUS_DISCONNECTED (4), EXTERIOR_HANDLE_STATUS_FAULT (5) |
| `VCLEFT_frontHandleRawStatus` | — | Côté gauche — Avant poignée brut état. Valeurs : EXTERIOR_HANDLE_STATUS_SNA (0), EXTERIOR_HANDLE_STATUS_INDETERMINATE (1), EXTERIOR_HANDLE_STATUS_NOT_ACTIVE (2), EXTERIOR_HANDLE_STATUS_ACTIVE (3), EXTERIOR_HANDLE_STATUS_DISCONNECTED (4), EXTERIOR_HANDLE_STATUS_FAULT (5) |
| `VCLEFT_frontLatchRelDuty` | % | Côté gauche — Avant loquet relais rapport cyclique |
| `VCLEFT_mirrorFoldMaxCurrent` | A | Côté gauche — Rétroviseur pliage maximum courant |
| `VCLEFT_rearDoorState` | — | Côté gauche — Arrière portière état. Valeurs : DOOR_STATE_UNKNOWN (0), DOOR_STATE_CLOSED (1), DOOR_STATE_WAIT_FOR_SHORT_DROP (2), DOOR_STATE_RELEASING_LATCH (3), DOOR_STATE_OPEN_OR_AJAR (4) |
| `VCLEFT_rearHandle5vEnable` | — | Alimentation 5 V de la poignée arrière gauche activée |
| `VCLEFT_rearHandleDebounceStatus` | — | État anti-rebond de la poignée arrière gauche. Valeurs : EXTERIOR_HANDLE_STATUS_SNA (0), EXTERIOR_HANDLE_STATUS_INDETERMINATE (1), EXTERIOR_HANDLE_STATUS_NOT_ACTIVE (2), EXTERIOR_HANDLE_STATUS_ACTIVE (3), EXTERIOR_HANDLE_STATUS_DISCONNECTED (4), EXTERIOR_HANDLE_STATUS_FAULT (5) |
| `VCLEFT_rearHandleRawStatus` | — | Côté gauche — Arrière poignée brut état. Valeurs : EXTERIOR_HANDLE_STATUS_SNA (0), EXTERIOR_HANDLE_STATUS_INDETERMINATE (1), EXTERIOR_HANDLE_STATUS_NOT_ACTIVE (2), EXTERIOR_HANDLE_STATUS_ACTIVE (3), EXTERIOR_HANDLE_STATUS_DISCONNECTED (4), EXTERIOR_HANDLE_STATUS_FAULT (5) |
| `VCLEFT_rearLatchRelDuty` | % | Côté gauche — Arrière loquet relais rapport cyclique |
| `VCLEFT_vehicleInMotion` | — | Véhicule en mouvement (détecté côté gauche) |

### 0x123 — UI_alertMatrix1

**Matrice d'alertes de l'interface : 64 codes d'erreur et d'avertissement du système**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_a001_DriverDoorOpen` | — | Alerte nº 001 : conducteur portière ouvert |
| `UI_a002_DoorOpen` | — | Alerte nº 002 : portière ouvert |
| `UI_a003_TrunkOpen` | — | Alerte nº 003 : coffre ouvert |
| `UI_a004_FrunkOpen` | — | Alerte nº 004 : coffre avant (frunk) ouvert |
| `UI_a005_HeadlightsOnDoorOpen` | — | Alerte nº 005 : headlights activé portière ouvert |
| `UI_a006_RemoteServiceAlert` | — | Alerte nº 006 : alerte de service à distance |
| `UI_a007_SoftPackConfigMismatch` | — | Alerte nº 007 : incohérence de configuration logicielle du pack batterie |
| `UI_a008_TouchScreenError` | — | Alerte nº 008 : tactile écran erreur |
| `UI_a009_SquashfsError` | — | Alerte nº 009 : squashfs erreur |
| `UI_a010_MapsMissing` | — | Alerte nº 010 : données cartographiques manquantes |
| `UI_a011_IncorrectMap` | — | Alerte nº 011 : incorrect carte |
| `UI_a012_NotOnPrivateProperty` | — | Alerte nº 012 : non activé private property |
| `UI_a013_TPMSHardWarning` | — | Alerte nº 013 : alerte critique de pression des pneus (TPMS) |
| `UI_a014_TPMSSoftWarning` | — | Alerte nº 014 : alerte modérée de pression des pneus (TPMS) |
| `UI_a015_TPMSOverPressureWarning` | — | Alerte nº 015 : avertissement de surpression pneu (TPMS) |
| `UI_a016_TPMSTemperatureWarning` | — | Alerte nº 016 : alerte de température des pneus (TPMS) |
| `UI_a017_TPMSSystemFault` | — | Alerte nº 017 : défaut du système de surveillance des pneus (TPMS) |
| `UI_a018_SlipStartOn` | — | Alerte nº 018 : mode démarrage sur sol glissant (anti-patinage) activé |
| `UI_a019_ParkBrakeFault` | — | Alerte nº 019 : défaut du frein de stationnement |
| `UI_a020_SteeringReduced` | — | Alerte nº 020 : direction reduced |
| `UI_a021_RearSeatbeltUnbuckled` | — | Alerte nº 021 : arrière ceinture de sécurité unbuckled |
| `UI_a022_ApeFusesEtc` | — | Alerte nº 022 : fusibles APE (processeur Autopilot) et composants associés |
| `UI_a023_CellInternetCheckFailed` | — | Alerte nº 023 : échec de la vérification Internet via le réseau cellulaire |
| `UI_a024_WifiInternetCheckFailed` | — | Alerte nº 024 : échec de la vérification Internet via WiFi |
| `UI_a025_WifiOnlineCheckFailed` | — | Alerte nº 025 : échec de la vérification de connexion WiFi |
| `UI_a026_ModemResetLoopDetected` | — | Alerte nº 026 : boucle de réinitialisation du modem détectée |
| `UI_a027_AutoSteerMIA` | — | Alerte nº 027 : automatique direction absent (MIA) |
| `UI_a028_FrontTrunkPopupClosed` | — | Alerte nº 028 : avant coffre popup fermé |
| `UI_a029_ModemMIA` | — | Alerte nº 029 : modem absent (MIA) |
| `UI_a030_ModemVMCrash` | — | Alerte nº 030 : plantage de la machine virtuelle du modem |
| `UI_a031_BrakeFluidLow` | — | Alerte nº 031 : niveau de liquide de frein bas |
| `UI_a032_CellModemRecoveryResets` | — | Alerte nº 032 : réinitialisations de récupération du modem cellulaire |
| `UI_a033_ApTrialExpired` | — | Alerte nº 033 : ap essai expired |
| `UI_a034_WakeupProblem` | — | Alerte nº 034 : wakeup problem |
| `UI_a035_AudioWatchdogKernelError` | — | Alerte nº 035 : erreur noyau du chien de garde audio |
| `UI_a036_AudioWatchdogHfpError` | — | Alerte nº 036 : erreur du chien de garde audio (profil mains libres) |
| `UI_a037_AudioWatchdogXrunStormEr` | — | Alerte nº 037 : erreur du chien de garde audio (rafale de dépassements tampon) |
| `UI_a038_AudioWatchdogA2bI2cLocku` | — | Alerte nº 038 : erreur du chien de garde audio (blocage bus I2C/A2B) |
| `UI_a039_AudioA2bNeedRediscovery` | — | Alerte nº 039 : le bus audio A2B nécessite une redécouverte |
| `UI_a040_HomelinkTransmit` | — | Alerte nº 040 : HomeLink (ouvre-porte) transmit |
| `UI_a041_AudioDmesgXrun` | — | Alerte nº 041 : dépassement de tampon audio détecté dans les journaux système |
| `UI_a042_AudioDmesgRtThrottling` | — | Alerte nº 042 : limitation temps réel (throttling) audio dans les journaux système |
| `UI_a043_InvalidMapDataOverride` | — | Alerte nº 043 : invalide carte données forçage |
| `UI_a044_AudioDmesgDspException` | — | Alerte nº 044 : exception du processeur audio (DSP) dans les journaux système |
| `UI_a045_ECallNeedsService` | — | Alerte nº 045 : l'appel d'urgence (eCall) nécessite une intervention |
| `UI_a046_BackupCameraStreamError` | — | Alerte nº 046 : erreur de flux de la caméra de recul |
| `UI_a047_CellRoamingDisallowed` | — | Alerte nº 047 : cellulaire itinérance interdit |
| `UI_a048_AudioPremiumAmpCheckFail` | — | Alerte nº 048 : échec de la vérification de l'amplificateur audio premium |
| `UI_a049_BrakeShiftRequired` | — | Alerte nº 049 : frein changement de rapport requis |
| `UI_a050_BackupCameraIPUTimeout` | — | Alerte nº 050 : expiration du délai IPU de la caméra de recul |
| `UI_a051_BackupCameraFrameTimeout` | — | Alerte nº 051 : expiration du délai d'image de la caméra de recul |
| `UI_a052_KernelPanicReported` | — | Alerte nº 052 : noyau défaillance critique (kernel panic) reported |
| `UI_a053_QtCarExitError` | — | Alerte nº 053 : erreur de fermeture de l'application Qt du véhicule |
| `UI_a054_AudioBoostPowerBad` | — | Alerte nº 054 : audio assistance puissance défaillant |
| `UI_a055_ManualECallDisabled` | — | Alerte nº 055 : appel d'urgence manuel (eCall) désactivé |
| `UI_a056_ManualECallButtonDisconn` | — | Alerte nº 056 : bouton d'appel d'urgence manuel (eCall) déconnecté |
| `UI_a057_CellAntennaDisconnected` | — | Alerte nº 057 : cellulaire antenne déconnecté |
| `UI_a058_GPSAntennaDisconnected` | — | Alerte nº 058 : GPS antenne déconnecté |
| `UI_a059_ECallSpeakerDisconnected` | — | Alerte nº 059 : haut-parleur d'appel d'urgence (eCall) déconnecté |
| `UI_a060_ECallMicDisconnected` | — | Alerte nº 060 : microphone d'appel d'urgence (eCall) déconnecté |
| `UI_a061_SIMTestFailed` | — | Alerte nº 061 : échec du test de la carte SIM |
| `UI_a062_ENSTestFailed` | — | Alerte nº 062 : échec du test du système de stockage d'énergie (ENS) |
| `UI_a063_CellularTestFailed` | — | Alerte nº 063 : échec du test du module cellulaire |
| `UI_a064_ModemFirmwareTestFailed` | — | Alerte nº 064 : échec du test du micrologiciel modem |

### 0x126 — RearHVStatus

**État haute tension de l'onduleur arrière : tension, courant moteur, fréquence de commutation** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `RearHighVoltage126` | V | Tension haute tension de l'onduleur arrière |
| `RearMotorCurrent126` | A | Courant du moteur arrière |
| `DIR_switchingFrequency` | kHz | Fréquence de commutation de l'onduleur arrière |
| `DIR_dcCableCurrentEst` | A | Courant estimé dans le câble DC de l'onduleur arrière |
| `DIR_vBatQF` | — | Indicateur de qualité de la tension batterie (arrière). Valeurs : NOT_QUALIFIED (0), QUALIFIED (1) |
| `DIR_targetFluxMode` | — | Mode de flux magnétique cible de l'onduleur arrière |

### 0x127 — LeftRearHVStatus

**État haute tension de l'onduleur arrière gauche : tension, courant, fréquence de commutation**

| Signal | Unité | Description |
|--------|-------|-------------|
| `LeftRear_targetFluxMode` | — | Mode de flux magnétique — onduleur arrière gauche |
| `LeftRear_switchingFrequency` | kHz | Fréquence de commutation — onduleur arrière gauche |
| `LeftRear_dcCableCurrentEst` | A | Courant estimé dans le câble DC — onduleur arrière gauche |
| `LeftRear_motorCurrent` | A | Courant moteur — onduleur arrière gauche |
| `LeftRear_vBatQF` | — | Qualité de la mesure de tension batterie — arrière gauche |
| `LeftRear_vBat` | V | Tension batterie vue par l'onduleur arrière gauche |

### 0x129 — SteeringAngle

**Angle du volant de direction et vitesse de rotation** | Cycle : 10 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `SteeringSensorC129` | — | Valeur du capteur de direction C |
| `SteeringSensorB129` | — | Valeur du capteur de direction B |
| `SteeringSensorA129` | — | Valeur du capteur de direction A |
| `SteeringSpeed129` | D/S | Vitesse de rotation du volant |
| `SteeringAngle129` | Deg | Angle du volant de direction |

### 0x12A — RightRearHVStatus

**État haute tension de l'onduleur arrière droit : tension, courant, fréquence de commutation**

| Signal | Unité | Description |
|--------|-------|-------------|
| `RightRear_targetFluxMode` | — | Mode de flux magnétique — onduleur arrière droit |
| `RightRear_switchingFrequency` | kHz | Fréquence de commutation — onduleur arrière droit |
| `RightRear_dcCableCurrentEst` | A | Courant estimé dans le câble DC — onduleur arrière droit |
| `RightRear_motorCurrent` | A | Courant moteur — onduleur arrière droit |
| `RightRear_vBatQF` | — | Qualité de la mesure de tension batterie — arrière droit |
| `RightRear_vBat` | V | Tension batterie vue par l'onduleur arrière droit |

### 0x132 — HVBattAmpVolt

**Batterie haute tension : tension totale, courant brut et lissé, temps restant de charge** | Cycle : 10 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `ChargeHoursRemaining132` | Min | Temps restant de charge (en minutes) |
| `BattVoltage132` | V | Tension totale de la batterie haute tension |
| `RawBattCurrent132` | A | Courant brut de la batterie haute tension |
| `SmoothBattCurrent132` | A | Courant lissé de la batterie haute tension |

### 0x13D — P_chargeStatus

**État du port de charge : type de chargeur (AC/DC), limite de courant, état du contacteur HV**

| Signal | Unité | Description |
|--------|-------|-------------|
| `CP_acChargeCurrentLimit` | A | Limite de courant de charge AC |
| `CP_chargeShutdownRequest` | — | Demande d'arrêt de la charge. Valeurs : NO_SHUTDOWN_REQUESTED (0), GRACEFUL_SHUTDOWN_REQUESTED (1), EMERGENCY_SHUTDOWN_REQUESTED (2) |
| `CP_evseChargeType` | — | Type de chargeur connecté (AC, DC ou aucun). Valeurs : NO_CHARGER_PRESENT (0), DC_CHARGER_PRESENT (1), AC_CHARGER_PRESENT (2) |
| `CP_hvChargeStatus` | — | État du circuit de charge haute tension. Valeurs : CP_CHARGE_INACTIVE (0), CP_CHARGE_CONNECTED (1), CP_CHARGE_STANDBY (2), CP_EXT_EVSE_TEST_ACTIVE (3), CP_EVSE_TEST_PASSED (4), CP_CHARGE_ENABLED (5), CP_CHARGE_FAULTED (6) |
| `CP_internalMaxAcCurrentLimit` | A | Limite interne de courant de charge AC |
| `CP_internalMaxDcCurrentLimit` | A | Limite interne de courant de charge DC |
| `CP_vehicleIsoCheckRequired` | — | Vérification d'isolation du véhicule requise |
| `CP_vehiclePrechargeRequired` | — | Précharge du véhicule requise |

### 0x142 — VCLEFT_liftgateStatus

**État du hayon (liftgate) côté gauche**

_Aucun signal défini._

### 0x154 — RearTorqueOld

**Couple moteur arrière (ancien format)** | Cycle : 10 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `RAWTorqueRear154` | NM | Couple brut du moteur arrière |

### 0x186 — IF_torque

**Onduleur avant : couple demandé, couple mesuré, vitesse de l'essieu, position pédale**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DIF_axleSpeed` | RPM | Vitesse de l'essieu avant. Valeurs : SNA (32768) |
| `DIF_slavePedalPos` | % | Position de la pédale (secondaire, avant). Valeurs : SNA (255) |
| `DIF_torqueActual` | Nm | Couple réel mesuré sur l'essieu avant. Valeurs : SNA (4096) |
| `DIF_torqueChecksum` | — | Somme de contrôle du message couple avant |
| `DIF_torqueCommand` | Nm | Couple demandé à l'essieu avant. Valeurs : SNA (4096) |
| `DIF_torqueCounter` | — | Compteur de séquence du message couple avant |
| `DIF_axleSpeedQF` | — | Indicateur de qualité de la vitesse essieu avant |

### 0x1A5 — rontHVStatus

**État haute tension de l'onduleur avant : tension, courant moteur, fréquence de commutation**

| Signal | Unité | Description |
|--------|-------|-------------|
| `FrontHighVoltage1A5` | V | Tension haute tension de l'onduleur avant |
| `FrontMotorCurrent1A5` | A | Courant du moteur avant |
| `DIF_switchingFrequency` | kHz | Fréquence de commutation de l'onduleur avant |
| `DIF_dcCableCurrentEst` | A | Courant estimé dans le câble DC de l'onduleur avant |
| `DIF_vBatQF` | — | Indicateur de qualité de la tension batterie (avant). Valeurs : NOT_QUALIFIED (0), QUALIFIED (1) |
| `DIF_targetFluxMode` | — | Mode de flux magnétique cible de l'onduleur avant |

### 0x1D4 — rontTorqueOld

**Couple moteur avant (ancien format)** | Cycle : 10 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `RAWTorqueFront1D4` | NM | Couple brut du moteur avant |

### 0x1D5 — rontTorque

**Couple moteur avant : demandé, mesuré, limites de couple**

| Signal | Unité | Description |
|--------|-------|-------------|
| `FrontTorqueRequest1D5` | NM | Avant couple request1d5 |
| `FrontTorque1D5` | NM | Couple moteur avant (signal brut 0x1D5) |

### 0x1D6 — I_limits

**Limites de l'onduleur de traction : couple max, courant max, puissance max, température**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DI_limitBaseSpeed` | — | Onduleur — Limite base vitesse |
| `DI_limitClutch` | — | Limite de couple de l'embrayage |
| `DI_limitDcCapTemp` | — | Onduleur — Limite courant continu (DC) capacité température |
| `DI_limitDeltaFluidTemp` | — | Limite d'écart de température du liquide de refroidissement |
| `DI_limitDiff` | — | Onduleur — Limite différentiel |
| `DI_limitDischargePower` | — | Onduleur — Limite décharge puissance |
| `DI_limitDriveTorque` | — | Onduleur — Limite traction couple |
| `DI_limitGracefulPowerOff` | — | Onduleur — Limite progressif puissance désactivé |
| `DI_limitIBat` | — | Onduleur — Limite I batterie |
| `DI_limitIGBTJunctTemp` | — | Onduleur — Limite IGBT junct température |
| `DI_limitInverterTemp` | — | Onduleur — Limite onduleur température |
| `DI_limitLimpMode` | — | Onduleur — Limite mode dégradé mode |
| `DI_limitMotorCurrent` | — | Onduleur — Limite moteur courant |
| `DI_limitMotorSpeed` | — | Onduleur — Limite moteur vitesse |
| `DI_limitMotorVoltage` | — | Onduleur — Limite moteur tension |
| `DI_limitObstacleDetection` | — | Onduleur — Limite obstacle detection |
| `DI_limitOilPumpFluidTemp` | — | Limite de température du liquide de la pompe à huile |
| `DI_limitPCBTemp` | — | Onduleur — Limite carte électronique température |
| `DI_limitPoleTemp` | — | Onduleur — Limite pole température |
| `DI_limitRegenPower` | — | Onduleur — Limite récupération puissance |
| `DI_limitRegenTorque` | — | Onduleur — Limite récupération couple |
| `DI_limitRotorTemp` | — | Onduleur — Limite rotor température |
| `DI_limitShift` | — | Onduleur — Limite changement de rapport |
| `DI_limitShockTorque` | — | Onduleur — Limite shock couple |
| `DI_limitStatorFrequency` | — | Limite de fréquence du stator atteinte |
| `DI_limitStatorTemp` | — | Limite de température du stator atteinte |
| `DI_limitTCDrive` | — | Onduleur — Limite TC traction |
| `DI_limitTCRegen` | — | Onduleur — Limite TC récupération |
| `DI_limitVBatHigh` | — | Onduleur — Limite V batterie haut |
| `DI_limitVBatLow` | — | Onduleur — Limite V batterie bas |
| `DI_limitVehicleSpeed` | — | Onduleur — Limite véhicule vitesse |
| `DI_limitdcLinkCapTemp` | — | Limite de température du condensateur de liaison DC |
| `DI_limithvDcCableTemp` | — | Limite de température du câble DC haute tension |
| `DI_limitnegDcBusbarTemp` | — | Limite de température de la barre de connexion DC négative |
| `DI_limitphaseOutBusBarWeldTemp` | — | Limite de température de la soudure barre de connexion de sortie |
| `DI_limitphaseOutBusbarTemp` | — | Limite de température de la barre de connexion de sortie de phase |
| `DI_limitphaseOutLugTemp` | — | Limite de température de la cosse de sortie de phase |
| `DI_limitposDcBusbarTemp` | — | Limite de température de la barre de connexion DC positive |

### 0x1D8 — RearTorque

**Couple moteur arrière : demandé, mesuré, limites de couple, freinage régénératif**

| Signal | Unité | Description |
|--------|-------|-------------|
| `Checksum1D8` | — | Somme de contrôle du message 0x1D8 |
| `Counter1D8` | — | Counter1d8 |
| `RearTorqueRequest1D8` | NM | Arrière couple request1d8 |
| `RearTorque1D8` | NM | Couple moteur arrière (signal brut 0x1D8) |
| `TorqueFlags1D8` | — | Couple flags1d8 |

### 0x201 — VCFRONT_loggingAndVitals10H

**Journalisation du contrôleur avant (10 Hz) : températures moteur, onduleur, liquide de refroidissement, état des vannes**

_Aucun signal défini._

### 0x204 — PCS_chgStatus

**État du chargeur embarqué (PCS) : courant, tension, phases, température**

| Signal | Unité | Description |
|--------|-------|-------------|
| `PCS_chargeShutdownRequest` | — | Demande d'arrêt de la charge (convertisseur PCS). Valeurs : NO_SHUTDOWN_REQUESTED (0), GRACEFUL_SHUTDOWN_REQUESTED (1), EMERGENCY_SHUTDOWN_REQUESTED (2) |
| `PCS_chgInstantAcPowerAvailable` | kW | Puissance AC instantanée disponible pour la charge |
| `PCS_chgMainState` | — | État principal du chargeur embarqué. Valeurs possibles : PCS_CHG_STATE_INIT (0), PCS_CHG_STATE_IDLE (1), PCS_CHG_STATE_STARTUP (2), PCS_CHG_STATE_WAIT_FOR_LINE_VOLTAGE (3), PCS_CHG_STATE_QUALIFY_LINE_CONFIG (4), PCS_CHG_STATE_SYSTEM_CONFIG (5), PCS_CHG_STATE_ENABLE (6), PCS_CHG_STATE_SHUTDOWN (7), PCS_CHG_STATE_FAULTED (8), PCS_CHG_STATE_CLEAR_FAULTS (9) |
| `PCS_chgMaxAcPowerAvailable` | kW | Puissance AC maximale disponible pour la charge |
| `PCS_chgPHAEnable` | — | Phase A du chargeur activée |
| `PCS_chgPHALineCurrentRequest` | A | Courant demandé sur la phase A du chargeur |
| `PCS_chgPHBEnable` | — | Phase B du chargeur activée |
| `PCS_chgPHBLineCurrentRequest` | A | Courant demandé sur la phase B du chargeur |
| `PCS_chgPHCEnable` | — | Phase C du chargeur activée |
| `PCS_chgPHCLineCurrentRequest` | A | Courant demandé sur la phase C du chargeur |
| `PCS_chgPwmEnableLine` | — | Activation du signal PWM du chargeur sur la ligne |
| `PCS_gridConfig` | — | Configuration de raccordement au réseau électrique (PCS). Valeurs : GRID_CONFIG_SNA (0), GRID_CONFIG_SINGLE_PHASE (1), GRID_CONFIG_THREE_PHASE (2), GRID_CONFIG_THREE_PHASE_DELTA (3) |
| `PCS_hvChargeStatus` | — | Convertisseur (PCS) — Haute tension charge état. Valeurs : PCS_CHARGE_STANDBY (0), PCS_CHARGE_BLOCKED (1), PCS_CHARGE_ENABLED (2), PCS_CHARGE_FAULTED (3) |
| `PCS_hwVariantType` | — | Convertisseur (PCS) — Hw variant type. Valeurs : PCS_48A_SINGLE_PHASE_VARIANT (0), PCS_32A_SINGLE_PHASE_VARIANT (1), PCS_THREE_PHASES_VARIANT (2), PCS_HW_VARIANT_TYPE_SNA (3) |

### 0x20A — HVP_contactorState

**État des contacteurs du pack haute tension (HVP) : positif, négatif, précharge, charge rapide**

| Signal | Unité | Description |
|--------|-------|-------------|
| `HVP_dcLinkAllowedToEnergize` | — | Autorisation de mise sous tension de la liaison DC |
| `HVP_fcContNegativeAuxOpen` | — | Pack haute tension — Charge rapide (DC) cont negative aux ouvert |
| `HVP_fcContNegativeState` | — | Pack haute tension — Charge rapide (DC) cont negative état. Valeurs : CONTACTOR_STATE_SNA (0), CONTACTOR_STATE_OPEN (1), CONTACTOR_STATE_PRECHARGE (2), CONTACTOR_STATE_BLOCKED (3), CONTACTOR_STATE_PULLED_IN (4), CONTACTOR_STATE_OPENING (5), CONTACTOR_STATE_ECONOMIZED (6), CONTACTOR_STATE_WELDED (7) |
| `HVP_fcContPositiveAuxOpen` | — | Pack haute tension — Charge rapide (DC) cont positive aux ouvert |
| `HVP_fcContPositiveState` | — | Pack haute tension — Charge rapide (DC) cont positive état. Valeurs : CONTACTOR_STATE_SNA (0), CONTACTOR_STATE_OPEN (1), CONTACTOR_STATE_PRECHARGE (2), CONTACTOR_STATE_BLOCKED (3), CONTACTOR_STATE_PULLED_IN (4), CONTACTOR_STATE_OPENING (5), CONTACTOR_STATE_ECONOMIZED (6), CONTACTOR_STATE_WELDED (7) |
| `HVP_fcContactorSetState` | — | Pack haute tension — Charge rapide (DC) contacteur set état. Valeurs possibles : CONTACTOR_SET_STATE_SNA (0), CONTACTOR_SET_STATE_OPEN (1), CONTACTOR_SET_STATE_CLOSING (2), CONTACTOR_SET_STATE_BLOCKED (3), CONTACTOR_SET_STATE_OPENING (4), CONTACTOR_SET_STATE_CLOSED (5), CONTACTOR_SET_STATE_PARTIAL_WELD (6), CONTACTOR_SET_STATE_WELDED (7), CONTACTOR_SET_STATE_POSITIVE_CLOSED (8), CONTACTOR_SET_STATE_NEGATIVE_CLOSED (9) |
| `HVP_fcCtrsClosingAllowed` | — | Fermeture des contacteurs de charge rapide (DC) autorisée |
| `HVP_fcCtrsOpenNowRequested` | — | Ouverture immédiate des contacteurs de charge rapide (DC) demandée |
| `HVP_fcCtrsOpenRequested` | — | Ouverture des contacteurs de charge rapide (DC) demandée |
| `HVP_fcCtrsRequestStatus` | — | État de la demande des contacteurs de charge rapide (DC). Valeurs : REQUEST_NOT_ACTIVE (0), REQUEST_ACTIVE (1), REQUEST_COMPLETED (2) |
| `HVP_fcCtrsResetRequestRequired` | — | Réinitialisation des contacteurs de charge rapide (DC) requise |
| `HVP_fcLinkAllowedToEnergize` | — | Autorisation de mise sous tension de la liaison de charge rapide. Valeurs : FC_LINK_ENERGY_NONE (0), FC_LINK_ENERGY_AC (1), FC_LINK_ENERGY_DC (2) |
| `HVP_hvilStatus` | — | État de l'interverrou haute tension (HVIL). Valeurs possibles : UNKNOWN (0), STATUS_OK (1), CURRENT_SOURCE_FAULT (2), INTERNAL_OPEN_FAULT (3), VEHICLE_OPEN_FAULT (4), PENTHOUSE_LID_OPEN_FAULT (5), UNKNOWN_LOCATION_OPEN_FAULT (6), VEHICLE_NODE_FAULT (7), NO_12V_SUPPLY (8), VEHICLE_OR_PENTHOUSE_LID_OPEN_FAULT (9) |
| `HVP_packContNegativeState` | — | Pack haute tension — Pack batterie cont negative état. Valeurs : CONTACTOR_STATE_SNA (0), CONTACTOR_STATE_OPEN (1), CONTACTOR_STATE_PRECHARGE (2), CONTACTOR_STATE_BLOCKED (3), CONTACTOR_STATE_PULLED_IN (4), CONTACTOR_STATE_OPENING (5), CONTACTOR_STATE_ECONOMIZED (6), CONTACTOR_STATE_WELDED (7) |
| `HVP_packContPositiveState` | — | Pack haute tension — Pack batterie cont positive état. Valeurs : CONTACTOR_STATE_SNA (0), CONTACTOR_STATE_OPEN (1), CONTACTOR_STATE_PRECHARGE (2), CONTACTOR_STATE_BLOCKED (3), CONTACTOR_STATE_PULLED_IN (4), CONTACTOR_STATE_OPENING (5), CONTACTOR_STATE_ECONOMIZED (6), CONTACTOR_STATE_WELDED (7) |
| `HVP_packContactorSetState` | — | Pack haute tension — Pack batterie contacteur set état. Valeurs possibles : CONTACTOR_SET_STATE_SNA (0), CONTACTOR_SET_STATE_OPEN (1), CONTACTOR_SET_STATE_CLOSING (2), CONTACTOR_SET_STATE_BLOCKED (3), CONTACTOR_SET_STATE_OPENING (4), CONTACTOR_SET_STATE_CLOSED (5), CONTACTOR_SET_STATE_PARTIAL_WELD (6), CONTACTOR_SET_STATE_WELDED (7), CONTACTOR_SET_STATE_POSITIVE_CLOSED (8), CONTACTOR_SET_STATE_NEGATIVE_CLOSED (9) |
| `HVP_packCtrsClosingAllowed` | — | Fermeture des contacteurs du pack batterie autorisée |
| `HVP_packCtrsOpenNowRequested` | — | Ouverture immédiate des contacteurs du pack batterie demandée |
| `HVP_packCtrsOpenRequested` | — | Ouverture des contacteurs du pack batterie demandée |
| `HVP_packCtrsRequestStatus` | — | État de la demande des contacteurs du pack batterie. Valeurs : REQUEST_NOT_ACTIVE (0), REQUEST_ACTIVE (1), REQUEST_COMPLETED (2) |
| `HVP_packCtrsResetRequestRequired` | — | Réinitialisation des contacteurs du pack batterie requise |
| `HVP_pyroTestInProgress` | — | Test pyrotechnique (fusible de sécurité) en cours |

### 0x20C — VCRIGHT_hvacRequest

**Requêtes de climatisation côté droit : température cible, débit d'air, mode**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCRIGHT_conditioningRequest` | — | Côté droit — Conditioning demande |
| `VCRIGHT_evapPerformanceLow` | — | Côté droit — Évaporateur performance bas |
| `VCRIGHT_hvacBlowerSpeedRPMReq` | RPM | Côté droit — Climatisation soufflerie vitesse tours/min req |
| `VCRIGHT_hvacEvapEnabled` | — | Côté droit — Climatisation évaporateur activé |
| `VCRIGHT_hvacHeatingEnabledLeft` | — | Côté droit — Climatisation chauffage activé gauche |
| `VCRIGHT_hvacHeatingEnabledRight` | — | Côté droit — Climatisation chauffage activé droite |
| `VCRIGHT_hvacPerfTestRunning` | — | Test de performance de la climatisation en cours (côté droit) |
| `VCRIGHT_hvacPerfTestState` | — | État du test de performance de la climatisation. Valeurs : STOPPED (0), WAITING (1), BLOWING (2) |
| `VCRIGHT_hvacUnavailable` | — | Côté droit — Climatisation indisponible |
| `VCRIGHT_tempAmbientRaw` | C | Côté droit — Température ambiante brut. Valeurs : SNA (0) |
| `VCRIGHT_tempEvaporator` | C | Côté droit — Température évaporateur. Valeurs : SNA (2047) |
| `VCRIGHT_tempEvaporatorTarget` | C | Côté droit — Température évaporateur cible. Valeurs : SNA (255) |
| `VCRIGHT_wattsDemandEvap` | W | Côté droit — Watts demande évaporateur |

### 0x212 — MS_status

**État général du BMS : mode de charge, puissance disponible, isolation, contacteurs, alertes** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `BMS_activeHeatingWorthwhile` | — | Le chauffage actif de la batterie est bénéfique |
| `BMS_chargeRequest` | — | Demande de démarrage de la charge |
| `BMS_chargeRetryCount` | — | Nombre de tentatives de relance de la charge |
| `BMS_chgPowerAvailable` | kW | Puissance de charge disponible. Valeurs : SNA (2047) |
| `BMS_contactorState` | — | État des contacteurs haute tension du pack. Valeurs : BMS_CTRSET_SNA (0), BMS_CTRSET_OPEN (1), BMS_CTRSET_OPENING (2), BMS_CTRSET_CLOSING (3), BMS_CTRSET_CLOSED (4), BMS_CTRSET_WELDED (5), BMS_CTRSET_BLOCKED (6) |
| `BMS_cpMiaOnHvs` | — | Perte de communication avec le port de charge côté HT |
| `BMS_diLimpRequest` | — | Demande de mode dégradé (limité) à l'onduleur. Valeurs : LIMP_REQUEST_NONE (0), LIMP_REQUEST_WELDED (1) |
| `BMS_ecuLogUploadRequest` | — | Demande de téléversement des journaux du calculateur. Valeurs : REQUEST_PRIORITY_NONE (0), REQUEST_PRIORITY_1 (1), REQUEST_PRIORITY_2 (2), REQUEST_PRIORITY_3 (3) |
| `BMS_hvState` | — | État du bus haute tension (sous/hors tension). Valeurs : HV_DOWN (0), HV_COMING_UP (1), HV_GOING_DOWN (2), HV_UP_FOR_DRIVE (3), HV_UP_FOR_CHARGE (4), HV_UP_FOR_DC_CHARGE (5), HV_UP (6) |
| `BMS_hvacPowerRequest` | — | Puissance demandée par la climatisation |
| `BMS_isolationResistance` | kOhm | Résistance d'isolement du pack batterie. Valeurs : SNA (1023) |
| `BMS_keepWarmRequest` | — | Demande de maintien en température de la batterie |
| `BMS_notEnoughPowerForDrive` | — | Puissance insuffisante pour la traction |
| `BMS_notEnoughPowerForSupport` | — | Puissance insuffisante pour les systèmes auxiliaires |
| `BMS_okToShipByAir` | — | Batterie approuvée pour le transport aérien |
| `BMS_okToShipByLand` | — | Batterie approuvée pour le transport terrestre |
| `BMS_pcsPwmEnabled` | — | BMS — Pcs commande PWM activé |
| `BMS_preconditionAllowed` | — | Préconditionnement de la batterie autorisé (BMS) |
| `BMS_smStateRequest` | — | BMS — Sm état demande. Valeurs possibles : BMS_STANDBY (0), BMS_DRIVE (1), BMS_SUPPORT (2), BMS_CHARGE (3), BMS_FEIM (4), BMS_CLEAR_FAULT (5), BMS_FAULT (6), BMS_WELD (7), BMS_TEST (8), BMS_SNA (9), BMS_DIAG (10) |
| `BMS_state` | — | BMS — État. Valeurs possibles : BMS_STANDBY (0), BMS_DRIVE (1), BMS_SUPPORT (2), BMS_CHARGE (3), BMS_FEIM (4), BMS_CLEAR_FAULT (5), BMS_FAULT (6), BMS_WELD (7), BMS_TEST (8), BMS_SNA (9), BMS_DIAG (10) |
| `BMS_uiChargeStatus` | — | BMS — Interface charge état. Valeurs : BMS_DISCONNECTED (0), BMS_NO_POWER (1), BMS_ABOUT_TO_CHARGE (2), BMS_CHARGING (3), BMS_CHARGE_COMPLETE (4), BMS_CHARGE_STOPPED (5) |
| `BMS_updateAllowed` | — | Mise à jour logicielle autorisée (BMS) |

### 0x214 — stChargeVA

**Charge rapide (DC) : tension et courant de la borne** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `FC_adapterLocked` | — | Adaptateur de charge rapide verrouillé |
| `FC_dcCurrent` | A | Charge rapide (DC) — Courant continu (DC) courant |
| `FC_dcVoltage` | V | Charge rapide (DC) — Courant continu (DC) tension |
| `FC_leakageTestNotSupported` | — | Test de fuite non supporté par la charge rapide (DC) |
| `FC_minCurrentLimit` | A | Charge rapide (DC) — Minimum courant limite |
| `FC_postID` | — | Identifiant du poste de charge rapide (DC). Valeurs : FC_POST_MASTER (0), FC_POST_SLAVE (1), FC_POST_ID_2 (2), FC_POST_ID_SNA (3) |
| `FC_protocolVersion` | — | Charge rapide (DC) — Protocol version |
| `FC_statusCode` | — | Charge rapide (DC) — État code. Valeurs possibles : FC_STATUS_NOTREADY_SNA (0), FC_STATUS_READY (1), FC_STATUS_UPDATE_IN_PROGRESS (2), FC_STATUS_DEPRECATED_3 (3), FC_STATUS_DEPRECATED_4 (4), FC_STATUS_INT_ISOACTIVE (5), FC_STATUS_EXT_ISOACTIVE (6), FC_STATUS_POST_OUT_OF_SERVICE (7), FC_STATUS_NOTCOMPATIBLE (13), FC_STATUS_MALFUNCTION (14), FC_STATUS_NODATA (15) |
| `FC_type` | — | Charge rapide (DC) — Type. Valeurs : FC_TYPE_SUPERCHARGER (0), FC_TYPE_CHADEMO (1), FC_TYPE_GB (2), FC_TYPE_CC_EVSE (3), FC_TYPE_COMBO (4), FC_TYPE_MC_EVSE (5), FC_TYPE_OTHER (6), FC_TYPE_SNA (7) |

### 0x215 — isolation

**Test d'isolation de la charge rapide (DC)** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `FCIsolation215` | kOhm | Charge rapide (DC) isolation |

### 0x217 — _status3

**État de la charge rapide (DC) — partie 3** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `FC_status3DummySig` | — | Charge rapide (DC) — Status3dummy sig |

### 0x21D — P_evseStatus

**État de la borne de charge (EVSE) : type de prise, proximité, pilote de charge**

| Signal | Unité | Description |
|--------|-------|-------------|
| `CP_acChargeState` | — | Port de charge — Courant alternatif (AC) charge état. Valeurs : AC_CHARGE_INACTIVE (0), AC_CHARGE_CONNECTED_CHARGE_BLOCKED (1), AC_CHARGE_STANDBY (2), AC_CHARGE_ENABLED (3), AC_CHARGE_ONBOARD_CHARGER_SHUTDOWN (4), AC_CHARGE_VEH_SHUTDOWN (5), AC_CHARGE_FAULT (6) |
| `CP_acNumRetries` | — | Port de charge — Courant alternatif (AC) numéro retries |
| `CP_cableCurrentLimit` | A | Port de charge — Câble courant limite |
| `CP_cableType` | — | Port de charge — Câble type. Valeurs : CHG_CABLE_TYPE_IEC (0), CHG_CABLE_TYPE_SAE (1), CHG_CABLE_TYPE_GB_AC (2), CHG_CABLE_TYPE_GB_DC (3), CHG_CABLE_TYPE_SNA (4) |
| `CP_digitalCommsAttempts` | — | Tentatives de communication numérique du port de charge |
| `CP_digitalCommsEstablished` | — | Communication numérique du port de charge établie |
| `CP_evseAccept` | — | Port de charge — Borne de charge (EVSE) accepté |
| `CP_evseChargeType_UI` | — | Type de chargeur connecté affiché à l'interface (AC, DC ou aucun). Valeurs : NO_CHARGER_PRESENT (0), DC_CHARGER_PRESENT (1), AC_CHARGER_PRESENT (2) |
| `CP_evseRequest` | — | Port de charge — Borne de charge (EVSE) demande |
| `CP_gbState` | — | État du protocole de charge GB/T (standard chinois). Valeurs possibles : GBDC_INACTIVE (0), GBDC_WAIT_FOR_COMMS (1), GBDC_COMMS_RECEIVED (2), GBDC_HANDSHAKING_EXT_ISO (3), GBDC_RECOGNITION (4), GBDC_CHARGE_PARAM_CONFIG (5), GBDC_VEH_PACK_PRECHARGE (6), GBDC_READY_TO_CHARGE (7), GBDC_CHARGING (8), GBDC_STOP_CHARGE_REQUESTED (9), GBDC_CHARGE_DISABLING (10), GBDC_END_OF_CHARGE (11), GBDC_ERROR_HANDLING (12), GBDC_RETRY_CHARGE (13), GBDC_FAULTED (14), GBDC_TESTER_PRESENT (15) |
| `CP_gbdcChargeAttempts` | — | Tentatives de charge via le convertisseur GBDC |
| `CP_gbdcFailureReason` | — | Port de charge — Gbdc failure raison. Valeurs : GBDC_FAILURE_NONE (0), GBDC_ATTEMPTS_EXPIRED (1), GBDC_SHUTDOWN_FAILURE (2), GBDC_CRITICAL_FAILURE (3) |
| `CP_gbdcStopChargeReason` | — | Port de charge — Gbdc arrêt charge raison. Valeurs : GBDC_STOP_REASON_NONE (0), GBDC_VEH_REQUESTED (1), GBDC_EVSE_REQUESTED (2), GBDC_COMMS_TIMEOUT (3), GBDC_EVSE_FAULT (4), GBDC_EVSE_CRITICAL_FAULT (5), GBDC_LIVE_DISCONNECT (6), GBDC_SUPERCHARGER_COMMS_TIMEOUT (7) |
| `CP_iecComboState` | — | Port de charge — Iec combo état. Valeurs possibles : IEC_COMBO_INACTIVE (0), IEC_COMBO_CONNECTED (1), IEC_COMBO_V2G_SESSION_SETUP (2), IEC_COMBO_SERVICE_DISCOVERY (3), IEC_COMBO_PAYMENT_SELECTION (4), IEC_COMBO_CHARGE_PARAM_DISCOVERY (5), IEC_COMBO_CABLE_CHECK (6), IEC_COMBO_PRECHARGE (7), IEC_COMBO_ENABLED (8), IEC_COMBO_SHUTDOWN (9), IEC_COMBO_END_OF_CHARGE (10), IEC_COMBO_FAULT (11), IEC_COMBO_WAIT_RESTART (12) |
| `CP_pilot` | — | Port de charge — Pilote (signal CP). Valeurs : CHG_PILOT_NONE (0), CHG_PILOT_FAULTED (1), CHG_PILOT_LINE_CHARGE (2), CHG_PILOT_FAST_CHARGE (3), CHG_PILOT_IDLE (4), CHG_PILOT_INVALID (5), CHG_PILOT_UNUSED_6 (6), CHG_PILOT_SNA (7) |
| `CP_pilotCurrent` | A | Port de charge — Pilote (signal CP) courant |
| `CP_proximity` | — | Port de charge — Proximité (signal PP). Valeurs : CHG_PROXIMITY_SNA (0), CHG_PROXIMITY_DISCONNECTED (1), CHG_PROXIMITY_UNLATCHED (2), CHG_PROXIMITY_LATCHED (3) |
| `CP_teslaDcState` | — | Port de charge — Tesla courant continu (DC) état. Valeurs possibles : TESLA_DC_INACTIVE (0), TESLA_DC_CONNECTED_CHARGE_BLOCKED (1), TESLA_DC_STANDBY (2), TESLA_DC_EXT_TESTS_ENABLED (3), TESLA_DC_EXT_TEST_ACTIVE (4), TESLA_DC_EXT_PRECHARGE_ACTIVE (5), TESLA_DC_ENABLED (6), TESLA_DC_EVSE_SHUTDOWN (7), TESLA_DC_VEH_SHUTDOWN (8), TESLA_DC_EMERGENCY_SHUTDOWN (9), TESLA_DC_FAULT (10) |
| `CP_teslaSwcanState` | — | Port de charge — Tesla swcan état. Valeurs : TESLA_SWCAN_INACTIVE (0), TESLA_SWCAN_ACCEPT (1), TESLA_SWCAN_RECEIVE (2), TESLA_SWCAN_ESTABLISHED (3), TESLA_SWCAN_FAULT (4), TESLA_SWCAN_GO_TO_SLEEP (5), TESLA_SWCAN_OFFBOARD_UPDATE_IN_PROGRESS (6) |

### 0x221 — VCFRONT_LVPowerState

**État de l'alimentation basse tension du contrôleur avant : 12V, 5V, régulateurs**

_Aucun signal défini._

### 0x224 — PCSDCDCstatus

**État du convertisseur DC-DC (12V) : tension, courant, puissance, température** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `DCDC12VSupportRtyCnt224` | — | Dcdc12v support rty cnt224 |
| `DCDC12VSupportStatus224` | — | Dcdc12v support état |
| `DCDCDischargeRtyCnt224` | — | DCDC décharge rty cnt224 |
| `DCDCFaulted224` | — | DCDC en défaut |
| `DCDCHvBusDischargeStatus224` | — | État de décharge du bus haute tension (DCDC) |
| `DCDCInitialPrechargeSubState224` | — | DCDC initial précharge sub état |
| `DCDCoutputCurrent224` | A | Courant de sortie du convertisseur DCDC |
| `DCDCstate224` | — | DCD cstate224. Valeurs : Idle (0), 12vChg (1), PrechargeStart (2), Precharge (3), HVactive (4), Shutdown (5), Error (6) |
| `DCDCoutputCurrentMax224` | A | Courant de sortie maximum du convertisseur DCDC |
| `DCDCOutputIsLimited224` | — | Sortie du convertisseur DC-DC limitée |
| `DCDCPrechargeRestartCnt224` | — | DCDC précharge restart cnt224 |
| `DCDCPrechargeRtyCnt224` | — | DCDC précharge rty cnt224 |
| `DCDCPrechargeStatus224` | — | DCDC précharge état |
| `DCDCPwmEnableLine224` | — | DCDC commande PWM activation ligne |
| `DCDCSubState224` | — | DCDC sub état. Valeurs possibles : PWR_UP_INIT (0), STANDBY (1), 12V_SUPPORT_ACTIVE (2), DIS_HVBUS (3), PCHG_FAST_DIS_HVBUS (4), PCHG_SLOW_DIS_HVBUS (5), PCHG_DWELL_CHARGE (6), PCHG_DWELL_WAIT (7), PCHG_DI_RECOVERY_WAIT (8), PCHG_ACTIVE (9), PCHG_FLT_FAST_DIS_HVBUS (10), SHUTDOWN (11), 12V_SUPPORT_FAULTED (12), DIS_HVBUS_FAULTED (13), PCHG_FAULTED (14), CLEAR_FAULTS (15), FAULTED (16), NUM (17) |
| `DCDCSupportingFixedLvTarget224` | — | Convertisseur DCDC en mode tension basse fixe (cible LV) |
| `PCS_ecuLogUploadRequest224` | — | Convertisseur (PCS) — Calculateur journal téléversement demande |

### 0x225 — VCRIGHT_LVPowerState

**État de l'alimentation basse tension du contrôleur droit : 12V, 5V, régulateurs**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCRIGHT_amplifierLVState` | — | Côté droit — Amplifier basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCRIGHT_cntctrPwrState` | — | Côté droit — Cntctr puissance état |
| `VCRIGHT_eFuseLockoutStatus` | — | État de verrouillage du fusible électronique. Valeurs : EFUSE_LOCKOUT_STATUS_IDLE (0), EFUSE_LOCKOUT_STATUS_PENDING (1), EFUSE_LOCKOUT_STATUS_ACTIVE (2) |
| `VCRIGHT_hvcLVState` | — | Côté droit — Hvc basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCRIGHT_lumbarLVState` | — | Côté droit — Soutien lombaire basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCRIGHT_ocsLVState` | — | Côté droit — Ocs basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCRIGHT_ptcLVState` | — | Côté droit — Résistance PTC (chauffage) basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCRIGHT_rcmLVState` | — | Côté droit — Rcm basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCRIGHT_rearOilPumpLVState` | — | Côté droit — Arrière huile pompe basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCRIGHT_swEnStatus` | — | Côté droit — Interrupteur en état |
| `VCRIGHT_tunerLVState` | — | Côté droit — Tuner basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCRIGHT_vehiclePowerStateDBG` | — | Côté droit — Véhicule puissance état DBG. Valeurs : VEHICLE_POWER_STATE_OFF (0), VEHICLE_POWER_STATE_CONDITIONING (1), VEHICLE_POWER_STATE_ACCESSORY (2), VEHICLE_POWER_STATE_DRIVE (3) |

### 0x227 — MP_state

**État du compresseur de climatisation : vitesse, puissance, température, pression**

| Signal | Unité | Description |
|--------|-------|-------------|
| `CMP_HVOverVoltage` | — | Compresseur : surtension haute tension détectée |
| `CMP_HVUnderVoltage` | — | Sous-tension haute tension (compresseur) |
| `CMP_VCFRONTCANTimeout` | — | Compresseur — Vcfrontcan expiration de délai |
| `CMP_currentSensorCal` | — | Compresseur — Courant capteur cal |
| `CMP_failedStart` | — | Compresseur — Échoué démarrage |
| `CMP_inverterTemperature` | C | Compresseur — Onduleur température |
| `CMP_motorVoltageSat` | — | Compresseur — Moteur tension saturation |
| `CMP_overCurrent` | — | Surintensité (compresseur) |
| `CMP_overTemperature` | — | Compresseur : surchauffe détectée |
| `CMP_ready` | — | Compresseur — Prêt |
| `CMP_repeatOverCurrent` | — | Compresseur : surintensité répétée |
| `CMP_shortCircuit` | — | Court-circuit détecté (compresseur) |
| `CMP_speedDuty` | % | Compresseur — Vitesse rapport cyclique |
| `CMP_speedRPM` | RPM | Compresseur — Vitesse tours/min |
| `CMP_state` | — | Compresseur — État. Valeurs : CMP_STATE_NONE (0), CMP_STATE_NORMAL (1), CMP_STATE_WAIT (2), CMP_STATE_FAULT (3), CMP_STATE_SOFT_START (4), CMP_STATE_SOFT_SHUTDOWN (5), CMP_STATE_SNA (15) |
| `CMP_underTemperature` | — | Sous-température (compresseur) |

### 0x228 — PBrightStatus

**État du frein de stationnement électrique (EPB) côté droit**

| Signal | Unité | Description |
|--------|-------|-------------|
| `EPBR12VFilt228` | V | Epbr12v filt228 |
| `EPBRcsmFaultReason228` | — | Frein de stationnement électrique (EPB) rcsm défaut raison |
| `EPBRdisconnected228` | — | Frein de stationnement électrique (EPB) rdisconnected228 |
| `EPBRCDPQualified228` | — | Epbrcdp qualified228 |
| `EPBResmCaliperRequest228` | — | Frein de stationnement électrique (EPB) resm étrier de frein demande. Valeurs : idle (1), engaging (2), disengaging (3) |
| `EPBResmOperationTrigger228` | — | Frein de stationnement électrique (EPB) resm operation déclenchement. Valeurs : ParkEngaged (1), Released (6), SuperPark (22) |
| `EPBRinternalCDPRequest228` | — | Demande CDP interne du frein de stationnement droit |
| `EPBRinternalStatusChecksum228` | — | Somme de contrôle de l'état interne du frein de stationnement droit |
| `EPBRinternalStatusCounter228` | — | Compteur de séquence de l'état interne du frein de stationnement droit |
| `EPBRlocalServiceModeActive228` | — | Frein de stationnement électrique (EPB) rlocal service mode actif |
| `EPBRlockoutUnlockCount228` | — | Compteur de déverrouillage/verrouillage du frein de stationnement droit |
| `EPBRsummonFaultReason228` | — | Raison de défaut du frein de stationnement droit lors de la convocation (Summon) |
| `EPBRsummonState228` | — | État du frein de stationnement droit lors de la convocation (Summon) |
| `EPBRunitFaultStatus228` | — | État de défaut de l'unité du frein de stationnement droit |
| `EPBRunitStatus228` | — | État de l'unité du frein de stationnement droit. Valeurs : DriveReleased (1), ParkEngaged (3), Engaging (8), Disengaging (10) |

### 0x229 — GearLever

**Levier de vitesses : position sélectionnée, demande de changement**

| Signal | Unité | Description |
|--------|-------|-------------|
| `GearLeverPosition229` | — | Position du levier de vitesses. Valeurs : Center (0), Half Down (1), Full Down (2), Half Up (3), Full Up (4) |
| `GearLeverButton229` | — | Bouton du levier de vitesses |

### 0x22A — HVP_pcsControl

**Commande du pack haute tension (HVP) vers le chargeur embarqué (PCS)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `HVP_dcLinkVoltageFiltered` | V | Pack haute tension — Courant continu (DC) liaison tension filtré. Valeurs : SNA (550) |
| `HVP_dcLinkVoltageRequest` | V | Pack haute tension — Courant continu (DC) liaison tension demande |
| `HVP_pcsChargeHwEnabled` | — | Pack haute tension — Pcs charge hw activé |
| `HVP_pcsControlRequest` | — | Pack haute tension — Pcs contrôle demande. Valeurs : SHUTDOWN (0), SUPPORT (1), PRECHARGE (2), DISCHARGE (3) |
| `HVP_pcsDcdcHwEnabled` | — | Pack haute tension — Pcs dcdc hw activé |

### 0x22E — PARK_sdiRear

**Capteurs de distance de stationnement arrière (ultrason)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `PARK_sdiRearChecksum` | — | Radar de stationnement — Capteur de distance arrière somme de contrôle |
| `PARK_sdiRearCounter` | — | Radar de stationnement — Capteur de distance arrière compteur |
| `PARK_sdiSensor10RawDistData` | cm | Radar de stationnement — Capteur de distance sensor10raw distance données. Valeurs : BLOCKED (0), NEAR_DETECTION (1), NO_OBJECT_DETECTED (500), SNA (511) |
| `PARK_sdiSensor11RawDistData` | cm | Radar de stationnement — Capteur de distance sensor11raw distance données. Valeurs : BLOCKED (0), NEAR_DETECTION (1), NO_OBJECT_DETECTED (500), SNA (511) |
| `PARK_sdiSensor12RawDistData` | cm | Radar de stationnement — Capteur de distance sensor12raw distance données. Valeurs : BLOCKED (0), NEAR_DETECTION (1), NO_OBJECT_DETECTED (500), SNA (511) |
| `PARK_sdiSensor7RawDistData` | cm | Radar de stationnement — Capteur de distance sensor7raw distance données. Valeurs : BLOCKED (0), NEAR_DETECTION (1), NO_OBJECT_DETECTED (500), SNA (511) |
| `PARK_sdiSensor8RawDistData` | cm | Radar de stationnement — Capteur de distance sensor8raw distance données. Valeurs : BLOCKED (0), NEAR_DETECTION (1), NO_OBJECT_DETECTED (500), SNA (511) |
| `PARK_sdiSensor9RawDistData` | cm | Radar de stationnement — Capteur de distance sensor9raw distance données. Valeurs : BLOCKED (0), NEAR_DETECTION (1), NO_OBJECT_DETECTED (500), SNA (511) |

### 0x232 — MS_contactorRequest

**Demandes d'activation des contacteurs batterie par le BMS**

| Signal | Unité | Description |
|--------|-------|-------------|
| `BMS_ensShouldBeActiveForDrive` | — | Le stockage d'énergie doit être actif pour rouler |
| `BMS_fcContactorRequest` | — | Demande de fermeture du contacteur de charge rapide DC. Valeurs : SET_REQUEST_SNA (0), SET_REQUEST_CLOSE (1), SET_REQUEST_OPEN (2), SET_REQUEST_OPEN_IMMEDIATELY (3), SET_REQUEST_CLOSE_NEGATIVE_ONLY (4), SET_REQUEST_CLOSE_POSITIVE_ONLY (5) |
| `BMS_fcLinkOkToEnergizeRequest` | — | Autorisation de mise sous tension pour la charge rapide. Valeurs : FC_LINK_ENERGY_NONE (0), FC_LINK_ENERGY_AC (1), FC_LINK_ENERGY_DC (2) |
| `BMS_gpoHasCompleted` | — | Séquence de mise hors tension terminée |
| `BMS_internalHvilSenseV` | V | Tension du capteur d'interverrou HT (HVIL) interne. Valeurs : SNA (65535) |
| `BMS_packContactorRequest` | — | Demande de fermeture des contacteurs principaux du pack. Valeurs : SET_REQUEST_SNA (0), SET_REQUEST_CLOSE (1), SET_REQUEST_OPEN (2), SET_REQUEST_OPEN_IMMEDIATELY (3), SET_REQUEST_CLOSE_NEGATIVE_ONLY (4), SET_REQUEST_CLOSE_POSITIVE_ONLY (5) |
| `BMS_pcsPwmDisable` | — | BMS — Pcs commande PWM désactivation |

### 0x23D — P_chargeStatus

**État du port de charge : courant, type de chargeur, contacteurs, précharge**

| Signal | Unité | Description |
|--------|-------|-------------|
| `CP_acChargeCurrentLimit` | A | Limite de courant de charge AC |
| `CP_chargeShutdownRequest` | — | Demande d'arrêt de la charge. Valeurs : NO_SHUTDOWN_REQUESTED (0), GRACEFUL_SHUTDOWN_REQUESTED (1), EMERGENCY_SHUTDOWN_REQUESTED (2) |
| `CP_hvChargeStatus` | — | État du circuit de charge haute tension. Valeurs : CP_CHARGE_INACTIVE (0), CP_CHARGE_CONNECTED (1), CP_CHARGE_STANDBY (2), CP_EXT_EVSE_TEST_ACTIVE (3), CP_EVSE_TEST_PASSED (4), CP_CHARGE_ENABLED (5), CP_CHARGE_FAULTED (6) |
| `CP_internalMaxCurrentLimit` | A | Limite interne de courant de charge |
| `CP_vehicleIsoCheckRequired` | — | Vérification d'isolation du véhicule requise |
| `CP_vehiclePrechargeRequired` | — | Précharge du véhicule requise |

### 0x241 — VCFRONT_coolant

**Circuit de refroidissement avant : températures, débits, pompes, vannes, réfrigérant** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCFRONT_coolantAirPurgeBatState` | — | Contrôleur avant — Liquide de refroidissement air purge batterie état. Valeurs : AIR_PURGE_STATE_INACTIVE (0), AIR_PURGE_STATE_ACTIVE (1), AIR_PURGE_STATE_COMPLETE (2), AIR_PURGE_STATE_INTERRUPTED (3), AIR_PURGE_STATE_PENDING (4) |
| `VCFRONT_coolantFlowBatActual` | LPM | Débit réel du liquide de refroidissement vers la batterie |
| `VCFRONT_coolantFlowBatReason` | — | Contrôleur avant — Liquide de refroidissement débit batterie raison. Valeurs possibles : NONE (0), COOLANT_AIR_PURGE (1), NO_FLOW_REQ (2), OVERRIDE_BATT (3), ACTIVE_MANAGER_BATT (4), PASSIVE_MANAGER_BATT (5), BMS_FLOW_REQ (6), DAS_FLOW_REQ (7), OVERRIDE_PT (8), ACTIVE_MANAGER_PT (9), PASSIVE_MANAGER_PT (10), PCS_FLOW_REQ (11), DI_FLOW_REQ (12), DIS_FLOW_REQ (13), HP_FLOW_REQ (14) |
| `VCFRONT_coolantFlowBatTarget` | LPM | Débit ciblé du liquide de refroidissement vers la batterie |
| `VCFRONT_coolantFlowPTActual` | LPM | Débit réel du liquide de refroidissement vers le moteur |
| `VCFRONT_coolantFlowPTReason` | — | Contrôleur avant — Liquide de refroidissement débit PT raison. Valeurs possibles : NONE (0), COOLANT_AIR_PURGE (1), NO_FLOW_REQ (2), OVERRIDE_BATT (3), ACTIVE_MANAGER_BATT (4), PASSIVE_MANAGER_BATT (5), BMS_FLOW_REQ (6), DAS_FLOW_REQ (7), OVERRIDE_PT (8), ACTIVE_MANAGER_PT (9), PASSIVE_MANAGER_PT (10), PCS_FLOW_REQ (11), DI_FLOW_REQ (12), DIS_FLOW_REQ (13), HP_FLOW_REQ (14) |
| `VCFRONT_coolantFlowPTTarget` | LPM | Débit ciblé du liquide de refroidissement vers le moteur |
| `VCFRONT_coolantHasBeenFilled` | — | Le liquide de refroidissement a été rempli |
| `VCFRONT_radiatorIneffective` | — | Contrôleur avant — Radiateur ineffective |
| `VCFRONT_wasteHeatRequestType` | — | Type de demande de dissipation de chaleur résiduelle. Valeurs : WASTE_TYPE_NONE (0), WASTE_TYPE_PARTIAL (1), WASTE_TYPE_FULL (2) |

### 0x242 — VCLEFT_LVPowerState

**État de l'alimentation basse tension du contrôleur gauche : 12V, 5V, régulateurs**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCLEFT_cpLVState` | — | Côté gauche — Cp basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCLEFT_diLVState` | — | Côté gauche — Di basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCLEFT_lumbarLVState` | — | Côté gauche — Soutien lombaire basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCLEFT_rcmLVState` | — | Côté gauche — Rcm basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCLEFT_sccmLVState` | — | Côté gauche — Sccm basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCLEFT_swcLVState` | — | Côté gauche — Swc basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCLEFT_tpmsLVState` | — | Côté gauche — Capteur de pression pneu (TPMS) basse tension état. Valeurs : LV_OFF (0), LV_ON (1), LV_GOING_DOWN (2), LV_FAULT (3) |
| `VCLEFT_vehiclePowerStateDBG` | — | Côté gauche — Véhicule puissance état DBG. Valeurs : VEHICLE_POWER_STATE_OFF (0), VEHICLE_POWER_STATE_CONDITIONING (1), VEHICLE_POWER_STATE_ACCESSORY (2), VEHICLE_POWER_STATE_DRIVE (3) |

### 0x243 — VCRIGHT_hvacStatus

**État de la climatisation côté droit : compresseur, évaporateur, vanne d'expansion**

_Aucun signal défini._

### 0x244 — stChargeLimits

**Limites de la charge rapide (DC) : courant max, tension max, puissance max** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `FCMinVlimit244` | V | Charge rapide (DC) minimum vlimit244 |
| `FCMaxVlimit244` | V | Charge rapide (DC) maximum vlimit244 |
| `FCCurrentLimit244` | A | Charge rapide (DC) courant limite |
| `FCPowerLimit244` | kW | Charge rapide (DC) puissance limite |

### 0x249 — SCCMLeftStalk

**Commodo gauche : clignotants, appels de phares, essuie-glaces**

| Signal | Unité | Description |
|--------|-------|-------------|
| `SCCM_highBeamStalkStatus` | — | État du commodo de feux de route (appels de phares). Valeurs : IDLE (0), PULL (1), PUSH (2), SNA (3) |
| `SCCM_leftStalkCounter` | — | Compteur de séquence du commodo gauche |
| `SCCM_leftStalkCrc` | — | CRC du commodo gauche |
| `SCCM_leftStalkReserved1` | — | Commodo gauche — réservé 1 |
| `SCCM_turnIndicatorStalkStatus` | — | État du commodo de clignotants. Valeurs possibles : IDLE (0), UP_0_5 (1), UP_1 (2), UP_1_5 (3), UP_2 (4), DOWN_0_5 (5), DOWN_1 (6), DOWN_1_5 (7), DOWN_2 (8), SNA (9) |
| `SCCM_washWipeButtonStatus` | — | État du bouton lave-glace / essuie-glace. Valeurs : NOT_PRESSED (0), 1ST_DETENT (1), 2ND_DETENT (2), SNA (3) |

### 0x252 — MS_powerAvailable

**Puissance disponible de la batterie : décharge max, régénération max, limites thermiques** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `BMS_hvacPowerBudget` | kW | Budget de puissance pour la climatisation |
| `BMS_maxDischargePower` | kW | Puissance max autorisée en décharge |
| `BMS_maxRegenPower` | kW | Puissance max de récupération au freinage |
| `BMS_maxStationaryHeatPower` | kW | Puissance max du chauffage à l'arrêt |
| `BMS_notEnoughPowerForHeatPump` | — | Puissance insuffisante pour la pompe à chaleur |
| `BMS_powerLimitsState` | — | BMS — Puissance limites état. Valeurs : POWER_NOT_CALCULATED_FOR_DRIVE (0), POWER_CALCULATED_FOR_DRIVE (1) |

### 0x257 — Ispeed

**Vitesse affichée du véhicule et unité de mesure (km/h ou mph)** | Cycle : 20 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `DI_speedChecksum` | — | Onduleur — Vitesse somme de contrôle |
| `DI_speedCounter` | — | Onduleur — Vitesse compteur |
| `DI_uiSpeed` | — | Vitesse affichée au tableau de bord. Valeurs : DI_UI_SPEED_SNA (511) |
| `DI_uiSpeedHighSpeed` | — | Vitesse affichée (mode haute vitesse). Valeurs : DI_UI_HIGH_SPEED_SNA (511) |
| `DI_uiSpeedUnits` | — | Unité de vitesse (0 = mph, 1 = km/h). Valeurs : DI_SPEED_MPH (0), DI_SPEED_KPH (1) |
| `DI_vehicleSpeed` | kph | Vitesse du véhicule mesurée par l'onduleur. Valeurs : SNA (4095) |

### 0x25D — P_status

**État du port de charge : température, pilote de proximité, état de connexion**

| Signal | Unité | Description |
|--------|-------|-------------|
| `CP_UHF_controlState` | — | Port de charge — UHF contrôle état. Valeurs possibles : CP_UHF_INIT (0), CP_UHF_CONFIG (1), CP_UHF_IDLE (2), CP_UHF_CALIBRATE (3), CP_UHF_PREPARE_RX (4), CP_UHF_RX (5), CP_UHF_CHECK_RX (6), CP_UHF_READ_RXFIFO (7), CP_UHF_HANDLE_FOUND (8), CP_UHF_SLEEP (9), CP_UHF_FAULT (10) |
| `CP_UHF_handleFound` | — | Port de charge — UHF poignée found |
| `CP_apsVoltage` | V | Port de charge — Aps tension |
| `CP_chargeCablePresent` | — | Câble de charge branché (présent). Valeurs : CABLE_NOT_PRESENT (0), CABLE_PRESENT (1) |
| `CP_chargeCableSecured` | — | Port de charge — Charge câble secured |
| `CP_chargeCableState` | — | Port de charge — Charge câble état. Valeurs : CHARGE_CABLE_UNKNOWN_SNA (0), CHARGE_CABLE_NOT_CONNECTED (1), CHARGE_CABLE_CONNECTED (2) |
| `CP_chargeDoorOpen` | — | Port de charge — Charge portière ouvert |
| `CP_chargeDoorOpenUI` | — | Port de charge — Charge portière ouvert interface |
| `CP_coldWeatherMode` | — | Mode temps froid (protection de charge par basses températures). Valeurs : CP_COLD_WEATHER_NONE (0), CP_COLD_WEATHER_LATCH_MITIGATION (1) |
| `CP_coverClosed` | — | Port de charge — Couvercle fermé |
| `CP_doorButtonPressed` | — | Port de charge — Portière bouton appuyé |
| `CP_doorControlState` | — | Port de charge — Portière contrôle état. Valeurs : CP_doorInit (0), CP_doorIdle (1), CP_doorOpenRequested (2), CP_doorOpening (3), CP_doorSenseOpen (4), CP_doorClosing (5), CP_doorSenseClosed (6) |
| `CP_doorOpenRequested` | — | Port de charge — Portière ouvert demandé |
| `CP_faultLineSensed` | — | Port de charge — Défaut ligne sensed. Valeurs : FAULT_LINE_CLEARED (0), FAULT_LINE_SET (1) |
| `CP_hvInletExposed` | — | Port de charge — Haute tension entrée exposed |
| `CP_inductiveDoorState` | — | État de la trappe de charge inductive. Valeurs : CP_INDUCTIVE_DOOR_INIT (0), CP_INDUCTIVE_DOOR_INIT_FROM_CHARGE (1), CP_INDUCTIVE_DOOR_INIT_FROM_DRIVE (2), CP_INDUCTIVE_DOOR_PRESENT (3), CP_INDUCTIVE_DOOR_NOT_PRESENT (4), CP_INDUCTIVE_DOOR_OFF_DRIVE (5), CP_INDUCTIVE_DOOR_OFF_CHARGE (6), CP_INDUCTIVE_DOOR_FAULT (7) |
| `CP_inductiveSensorState` | — | Port de charge — Inductive capteur état. Valeurs : CP_INDUCTIVE_SENSOR_INIT (0), CP_INDUCTIVE_SENSOR_POLL (1), CP_INDUCTIVE_SENSOR_SHUTDOWN (2), CP_INDUCTIVE_SENSOR_PAUSE (3), CP_INDUCTIVE_SENSOR_WAIT_FOR_INIT (4), CP_INDUCTIVE_SENSOR_FAULT (5), CP_INDUCTIVE_SENSOR_RESET (6), CP_INDUCTIVE_SENSOR_CONFIG (7) |
| `CP_insertEnableLine` | — | Activation de la ligne d'insertion du chargeur |
| `CP_latch2ControlState` | — | Port de charge — Latch2control état. Valeurs : CP_latchInit (0), CP_latchIdle (1), CP_latchDisengageRequested (2), CP_latchDisengaging (3), CP_latchDisengaged (4), CP_latchEngaging (5) |
| `CP_latch2State` | — | Port de charge — Latch2state. Valeurs : CP_LATCH_SNA (0), CP_LATCH_DISENGAGED (1), CP_LATCH_ENGAGED (2), CP_LATCH_BLOCKING (3) |
| `CP_latchControlState` | — | Port de charge — Loquet contrôle état. Valeurs : CP_latchInit (0), CP_latchIdle (1), CP_latchDisengageRequested (2), CP_latchDisengaging (3), CP_latchDisengaged (4), CP_latchEngaging (5) |
| `CP_latchEngaged` | — | Port de charge — Loquet engagé |
| `CP_latchState` | — | Port de charge — Loquet état. Valeurs : CP_LATCH_SNA (0), CP_LATCH_DISENGAGED (1), CP_LATCH_ENGAGED (2), CP_LATCH_BLOCKING (3) |
| `CP_ledColor` | — | Port de charge — LED couleur. Valeurs possibles : CP_LEDS_OFF (0), CP_LEDS_RED (1), CP_LEDS_GREEN (2), CP_LEDS_BLUE (3), CP_LEDS_WHITE (4), CP_LEDS_FLASHING_GREEN (5), CP_LEDS_FLASHING_AMBER (6), CP_LEDS_AMBER (7), CP_LEDS_RAVE (8), CP_LEDS_DEBUG (9), CP_LEDS_FLASHING_BLUE (10) |
| `CP_numAlertsSet` | — | Nombre d'alertes actives du port de charge |
| `CP_permanentPowerRequest` | — | Port de charge — Permanent puissance demande |
| `CP_swcanRelayClosed` | — | Port de charge — Swcan relais fermé |
| `CP_type` | — | Port de charge — Type. Valeurs : CP_TYPE_US_TESLA (0), CP_TYPE_EURO_IEC (1), CP_TYPE_GB (2), CP_TYPE_IEC_CCS (3) |
| `CP_vehicleUnlockRequest` | — | Demande de déverrouillage du véhicule (port de charge) |

### 0x261 — _12vBattStatus

**Batterie basse tension 12V : tension, courant, température, capacité** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCFRONT_12VBatteryStatusChecksum` | — | Contrôleur avant — 12V batterie état somme de contrôle |
| `VCFRONT_12VBatteryStatusCounter` | — | Contrôleur avant — 12V batterie état compteur |
| `VCFRONT_LVLoadRequest` | — | Demande de charge de l'alimentation basse tension (12 V) |
| `VCFRONT_good12VforUpdate` | — | Tension 12 V suffisante pour une mise à jour |

### 0x264 — hargeLineStatus

**État de la ligne de charge : tension, courant, phases, connexion** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `ChargeLinePower264` | kW | Charge ligne puissance |
| `ChargeLineCurrentLimit264` | A | Charge ligne courant limite |
| `ChargeLineVoltage264` | V | Charge ligne tension |
| `ChargeLineCurrent264` | A | Charge ligne courant |

### 0x266 — RearInverterPower

**Puissance de l'onduleur arrière : puissance mécanique et courant du bus haute tension** | Cycle : 10 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `RearHeatPowerMax266` | kW | Arrière chauffage puissance maximum |
| `RearPowerLimit266` | kW | Arrière puissance limite |
| `RearHeatPower266` | kW | Arrière chauffage puissance |
| `RearHeatPowerOptimal266` | kW | Arrière chauffage puissance optimal266 |
| `RearExcessHeatCmd` | kW | Commande de chauffage excessif — circuit arrière |
| `RearPower266` | kW | Puissance mécanique du moteur arrière |

### 0x267 — I_vehicleEstimates

**Estimations du véhicule par l'onduleur : pente de la route, résistance au roulement**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DI_gradeEst` | % | Estimation de la pente de la route |
| `DI_gradeEstInternal` | % | Onduleur — Pente estimé interne |
| `DI_mass` | kg | Onduleur — Masse |
| `DI_massConfidence` | — | Onduleur — Masse confiance. Valeurs : MASS_NOT_CONFIDED (0), MASS_CONFIDED (1) |
| `DI_relativeTireTreadDepth` | mm | Onduleur — Relative pneu tread depth. Valeurs : SNA (32) |
| `DI_steeringAngleOffset` | Deg | Onduleur — Direction angle décalage |
| `DI_tireFitment` | — | Onduleur — Pneu fitment. Valeurs : FITMENT_SQUARE (0), FITMENT_STAGGERED (1), FITMENT_SNA (3) |
| `DI_trailerDetected` | — | Onduleur — Trailer détecté. Valeurs : TRAILER_NOT_DETECTED (0), TRAILER_DETECTED (1) |
| `DI_vehicleEstimatesChecksum` | — | Onduleur — Véhicule estimates somme de contrôle |
| `DI_vehicleEstimatesCounter` | — | Onduleur — Véhicule estimates compteur |

### 0x268 — SystemPower

**Puissance du système : consommation totale, distribution entre sous-systèmes** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `SystemRegenPowerMax268` | kW | Système récupération puissance maximum |
| `SystemHeatPowerMax268` | kW | Système chauffage puissance maximum |
| `SystemHeatPower268` | kW | Système chauffage puissance |
| `SystemDrivePowerMax268` | kW | Système traction puissance maximum |
| `DI_primaryUnitSiliconType` | — | Type de semi-conducteur de l'onduleur principal (MOSFET ou IGBT). Valeurs : MOSFET (0), IGBT (1) |

### 0x269 — LeftRearPower

**Puissance de l'onduleur arrière gauche**

| Signal | Unité | Description |
|--------|-------|-------------|
| `LeftRearHeatPowerOptimal` | kW | Gauche arrière chauffage puissance optimal |
| `LeftRearPower` | kW | Gauche arrière puissance |
| `LeftRearPowerMax` | kW | Gauche arrière puissance maximum |
| `LeftRearPowerLimit` | kW | Gauche arrière puissance limite |
| `LeftRearHeatPower` | kW | Gauche arrière chauffage puissance |
| `LeftRearExcessHeatCmd` | kW | Commande de chauffage excessif — circuit arrière gauche |

### 0x273 — UI_vehicleControl

**Commandes véhicule depuis l'interface : mode sport, freinage régénératif, suspension**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_accessoryPowerRequest` | — | Interface — Accessoire puissance demande |
| `UI_alarmEnabled` | — | Alarme antivol activée |
| `UI_ambientLightingEnabled` | — | Éclairage d'ambiance activé |
| `UI_autoFoldMirrorsOn` | — | Interface — Automatique pliage rétroviseurs activé |
| `UI_autoHighBeamEnabled` | — | Interface — Automatique haut faisceau activé |
| `UI_childDoorLockOn` | — | Sécurité enfants (verrouillage des portières arrière) activée |
| `UI_displayBrightnessLevel` | % | Interface — Écran luminosité niveau. Valeurs : SNA (255) |
| `UI_domeLightSwitch` | — | Interface — Plafonnier feu interrupteur. Valeurs : DOME_LIGHT_SWITCH_OFF (0), DOME_LIGHT_SWITCH_ON (1), DOME_LIGHT_SWITCH_AUTO (2) |
| `UI_driveStateRequest` | — | Interface — Traction état demande. Valeurs : DRIVE_STATE_REQ_IDLE (0), DRIVE_STATE_REQ_START (1) |
| `UI_frontFogSwitch` | — | Interface — Avant antibrouillard interrupteur |
| `UI_frontLeftSeatHeatReq` | — | Demande de chauffage du siège avant gauche. Valeurs : HEATER_REQUEST_OFF (0), HEATER_REQUEST_LEVEL1 (1), HEATER_REQUEST_LEVEL2 (2), HEATER_REQUEST_LEVEL3 (3) |
| `UI_frontRightSeatHeatReq` | — | Demande de chauffage du siège avant droit. Valeurs : HEATER_REQUEST_OFF (0), HEATER_REQUEST_LEVEL1 (1), HEATER_REQUEST_LEVEL2 (2), HEATER_REQUEST_LEVEL3 (3) |
| `UI_frunkRequest` | — | Interface — Coffre avant (frunk) demande |
| `UI_globalUnlockOn` | — | Déverrouillage global activé |
| `UI_honkHorn` | — | Klaxonner (appel de klaxon) |
| `UI_intrusionSensorOn` | — | Capteur d'intrusion (alarme) activé |
| `UI_lockRequest` | — | Interface — Verrouillage demande. Valeurs : UI_LOCK_REQUEST_IDLE (0), UI_LOCK_REQUEST_LOCK (1), UI_LOCK_REQUEST_UNLOCK (2), UI_LOCK_REQUEST_REMOTE_UNLOCK (3), UI_LOCK_REQUEST_REMOTE_LOCK (4), UI_LOCK_REQUEST_SNA (7) |
| `UI_mirrorDipOnReverse` | — | Interface — Rétroviseur code (croisement) activé marche arrière |
| `UI_mirrorFoldRequest` | — | Demande de repliage des rétroviseurs. Valeurs : MIRROR_FOLD_REQUEST_IDLE (0), MIRROR_FOLD_REQUEST_RETRACT (1), MIRROR_FOLD_REQUEST_PRESENT (2), MIRROR_FOLD_REQUEST_SNA (3) |
| `UI_mirrorHeatRequest` | — | Interface — Rétroviseur chauffage demande |
| `UI_powerOff` | — | Interface — Puissance désactivé |
| `UI_rearCenterSeatHeatReq` | — | Demande de chauffage du siège arrière central. Valeurs : HEATER_REQUEST_OFF (0), HEATER_REQUEST_LEVEL1 (1), HEATER_REQUEST_LEVEL2 (2), HEATER_REQUEST_LEVEL3 (3) |
| `UI_rearFogSwitch` | — | Interface — Arrière antibrouillard interrupteur |
| `UI_rearLeftSeatHeatReq` | — | Demande de chauffage du siège arrière gauche. Valeurs : HEATER_REQUEST_OFF (0), HEATER_REQUEST_LEVEL1 (1), HEATER_REQUEST_LEVEL2 (2), HEATER_REQUEST_LEVEL3 (3) |
| `UI_rearRightSeatHeatReq` | — | Demande de chauffage du siège arrière droit. Valeurs : HEATER_REQUEST_OFF (0), HEATER_REQUEST_LEVEL1 (1), HEATER_REQUEST_LEVEL2 (2), HEATER_REQUEST_LEVEL3 (3) |
| `UI_rearWindowLockout` | — | Verrouillage des vitres arrière (sécurité enfants) |
| `UI_remoteClosureRequest` | — | Demande de fermeture à distance. Valeurs : UI_REMOTE_CLOSURE_REQUEST_IDLE (0), UI_REMOTE_CLOSURE_REQUEST_REAR_TRUNK_MOVE (1), UI_REMOTE_CLOSURE_REQUEST_FRONT_TRUNK_MOVE (2), UI_REMOTE_CLOSURE_REQUEST_SNA (3) |
| `UI_remoteStartRequest` | — | Demande de démarrage à distance. Valeurs : UI_REMOTE_START_REQUEST_IDLE (0), UI_REMOTE_START_REQUEST_START (1), UI_REMOTE_START_REQUEST_SNA (4) |
| `UI_seeYouHomeLightingOn` | — | Éclairage d'accompagnement « See You Home » activé |
| `UI_steeringBacklightEnabled` | — | Interface — Direction rétro-éclairage activé. Valeurs : STEERING_BACKLIGHT_DISABLED (0), STEERING_BACKLIGHT_ENABLED (1) |
| `UI_steeringButtonMode` | — | Interface — Direction bouton mode. Valeurs : STEERING_BUTTON_MODE_OFF (0), STEERING_BUTTON_MODE_STEERING_COLUMN_ADJ (1), STEERING_BUTTON_MODE_MIRROR_LEFT (2), STEERING_BUTTON_MODE_MIRROR_RIGHT (3), STEERING_BUTTON_MODE_HEADLIGHT_LEFT (4), STEERING_BUTTON_MODE_HEADLIGHT_RIGHT (5) |
| `UI_stop12vSupport` | — | Interface — Stop12v support |
| `UI_summonActive` | — | Interface — Convocation (Summon) actif |
| `UI_unlockOnPark` | — | Déverrouillage automatique au passage en stationnement |
| `UI_walkAwayLock` | — | Verrouillage automatique en s'éloignant du véhicule |
| `UI_walkUpUnlock` | — | Déverrouillage automatique en s'approchant du véhicule |
| `UI_wiperMode` | — | Interface — Essuie-glace mode. Valeurs : WIPER_MODE_SNA (0), WIPER_MODE_SERVICE (1), WIPER_MODE_NORMAL (2), WIPER_MODE_PARK (3) |
| `UI_wiperRequest` | — | Interface — Essuie-glace demande. Valeurs : WIPER_REQUEST_SNA (0), WIPER_REQUEST_OFF (1), WIPER_REQUEST_AUTO (2), WIPER_REQUEST_SLOW_INTERMITTENT (3), WIPER_REQUEST_FAST_INTERMITTENT (4), WIPER_REQUEST_SLOW_CONTINUOUS (5), WIPER_REQUEST_FAST_CONTINUOUS (6) |

### 0x27C — RightRearPower

**Puissance de l'onduleur arrière droit**

| Signal | Unité | Description |
|--------|-------|-------------|
| `RightRearHeatPowerOptimal` | kW | Droite arrière chauffage puissance optimal |
| `RightRearPower` | kW | Droite arrière puissance |
| `RightRearHeatPowerMax` | kW | Droite arrière chauffage puissance maximum |
| `RightRearPowerLimit` | kW | Droite arrière puissance limite |
| `RightRearHeatPower` | kW | Droite arrière chauffage puissance |
| `RightRearExcessHeatCmd` | kW | Commande de chauffage excessif — circuit arrière droit |

### 0x27D — P_dcChargeLimits

**Limites de charge rapide (DC) : courant et tension maximaux demandés**

| Signal | Unité | Description |
|--------|-------|-------------|
| `CP_evseInstantDcCurrentLimit` | A | Port de charge — Borne de charge (EVSE) instant courant continu (DC) courant limite |
| `CP_evseMaxDcCurrentLimit` | A | Port de charge — Borne de charge (EVSE) maximum courant continu (DC) courant limite |
| `CP_evseMaxDcVoltageLimit` | V | Port de charge — Borne de charge (EVSE) maximum courant continu (DC) tension limite |
| `CP_evseMinDcCurrentLimit` | A | Port de charge — Borne de charge (EVSE) minimum courant continu (DC) courant limite |
| `CP_evseMinDcVoltageLimit` | V | Port de charge — Borne de charge (EVSE) minimum courant continu (DC) tension limite |

### 0x281 — VCFRONT_CMPRequest

**Requêtes du contrôleur avant vers le compresseur de climatisation**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCFRONT_CMPTargetDuty` | % | Contrôleur avant — CMP cible rapport cyclique |
| `VCFRONT_CMPPowerLimit` | W | Contrôleur avant — CMP puissance limite |
| `VCFRONT_CMPReset` | — | Contrôleur avant — CMP réinitialisation |
| `VCFRONT_CMPEnable` | — | Contrôleur avant — CMP activation |

### 0x282 — VCLEFT_hvacBlowerFeedback

**Retour d'information de la soufflerie de climatisation côté gauche**

_Aucun signal défini._

### 0x284 — UIvehicleModes

**Modes du véhicule depuis l'interface : conduite, confort, économie, sport**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UIfactoryMode284` | — | U ifactory mode |
| `UIhomelinkV2Command0284` | — | U ihomelink v2command0284 |
| `UIhomelinkV2Command1284` | — | U ihomelink v2command1284 |
| `UIhomelinkV2Command2284` | — | U ihomelink v2command2284 |
| `UIserviceMode284` | — | U iservice mode |
| `UIshowroomMode284` | — | U ishowroom mode |
| `UItransportMode284` | — | U itransport mode |
| `UIgameMode284` | — | U igame mode |
| `UIisDelivered284` | — | U iis delivered284 |
| `UIcarWashModeRequest284` | — | Demande d'activation du mode lavage automatique |
| `UIvaletMode284` | — | U ivalet mode |
| `UIsentryMode284` | — | U isentry mode |

### 0x287 — PTCcabinHeatSensorStatus

**État des capteurs de chauffage PTC de l'habitacle : température, courant, résistance**

| Signal | Unité | Description |
|--------|-------|-------------|
| `PTC_leftCurrentHV` | A | PTC — Gauche courant haute tension |
| `PTC_leftTempIGBT` | C | PTC — Gauche température IGBT |
| `PTC_rightCurrentHV` | A | PTC — Droite courant haute tension |
| `PTC_rightTempIGBT` | C | PTC — Droite température IGBT |
| `PTC_tempOCP` | C | PTC — Température OCP |
| `PTC_tempPCB` | C | PTC — Température carte électronique |
| `PTC_voltageHV` | V | PTC — Tension haute tension |

### 0x288 — PBleftStatus

**État du frein de stationnement électrique (EPB) côté gauche**

| Signal | Unité | Description |
|--------|-------|-------------|
| `EPBL12VFilt288` | V | Epbl12v filt288 |
| `EPBLcsmFaultReason288` | — | Frein de stationnement électrique (EPB) lcsm défaut raison |
| `EPBLdisconnected288` | — | Frein de stationnement électrique (EPB) ldisconnected288 |
| `EPBLCDPQualified288` | — | Epblcdp qualified288 |
| `EPBLesmCaliperRequest288` | — | Frein de stationnement électrique (EPB) lesm étrier de frein demande. Valeurs : idle (1), engaging (2), disengaging (3) |
| `EPBLesmOperationTrigger288` | — | Frein de stationnement électrique (EPB) lesm operation déclenchement. Valeurs : ParkEngaged (1), Released (6), SuperPark (22) |
| `EPBLinternalCDPRequest288` | — | Demande CDP interne du frein de stationnement gauche |
| `EPBLinternalStatusChecksum288` | — | Somme de contrôle de l'état interne du frein de stationnement gauche |
| `EPBLinternalStatusCounter288` | — | Compteur de séquence de l'état interne du frein de stationnement gauche |
| `EPBLlocalServiceModeActive288` | — | Frein de stationnement électrique (EPB) llocal service mode actif |
| `EPBLlockoutUnlockCount288` | — | Compteur de déverrouillage/verrouillage du frein de stationnement gauche |
| `EPBLsummonFaultReason288` | — | Raison de défaut du frein de stationnement gauche lors de la convocation (Summon) |
| `EPBLsummonState288` | — | État du frein de stationnement gauche lors de la convocation (Summon) |
| `EPBLunitFaultStatus288` | — | État de défaut de l'unité du frein de stationnement gauche |
| `EPBLunitStatus288` | — | État de l'unité du frein de stationnement gauche. Valeurs : DriveReleased (1), ParkEngaged (3), Engaging (8), Disengaging (10) |

### 0x292 — MS_SOC

**État de charge de la batterie (SoC) : minimum, maximum, moyen, affiché (avec tampon)** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `BattBeginningOfLifeEnergy292` | kWh | Énergie initiale de la batterie en début de vie. Valeurs : SNA (1023) |
| `SOCmax292` | % | SO cmax292 |
| `SOCave292` | % | SO cave292 |
| `SOCUI292` | % | État de charge affiché (inclut une réserve de ~5 %) |
| `SOCmin292` | % | SO cmin292 |
| `BMS_battTempPct` | % | Pourcentage de température batterie par rapport aux limites. Valeurs : SNA (255) |

### 0x293 — UI_chassisControl

**Commandes du châssis depuis l'interface : suspension, direction, stabilité** | Cycle : 500 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_accOvertakeEnable` | — | Activation du dépassement automatique (régulateur adaptatif). Valeurs : ACC_OVERTAKE_OFF (0), ACC_OVERTAKE_ON (1), SNA (3) |
| `UI_aebEnable` | — | Interface — Aeb activation. Valeurs : AEB_OFF (0), AEB_ON (1), SNA (3) |
| `UI_aesEnable` | — | Interface — Aes activation. Valeurs : AES_OFF (0), AES_ON (1), SNA (3) |
| `UI_ahlbEnable` | — | Interface — Ahlb activation. Valeurs : AHLB_OFF (0), AHLB_ON (1), SNA (3) |
| `UI_autoLaneChangeEnable` | — | Activation du changement de voie automatique. Valeurs : OFF (0), ON (1), SNA (3) |
| `UI_autoParkRequest` | — | Interface — Automatique stationnement demande. Valeurs possibles : NONE (0), PARK_LEFT_PARALLEL (1), PARK_LEFT_CROSS (2), PARK_RIGHT_PARALLEL (3), PARK_RIGHT_CROSS (4), PARALLEL_PULL_OUT_TO_LEFT (5), PARALLEL_PULL_OUT_TO_RIGHT (6), ABORT (7), COMPLETE (8), SEARCH (9), PAUSE (10), RESUME (11), SNA (15) |
| `UI_bsdEnable` | — | Interface — Bsd activation. Valeurs : BSD_OFF (0), BSD_ON (1), SNA (3) |
| `UI_chassisControlChecksum` | — | Somme de contrôle des commandes châssis |
| `UI_chassisControlCounter` | — | Compteur de séquence des commandes châssis |
| `UI_dasDebugEnable` | — | Interface — Conduite autonome débogage activation |
| `UI_distanceUnits` | — | Unité de distance (km ou miles). Valeurs : DISTANCEUNITS_KM (0), DISTANCEUNITS_MILES (1) |
| `UI_fcwEnable` | — | Interface — Fcw activation. Valeurs : FCW_OFF (0), FCW_ON (1), SNA (3) |
| `UI_fcwSensitivity` | — | Interface — Fcw sensitivity. Valeurs : AEB_SENSITIVITY_EARLY (0), AEB_SENSITIVITY_AVERAGE (1), AEB_SENSITIVITY_LATE (2), SNA (3) |
| `UI_latControlEnable` | — | Activation du contrôle latéral (direction assistée). Valeurs : LATERAL_CONTROL_OFF (0), LATERAL_CONTROL_ON (1), LATERAL_CONTROL_UNAVAILABLE (2), LATERAL_CONTROL_SNA (3) |
| `UI_ldwEnable` | — | Activation de l'avertissement de sortie de voie (LDW). Valeurs : NO_HAPTIC (0), LDW_TRIGGERS_HAPTIC (1), SNA (3) |
| `UI_narrowGarages` | — | Mode garages étroits (rétroviseurs repliés) |
| `UI_parkBrakeRequest` | — | Demande d'activation du frein de stationnement. Valeurs : PARK_BRAKE_REQUEST_IDLE (0), PARK_BRAKE_REQUEST_PRESSED (1), PARK_BRAKE_REQUEST_SNA (3) |
| `UI_pedalSafetyEnable` | — | Interface — Pédale safety activation. Valeurs : PEDAL_SAFETY_OFF (0), PEDAL_SAFETY_ON (1), SNA (3) |
| `UI_rebootAutopilot` | — | Interface — Reboot Autopilot |
| `UI_redLightStopSignEnable` | — | Activation de la détection des feux rouges et panneaux stop. Valeurs : RLSSW_OFF (0), RLSSW_ON (1), SNA (3) |
| `UI_selfParkTune` | — | Réglage fin du stationnement automatique. Valeurs : SNA (15) |
| `UI_steeringTuneRequest` | — | Interface — Direction tune demande. Valeurs : STEERING_TUNE_COMFORT (0), STEERING_TUNE_STANDARD (1), STEERING_TUNE_SPORT (2) |
| `UI_tractionControlMode` | — | Interface — Traction contrôle mode. Valeurs : TC_NORMAL_SELECTED (0), TC_SLIP_START_SELECTED (1), TC_DEV_MODE_1_SELECTED (2), TC_DEV_MODE_2_SELECTED (3), TC_ROLLS_MODE_SELECTED (4), TC_DYNO_MODE_SELECTED (5), TC_OFFROAD_ASSIST_SELECTED (6) |
| `UI_trailerMode` | — | Interface — Trailer mode. Valeurs : TRAILER_MODE_OFF (0), TRAILER_MODE_ON (1) |
| `UI_winchModeRequest` | — | Interface — Winch mode demande. Valeurs : WINCH_MODE_IDLE (0), WINCH_MODE_ENTER (1), WINCH_MODE_EXIT (2) |
| `UI_zeroSpeedConfirmed` | — | Interface — Zero vitesse confirmé. Valeurs : ZERO_SPEED_CANCELED (0), ZERO_SPEED_CONFIRMED (1), ZERO_SPEED_UNUSED (2), ZERO_SPEED_SNA (3) |

### 0x29D — P_dcChargeStatus

**État de la charge rapide (DC) : puissance, courant, tension, progression**

| Signal | Unité | Description |
|--------|-------|-------------|
| `CP_evseOutputDcCurrent` | A | Port de charge — Borne de charge (EVSE) sortie courant continu (DC) courant |
| `CP_evseOutputDcCurrentStale` | — | Port de charge — Borne de charge (EVSE) sortie courant continu (DC) courant stale |
| `CP_evseOutputDcVoltage` | V | Port de charge — Borne de charge (EVSE) sortie courant continu (DC) tension |

### 0x2A8 — MPD_state

**État du compresseur de climatisation (variante D) : vitesse, puissance, diagnostic**

| Signal | Unité | Description |
|--------|-------|-------------|
| `CMPD_inputHVCurrent` | A | Compresseur (D) — Entrée haute tension courant |
| `CMPD_inputHVPower` | W | Compresseur (D) — Entrée haute tension puissance |
| `CMPD_inputHVVoltage` | V | Compresseur (D) — Entrée haute tension tension |
| `CMPD_powerLimitActive` | — | Compresseur (D) — Puissance limite actif |
| `CMPD_powerLimitTooLowToStart` | — | Puissance disponible trop faible pour démarrer le compresseur |
| `CMPD_ready` | — | Compresseur (D) — Prêt |
| `CMPD_speedDuty` | % | Compresseur (D) — Vitesse rapport cyclique |
| `CMPD_speedRPM` | RPM | Compresseur (D) — Vitesse tours/min |
| `CMPD_state` | — | Compresseur (D) — État. Valeurs : CMPD_STATE_INIT (0), CMPD_STATE_RUNNING (1), CMPD_STATE_STANDBY (2), CMPD_STATE_FAULT (3), CMPD_STATE_IDLE (4), CMPD_STATE_SNA (15) |
| `CMPD_wasteHeatState` | — | État de la dissipation de chaleur résiduelle (compresseur D). Valeurs : CMPD_WASTE_HEAT_STATE_OFF (0), CMPD_WASTE_HEAT_STATE_ACTIVE (1), CMPD_WASTE_HEAT_STATE_NOT_AVAILABLE (2), CMPD_WASTE_HEAT_STATE_UNUSED (3) |

### 0x2B4 — PCS_dcdcRailStatus

**État des rails d'alimentation du convertisseur DC-DC (PCS)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `PCS_dcdcHvBusVolt` | V | Tension du bus HT au convertisseur DC-DC |
| `PCS_dcdcLvBusVolt` | V | Tension du bus 12 V au convertisseur DC-DC |
| `PCS_dcdcLvOutputCurrent` | A | Convertisseur (PCS) — Dcdc basse tension sortie courant |

### 0x2B6 — I_chassisControlStatus

**Retour d'état des commandes châssis par l'onduleur de traction**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DI_btcStateUI` | — | État du chauffage de batterie (BTC) affiché à l'interface |
| `DI_ptcStateUI` | — | État de la résistance chauffante PTC affiché à l'interface. Valeurs : FAULTED (0), BACKUP (1), ON (2), SNA (3) |
| `DI_tcTelltaleFlash` | — | Clignotement du voyant antipatinage |
| `DI_tcTelltaleOn` | — | Voyant antipatinage allumé |
| `DI_tractionControlModeUI` | — | Onduleur — Traction contrôle mode interface. Valeurs : NORMAL (0), SLIP_START (1), DEV_MODE_1 (2), DEV_MODE_2 (3), ROLLS_MODE (4), DYNO_MODE (5), OFFROAD_ASSIST (6) |
| `DI_vdcTelltaleFlash` | — | Clignotement du voyant de contrôle de stabilité |
| `DI_vdcTelltaleOn` | — | Voyant de contrôle de stabilité allumé |
| `DI_vehicleHoldTelltaleOn` | — | Voyant de maintien du véhicule à l'arrêt allumé |

### 0x2BD — P_dcPowerLimits

**Limites de puissance de la charge rapide (DC)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `CP_evseInstantDcPowerLimit` | kW | Port de charge — Borne de charge (EVSE) instant courant continu (DC) puissance limite |
| `CP_evseMaxDcPowerLimit` | kW | Port de charge — Borne de charge (EVSE) maximum courant continu (DC) puissance limite |

### 0x2D2 — MSVAlimits

**Limites de tension de la batterie (BMS) : tensions de cellule min/max** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `MinVoltage2D2` | V | Minimum voltage2d2 |
| `MaxDischargeCurrent2D2` | A | Maximum décharge current2d2 |
| `MaxChargeCurrent2D2` | A | Maximum charge current2d2 |
| `MaxVoltage2D2` | V | Maximum voltage2d2 |

### 0x2E1 — VCFRONT_status

**État du contrôleur avant : mode de fonctionnement, erreurs, diagnostics**

_Aucun signal défini._

### 0x2E5 — rontInverterPower

**Puissance de l'onduleur avant : puissance mécanique et courant du bus haute tension** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `FrontHeatPowerOptimal2E5` | kW | Avant chauffage puissance optimal2e5 |
| `FrontPower2E5` | kW | Puissance mécanique du moteur avant |
| `FrontHeatPowerMax2E5` | kW | Avant chauffage puissance max2e5 |
| `FrontPowerLimit2E5` | kW | Avant puissance limit2e5 |
| `FrontHeatPower2E5` | kW | Avant chauffage power2e5 |
| `FrontExcessHeatCmd` | kW | Commande de chauffage excessif — circuit avant |

### 0x2E6 — PlaidFrontPower

**Puissance des onduleurs avant (version Plaid / tri-moteur)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `PFrontHeatPowerOptimal` | kW | P avant chauffage puissance optimal |
| `PFrontPower` | kW | P avant puissance |
| `PFrontHeatPowerMax` | kW | P avant chauffage puissance maximum |
| `PFrontPowerLimit` | kW | P avant puissance limite |
| `PFrontHeatPower` | kW | P avant chauffage puissance |
| `PFrontExcessHeatCmd` | kW | Commande de chauffage excessif — circuit P avant |

### 0x2F1 — VCFRONT_eFuseDebugStatus

**État de débogage des fusibles électroniques (eFuse) du contrôleur avant**

_Aucun signal défini._

### 0x2F3 — UI_hvacRequest

**Requêtes de climatisation depuis l'interface : température, ventilation, mode**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_hvacReqACDisable` | — | Demande de désactivation de la climatisation AC. Valeurs : AUTO (0), OFF (1), ON (2) |
| `UI_hvacReqAirDistributionMode` | — | Mode de distribution d'air demandé (climatisation). Valeurs : AUTO (0), MANUAL_FLOOR (1), MANUAL_PANEL (2), MANUAL_PANEL_FLOOR (3), MANUAL_DEFROST (4), MANUAL_DEFROST_FLOOR (5), MANUAL_DEFROST_PANEL (6), MANUAL_DEFROST_PANEL_FLOOR (7) |
| `UI_hvacReqBlowerSegment` | — | Segment de puissance de la soufflerie demandé. Valeurs possibles : OFF (0), 1 (1), 2 (2), 3 (3), 4 (4), 5 (5), 6 (6), 7 (7), 8 (8), 9 (9), 10 (10), AUTO (11) |
| `UI_hvacReqKeepClimateOn` | — | Demande de maintien de la climatisation active (mode chien, fête…). Valeurs : KEEP_CLIMATE_ON_REQ_OFF (0), KEEP_CLIMATE_ON_REQ_ON (1), KEEP_CLIMATE_ON_REQ_DOG (2), KEEP_CLIMATE_ON_REQ_PARTY (3) |
| `UI_hvacReqManualDefogState` | — | Demande manuelle de désembuage / dégivrage. Valeurs : NONE (0), DEFOG (1), DEFROST (2) |
| `UI_hvacReqRecirc` | — | Demande de recirculation d'air (climatisation). Valeurs : AUTO (0), RECIRC (1), FRESH (2) |
| `UI_hvacReqSecondRowState` | — | État demandé de la climatisation deuxième rangée. Valeurs : AUTO (0), OFF (1), LOW (2), MED (3), HIGH (4) |
| `UI_hvacReqTempSetpointLeft` | C | Interface — Climatisation req température setpoint gauche. Valeurs : LO (0), HI (26) |
| `UI_hvacReqTempSetpointRight` | C | Interface — Climatisation req température setpoint droite. Valeurs : LO (0), HI (26) |
| `UI_hvacReqUserPowerState` | — | État de puissance de la climatisation demandé par l'utilisateur. Valeurs : OFF (0), ON (1), PRECONDITION (2), OVERHEAT_PROTECT_FANONLY (3), OVERHEAT_PROTECT (4) |
| `UI_hvacUseModeledDuctTemp` | — | Utilisation de la température de conduit d'air modélisée pour la climatisation |

### 0x300 — MS_info

**Informations du BMS : identifiants, version logicielle, configuration**

_Aucun signal défini._

### 0x301 — VCFRONT_info

**Informations du contrôleur avant : identifiants, version logicielle**

_Aucun signal défini._

### 0x309 — S_object

**Objets détectés par le système de conduite autonome : position, type, vitesse relative**

_Aucun signal défini._

### 0x312 — MSthermal

**Températures détaillées de la batterie : cellules (briques), barres de connexion, liquide de refroidissement** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `BMSdissipation312` | kW | BM sdissipation312 |
| `BMSflowRequest312` | LPM | BM sflow demande |
| `BMSinletActiveCoolTarget312` | C | BM sinlet actif refroidissement cible |
| `BMSinletActiveHeatTarget312` | C | BM sinlet actif chauffage cible |
| `BMSinletPassiveTarget312` | C | BM sinlet passif cible |
| `BMSnoFlowRequest312` | — | BM sno débit demande |
| `BMSpcsNoFlowRequest312` | — | BM spcs no débit demande |
| `BMSmaxPackTemperature` | C | BM smax pack batterie température |
| `BMSminPackTemperature` | C | BM smin pack batterie température |

### 0x313 — UI_trackModeSettings

**Réglages du mode circuit (Track Mode) : stabilité, régénération, répartition de puissance**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_trackCmpOverclock` | — | Interface — Circuit cmp overclock. Valeurs : TRACK_MODE_CMP_OVERCLOCK_OFF (0), TRACK_MODE_CMP_OVERCLOCK_ON (1) |
| `UI_trackModeRequest` | — | Interface — Circuit mode demande. Valeurs : TRACK_MODE_REQUEST_IDLE (0), TRACK_MODE_REQUEST_ON (1), TRACK_MODE_REQUEST_OFF (2) |
| `UI_trackModeSettingsChecksum` | — | Somme de contrôle des réglages du mode circuit |
| `UI_trackModeSettingsCounter` | — | Compteur de séquence des réglages du mode circuit |
| `UI_trackPostCooling` | — | Refroidissement post-session sur circuit. Valeurs : TRACK_MODE_POST_COOLING_OFF (0), TRACK_MODE_POST_COOLING_ON (1) |
| `UI_trackRotationTendency` | % | Interface — Circuit rotation tendency |
| `UI_trackStabilityAssist` | % | Interface — Circuit stability assistance |

### 0x315 — RearInverterTemps

**Températures de l'onduleur arrière : stator, rotor, huile, transistors, carte de commande**

| Signal | Unité | Description |
|--------|-------|-------------|
| `RearTempInvPCB315` | C | Arrière température inv carte électronique |
| `RearTempPctStator315` | % | Température du stator arrière (% de la limite) |
| `RearTempPctInverter315` | % | Température de l'onduleur arrière (% de la limite) |
| `RearTempInvCapbank315` | C | Arrière température inv banc de condensateurs |
| `RearTempStator315` | C | Température du stator arrière |
| `RearTempInvHeatsink315` | C | Arrière température inv heatsink315 |
| `RearTempInverter315` | C | Arrière température onduleur |
| `DIR_inverterTQF` | — | Onduleur arrière — Onduleur TQF. Valeurs : INVERTERT_INIT (0), INVERTERT_IRRATIONAL (1), INVERTERT_RATIONAL (2), INVERTERT_UNKNOWN (3) |

### 0x318 — SystemTimeUTC

**Heure système UTC : année, mois, jour, heure, minute, seconde**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UTCyear318` | yr | Année (UTC) |
| `UTCmonth318` | mo | Mois (UTC) |
| `UTCseconds318` | sec | Seconde (UTC) |
| `UTChour318` | hr | UT chour318 |
| `UTCday318` | dy | Jour (UTC) |
| `UTCminutes318` | min | Minute (UTC) |

### 0x31C — _chgStatus

**État du câble de charge (CC) : détection, connexion, communication**

| Signal | Unité | Description |
|--------|-------|-------------|
| `CC_currentLimit` | A | Câble de charge — Courant limite. Valeurs : SNA (255) |
| `CC_deltaTransformer` | — | Câble de charge — Variation transformer |
| `CC_gridGrounding` | — | Mise à la terre du réseau électrique (câble de charge). Valeurs : CC_GRID_GROUNDING_TN_TT (0), CC_GRID_GROUNDING_IT_SplitPhase (1), CC_GRID_GROUNDING_SNA (2) |
| `CC_line1Voltage` | V | Câble de charge — Line1voltage. Valeurs : SNA (511) |
| `CC_line2Voltage` | V | Câble de charge — Line2voltage. Valeurs : SNA (511) |
| `CC_line3Voltage` | V | Câble de charge — Line3voltage. Valeurs : SNA (511) |
| `CC_numPhases` | — | Câble de charge — Numéro phases. Valeurs : SNA (0) |
| `CC_numVehCharging` | — | Câble de charge — Numéro véhicule en charge |
| `CC_pilotState` | — | Câble de charge — Pilote (signal CP) état. Valeurs : CC_PILOT_STATE_READY (0), CC_PILOT_STATE_IDLE (1), CC_PILOT_STATE_FAULTED (2), CC_PILOT_STATE_SNA (3) |

### 0x321 — VCFRONT_sensors

**Capteurs du contrôleur avant : températures de liquide de refroidissement, débits, pression** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCFRONT_battSensorIrrational` | — | Contrôleur avant — Batterie capteur irrational |
| `VCFRONT_brakeFluidLevel` | — | Niveau de liquide de frein. Valeurs : SNA (0), LOW (1), NORMAL (2) |
| `VCFRONT_coolantLevel` | — | Contrôleur avant — Liquide de refroidissement niveau. Valeurs : NOT_OK (0), FILLED (1) |
| `VCFRONT_ptSensorIrrational` | — | Contrôleur avant — Pt capteur irrational |
| `VCFRONT_tempAmbient` | C | Contrôleur avant — Température ambiante. Valeurs : SNA (0) |
| `VCFRONT_tempAmbientFiltered` | C | Contrôleur avant — Température ambiante filtré. Valeurs : SNA (0) |
| `VCFRONT_tempCoolantBatInlet` | C | Température du liquide de refroidissement à l'entrée de la batterie. Valeurs : SNA (1023) |
| `VCFRONT_tempCoolantPTInlet` | C | Température du liquide de refroidissement à l'entrée du moteur. Valeurs : SNA (2047) |
| `VCFRONT_washerFluidLevel` | — | Niveau de liquide lave-glace. Valeurs : SNA (0), LOW (1), NORMAL (2) |

### 0x332 — ttBrickMinMax

**Tensions minimale et maximale des cellules (briques) de la batterie**

_Aucun signal défini._

### 0x333 — UI_chargeRequest

**Demande de charge depuis l'interface : niveau cible, heure de départ programmée** | Cycle : 500 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_acChargeCurrentLimit` | A | Interface — Courant alternatif (AC) charge courant limite. Valeurs : SNA (127) |
| `UI_brickBalancingDisabled` | — | Interface — Cellule (brique) équilibrage désactivé. Valeurs : FALSE (0), TRUE (1) |
| `UI_brickVLoggingRequest` | — | Demande d'enregistrement des tensions de cellules (briques). Valeurs : FALSE (0), TRUE (1) |
| `UI_chargeEnableRequest` | — | Interface — Charge activation demande |
| `UI_chargeTerminationPct` | % | Interface — Charge termination pct |
| `UI_closeChargePortDoorRequest` | — | Demande de fermeture de la trappe du port de charge |
| `UI_openChargePortDoorRequest` | — | Interface — Ouvert charge port portière demande |
| `UI_scheduledDepartureEnabled` | — | Interface — Scheduled départ activé |
| `UI_smartAcChargingEnabled` | — | Charge AC intelligente (heures creuses) activée |
| `UI_cpInletHeaterRequest` | — | Interface — Cp entrée résistance de chauffage demande |
| `UI_socSnapshotExpirationTime` | weeks | Interface — État de charge snapshot expiration temps |

### 0x334 — UI_powertrainControl

**Commandes de la chaîne de traction : mode sport, vitesse max, activation du moteur**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_DIAppSliderDebug` | — | Interface — DI application slider débogage |
| `UI_limitMode` | — | Interface — Limite mode. Valeurs : LIMIT_NORMAL (0), LIMIT_VALET (1), LIMIT_FACTORY (2), LIMIT_SERVICE (3) |
| `UI_motorOnMode` | — | Interface — Moteur activé mode. Valeurs : MOTORONMODE_NORMAL (0), MOTORONMODE_FRONT_ONLY (1), MOTORONMODE_REAR_ONLY (2) |
| `UI_pedalMap` | — | Interface — Pédale carte. Valeurs : CHILL (0), SPORT (1), PERFORMANCE (2) |
| `UI_powertrainControlChecksum` | — | Interface — Chaîne de traction contrôle somme de contrôle |
| `UI_powertrainControlCounter` | — | Interface — Chaîne de traction contrôle compteur |
| `UI_regenTorqueMax` | % | Interface — Récupération couple maximum |
| `UI_speedLimit` | kph | Interface — Vitesse limite. Valeurs : SNA (255) |
| `UI_stoppingMode` | — | Interface — Stopping mode. Valeurs : STANDARD (0), CREEP (1), HOLD (2) |
| `UI_systemPowerLimit` | kW | Interface — Système puissance limite. Valeurs : SNA (31) |
| `UI_systemTorqueLimit` | Nm | Interface — Système couple limite. Valeurs : SNA (63) |
| `UI_wasteMode` | — | Mode de dissipation de chaleur résiduelle. Valeurs : WASTE_TYPE_NONE (0), WASTE_TYPE_PARTIAL (1), WASTE_TYPE_FULL (2) |
| `UI_wasteModeRegenLimit` | — | Limite de récupération en mode dissipation de chaleur. Valeurs : MAX_REGEN_CURRENT_MAX (0), MAX_REGEN_CURRENT_30A (1), MAX_REGEN_CURRENT_10A (2), MAX_REGEN_CURRENT_0A (3) |
| `UI_closureConfirmed` | — | Interface — Fermeture confirmé. Valeurs : CONFIRMED_NONE (0), CONFIRMED_FRUNK (1), CONFIRMED_PROX (2) |

### 0x335 — RearDIinfo

**Informations de l'onduleur arrière : versions, identifiants, calibrations**

_Aucun signal défini._

### 0x336 — MaxPowerRating

**Puissance maximale autorisée du véhicule** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `DriveRegenRating336` | kW | Traction récupération rating336 |
| `DrivePowerRating336` | kW | Traction puissance rating336 |
| `DI_performancePackage` | — | Onduleur — Performance package. Valeurs : BASE (0), PERFORMANCE (1), BASE_PLUS (3), SNA (4) |

### 0x33A — UI_rangeSOC

**Autonomie et état de charge (SoC) affichés : autonomie en miles/km, pourcentage batterie**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_idealRange` | mi | Autonomie théorique en conditions idéales |
| `UI_Range` | mi | Autonomie restante affichée (en miles, ×1,609 = km) |
| `UI_SOC` | % | Pourcentage de batterie affiché à l'écran |
| `UI_uSOE` | % | État d'énergie utilisable affiché |
| `UI_ratedWHpM` | WHpM | Interface — Rated W pompe à chaleur M |

### 0x352 — MS_energyStatus

**État d'énergie de la batterie : énergie restante, nominale, idéale, tampon, énergie totale du pack** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `BMS_energyBuffer` | KWh | Réserve d'énergie tampon (non utilisable). Valeurs : SNA (255) |
| `BMS_energyToChargeComplete` | KWh | Énergie nécessaire pour terminer la charge. Valeurs : SNA (2047) |
| `BMS_expectedEnergyRemaining` | KWh | Énergie restante attendue dans la batterie. Valeurs : SNA (2047) |
| `BMS_fullChargeComplete` | — | Charge complète terminée |
| `BMS_idealEnergyRemaining` | KWh | Énergie restante en conditions idéales. Valeurs : SNA (2047) |
| `BMS_nominalEnergyRemaining` | KWh | Énergie restante nominale. Valeurs : SNA (2047) |
| `BMS_nominalFullPackEnergy` | KWh | Énergie totale nominale du pack batterie plein. Valeurs : SNA (2047) |

### 0x353 — UI_status

**État de l'interface utilisateur (doublon sur bus 0x353) : mêmes signaux que 0x00C**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_audioActive` | — | Audio en cours de lecture |
| `UI_autopilotTrial` | — | État de l'essai Autopilot |
| `UI_bluetoothActive` | — | Bluetooth activé |
| `UI_cameraActive` | — | Caméra de recul active |
| `UI_cellActive` | — | Module cellulaire (4G/LTE) activé |
| `UI_cellConnected` | — | Connexion cellulaire établie |
| `UI_cellNetworkTechnology` | — | Type de réseau cellulaire utilisé |
| `UI_cellReceiverPower` | dB | Puissance du signal cellulaire reçu |
| `UI_cellSignalBars` | — | Nombre de barres de signal cellulaire |
| `UI_cpuTemperature` | C | Température du processeur de l'écran tactile |
| `UI_developmentCar` | — | Véhicule de développement/test |
| `UI_displayOn` | — | Écran tactile allumé |
| `UI_displayReady` | — | Écran tactile prêt à l'utilisation |
| `UI_factoryReset` | — | Mode de réinitialisation usine |
| `UI_falseTouchCounter` | 1 | Compteur de faux appuis sur l'écran tactile |
| `UI_gpsActive` | — | Module GPS activé |
| `UI_pcbTemperature` | C | Température de la carte électronique de l'écran |
| `UI_radioActive` | — | Radio FM/AM active |
| `UI_readyForDrive` | — | Véhicule prêt à rouler |
| `UI_screenshotActive` | — | Capture d'écran en cours |
| `UI_systemActive` | — | Système d'infodivertissement actif |
| `UI_touchActive` | — | Écran tactile réactif aux appuis |
| `UI_vpnActive` | — | Tunnel VPN Tesla actif |
| `UI_wifiActive` | — | Module WiFi activé |
| `UI_wifiConnected` | — | WiFi connecté à un réseau |

### 0x376 — rontInverterTemps

**Températures de l'onduleur avant : stator, rotor, huile, transistors, carte de commande** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `TempInvPCB376` | C | Température inv carte électronique |
| `TempPctStator376` | % | Température du stator avant (% de la limite) |
| `TempPctInverter376` | % | Température de l'onduleur avant (% de la limite) |
| `TempInvCapbank376` | C | Température inv banc de condensateurs |
| `TempStator376` | C | Température du stator avant |
| `TempInvHeatsink376` | C | Température inv heatsink376 |
| `TempInverter376` | C | Température onduleur |
| `DIF_inverterTQF` | — | Onduleur avant — Onduleur TQF. Valeurs : INVERTERT_INIT (0), INVERTERT_IRRATIONAL (1), INVERTERT_RATIONAL (2), INVERTERT_UNKNOWN (3) |

### 0x37D — P_thermalStatus

**État thermique du port de charge : températures des connecteurs, câble, prise**

| Signal | Unité | Description |
|--------|-------|-------------|
| `CP_acPinTemperature` | C | Port de charge — Courant alternatif (AC) broche température |
| `CP_dTdt_dcPinActual` | C | Port de charge — D tdt courant continu (DC) broche réel |
| `CP_dTdt_dcPinExpected` | C | Port de charge — D tdt courant continu (DC) broche attendu |
| `CP_dcPinTemperature` | C | Port de charge — Courant continu (DC) broche température |
| `CP_dcPinTemperatureModeled` | C | Température modélisée de la broche de charge CC (DC) |

### 0x381 — VCFRONT_logging1Hz

**Journalisation du contrôleur avant (1 Hz) : données à basse fréquence**

_Aucun signal défini._

### 0x383 — VCRIGHT_thsStatus

**Capteurs environnementaux côté droit : température, humidité, charge solaire (infrarouge et visible)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCRIGHT_estimatedThsSolarLoad` | W/m2 | Côté droit — Estimé ths solar load. Valeurs : SNA (1023) |
| `VCRIGHT_estimatedVehicleSituatio` | — | Côté droit — Estimé véhicule situatio. Valeurs : VEHICLE_SITUATION_UNKNOWN (0), VEHICLE_SITUATION_INDOOR (1), VEHICLE_SITUATION_OUTDOOR (2) |
| `VCRIGHT_thsActive` | — | Côté droit — Ths actif |
| `VCRIGHT_thsHumidity` | % | Côté droit — Ths humidity. Valeurs : SNA (255) |
| `VCRIGHT_thsSolarLoadInfrared` | W/m2 | Côté droit — Ths solar load infrared. Valeurs : SNA (1023) |
| `VCRIGHT_thsSolarLoadVisible` | W/m2 | Côté droit — Ths solar load visible. Valeurs : SNA (1023) |
| `VCRIGHT_thsTemperature` | C | Côté droit — Ths température. Valeurs : SNA (128) |

### 0x392 — MS_packConfig

**Configuration du pack batterie : numéro de série, type de modules, masse, capacité**

_Aucun signal défini._

### 0x395 — IR_oilPump

**Pompe à huile de l'onduleur arrière : débit, pression, état**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DIR_oilPumpFlowActual` | LPM | Onduleur arrière — Huile pompe débit réel |
| `DIR_oilPumpFlowTarget` | LPM | Onduleur arrière — Huile pompe débit cible |
| `DIR_oilPumpFluidT` | C | Température du liquide de la pompe à huile (arrière) |
| `DIR_oilPumpFluidTQF` | — | Qualité de la mesure de température du liquide de pompe à huile. Valeurs : OIL_PUMP_FLUIDT_LOW_CONFIDENCE (0), OIL_PUMP_FLUIDT_HIGH_CONFIDENCE (1) |
| `DIR_oilPumpLeadAngle` | deg | Onduleur arrière — Huile pompe ligne angle |
| `DIR_oilPumpPhaseCurrent` | A | Onduleur arrière — Huile pompe phase courant |
| `DIR_oilPumpPressureEstimate` | kPa | Onduleur arrière — Huile pompe pression estimate |
| `DIR_oilPumpPressureExpected` | kPa | Onduleur arrière — Huile pompe pression attendu |
| `DIR_oilPumpPressureResidual` | kPa | Onduleur arrière — Huile pompe pression residual |
| `DIR_oilPumpState` | — | Onduleur arrière — Huile pompe état. Valeurs : OIL_PUMP_STANDBY (0), OIL_PUMP_ENABLE (1), OIL_PUMP_COLD_STARTUP (2), OIL_PUMP_FAULTED (6), OIL_PUMP_SNA (7) |

### 0x396 — rontOilPump

**Pompe à huile de l'onduleur avant : débit, pression, état**

| Signal | Unité | Description |
|--------|-------|-------------|
| `FrontOilPumpPressureExpected396` | kPa | Avant huile pompe pression attendu |
| `FrontOilPumpPhaseCurrent396` | A | Avant huile pompe phase courant |
| `FrontOilFlowActual396` | LPM | Avant huile débit réel |
| `FrontOilPumpDutyCycle396` | % | Avant huile pompe rapport cyclique cycle |
| `FrontOilPumpOilTempEst396` | C | Avant huile pompe huile température estimé |
| `FrontOilPumpFluidTemp396` | C | Température du liquide de la pompe à huile (avant) |
| `FrontOilPumpState396` | — | Avant huile pompe état. Valeurs : OIL_PUMP_STANDBY (0), OIL_PUMP_ENABLE (1), OIL_PUMP_COLD_STARTUP (2), OIL_PUMP_FAULTED (6), OIL_PUMP_SNA (7) |
| `FrontOilPumpPressureEstimate396` | kPa | Avant huile pompe pression estimate396 |
| `FrontOilPumpOilTempEstConfident3` | — | Avant huile pompe huile température estimé confident3 |
| `FrontOilPumpLeadAngle396` | deg | Avant huile pompe ligne angle |

### 0x3A1 — VCFRONT_vehicleStatus

**État du véhicule depuis le contrôleur avant : contacteurs, alimentation, veille**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCFRONT_12vStatusForDrive` | — | État de l'alimentation 12 V pour la conduite. Valeurs : NOT_READY_FOR_DRIVE_12V (0), READY_FOR_DRIVE_12V (1), EXIT_DRIVE_REQUESTED_12V (2) |
| `VCFRONT_2RowCenterUnbuckled` | — | Contrôleur avant — 2row centre unbuckled. Valeurs : CHIME_NONE (0), CHIME_OCCUPIED_AND_UNBUCKLED (1), CHIME_SNA (2) |
| `VCFRONT_2RowLeftUnbuckled` | — | Contrôleur avant — 2row gauche unbuckled. Valeurs : CHIME_NONE (0), CHIME_OCCUPIED_AND_UNBUCKLED (1), CHIME_SNA (2) |
| `VCFRONT_2RowRightUnbuckled` | — | Contrôleur avant — 2row droite unbuckled. Valeurs : CHIME_NONE (0), CHIME_OCCUPIED_AND_UNBUCKLED (1), CHIME_SNA (2) |
| `VCFRONT_APGlassHeaterState` | — | Contrôleur avant — AP glass résistance de chauffage état. Valeurs : HEATER_STATE_SNA (0), HEATER_STATE_ON (1), HEATER_STATE_OFF (2), HEATER_STATE_OFF_UNAVAILABLE (3), HEATER_STATE_FAULT (4) |
| `VCFRONT_accPlusAvailable` | — | Régulateur de vitesse adaptatif avancé disponible |
| `VCFRONT_batterySupportRequest` | — | Demande de soutien de la batterie par le contrôleur avant |
| `VCFRONT_bmsHvChargeEnable` | — | Contrôleur avant — Bms haute tension charge activation |
| `VCFRONT_diPowerOnState` | — | État de mise sous tension de l'onduleur (DI) par le contrôleur avant. Valeurs : DI_POWERED_OFF (0), DI_POWERED_ON_FOR_SUMMON (1), DI_POWERED_ON_FOR_STATIONARY_HEAT (2), DI_POWERED_ON_FOR_DRIVE (3), DI_POWER_GOING_DOWN (4) |
| `VCFRONT_driverBuckleStatus` | — | Contrôleur avant — Conducteur boucle de ceinture état. Valeurs : UNBUCKLED (0), BUCKLED (1) |
| `VCFRONT_driverDoorStatus` | — | Contrôleur avant — Conducteur portière état. Valeurs : DOOR_OPEN (0), DOOR_CLOSED (1) |
| `VCFRONT_driverIsLeaving` | — | Le conducteur quitte le véhicule |
| `VCFRONT_driverIsLeavingAnySpeed` | — | Le conducteur quitte le véhicule (toute vitesse) |
| `VCFRONT_driverPresent` | — | Conducteur détecté (présent) |
| `VCFRONT_driverUnbuckled` | — | Contrôleur avant — Conducteur unbuckled. Valeurs : CHIME_NONE (0), CHIME_OCCUPIED_AND_UNBUCKLED (1), CHIME_SNA (2) |
| `VCFRONT_ota12VSupportRequest` | — | Contrôleur avant — Ota12v support demande |
| `VCFRONT_passengerPresent` | — | Passager détecté (présent) |
| `VCFRONT_passengerUnbuckled` | — | Contrôleur avant — Passager unbuckled. Valeurs : CHIME_NONE (0), CHIME_OCCUPIED_AND_UNBUCKLED (1), CHIME_SNA (2) |
| `VCFRONT_pcs12vVoltageTarget` | V | Contrôleur avant — Pcs12v tension cible |
| `VCFRONT_pcsEFuseVoltage` | V | Tension du fusible électronique du convertisseur PCS. Valeurs : SNA (1023) |
| `VCFRONT_preconditionRequest` | — | Demande de préconditionnement (contrôleur avant) |
| `VCFRONT_standbySupplySupported` | — | Alimentation en veille (standby) prise en charge par le contrôleur avant |
| `VCFRONT_thermalSystemType` | — | Contrôleur avant — Thermique système type. Valeurs : LEGACY_THERMAL_SYSTEM (0), HEAT_PUMP_THERMAL_SYSTEM (1) |
| `VCFRONT_vehicleStatusChecksum` | — | Contrôleur avant — Véhicule état somme de contrôle |
| `VCFRONT_vehicleStatusCounter` | — | Contrôleur avant — Véhicule état compteur |

### 0x3B3 — UI_vehicleControl2

**Commandes véhicule depuis l'interface (partie 2) : réglages avancés**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_PINToDriveEnabled` | — | Code PIN requis pour démarrer (activé) |
| `UI_PINToDrivePassed` | — | Code PIN de démarrage validé |
| `UI_UMCUpdateInhibit` | — | Inhibition de mise à jour du chargeur mobile (UMC) |
| `UI_VCLEFTFeature1` | — | Interface — Vcleft fonction |
| `UI_VCSECFeature1` | — | Interface — VCSEC fonction |
| `UI_WCUpdateInhibit` | — | Inhibition de mise à jour du connecteur mural (WC) |
| `UI_alarmTriggerRequest` | — | Demande de déclenchement de l'alarme |
| `UI_autoRollWindowsOnLockEnable` | — | Fermeture automatique des vitres au verrouillage |
| `UI_autopilotPowerStateRequest` | — | Interface — Autopilot puissance état demande. Valeurs : AUTOPILOT_NOMINAL (0), AUTOPILOT_SENTRY (1), AUTOPILOT_SUSPEND (2) |
| `UI_batteryPreconditioningRequest` | — | Demande de préconditionnement de la batterie |
| `UI_coastDownMode` | — | Interface — Roulage libre descente mode |
| `UI_conditionalLoggingEnabledVCSE` | — | Enregistrement conditionnel activé pour le VCSEC |
| `UI_efuseMXResistanceEstArmed` | — | Estimation de résistance du fusible électronique MX armée |
| `UI_freeRollModeRequest` | — | Demande de mode roue libre |
| `UI_gloveboxRequest` | — | Demande d'ouverture de la boîte à gants |
| `UI_lightSwitch` | — | Interface — Feu interrupteur. Valeurs : LIGHT_SWITCH_AUTO (0), LIGHT_SWITCH_ON (1), LIGHT_SWITCH_PARKING (2), LIGHT_SWITCH_OFF (3), LIGHT_SWITCH_SNA (4) |
| `UI_locksPanelActive` | — | Panneau de verrouillage actif |
| `UI_readyToAddKey` | — | Véhicule prêt à enregistrer une nouvelle clé |
| `UI_shorted12VCellTestMode` | — | Mode test de cellule 12 V en court-circuit. Valeurs : SHORTED_CELL_TEST_MODE_DISABLED (0), SHORTED_CELL_TEST_MODE_SHADOW (1), SHORTED_CELL_TEST_MODE_ACTIVE (2) |
| `UI_soundHornOnLock` | — | Interface — Sound klaxon activé verrouillage |
| `UI_summonState` | — | Interface — Convocation (Summon) état. Valeurs : SNA (0), IDLE (1), PRE_PRIMED (2), ACTIVE (3) |
| `UI_trunkRequest` | — | Interface — Coffre demande |
| `UI_userPresent` | — | Utilisateur détecté (présent) |
| `UI_windowRequest` | — | Interface — Vitre demande. Valeurs : WINDOW_REQUEST_IDLE (0), WINDOW_REQUEST_GOTO_PERCENT (1), WINDOW_REQUEST_GOTO_VENT (2), WINDOW_REQUEST_GOTO_CLOSED (3), WINDOW_REQUEST_GOTO_OPEN (4) |

### 0x3B6 — odometer

**Compteur kilométrique (odomètre) du véhicule** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `Odometer3B6` | KM | Odometer3b6. Valeurs : SNA (4294967) |

### 0x3BB — UI_power

**Données de puissance affichées par l'interface : consommation instantanée, moyenne**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_powerExpected` | kW | Interface — Puissance attendu |
| `UI_powerIdeal` | kW | Interface — Puissance ideal |

### 0x3C2 — VCLEFT_switchStatus

**État des interrupteurs côté gauche : vitres, sièges, verrouillage, rétroviseurs** | Cycle : 50 ms

_Aucun signal défini._

### 0x3C3 — VCRIGHT_switchStatus

**État des interrupteurs côté droit : vitres, sièges, verrouillage, coffre**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCRIGHT_2RowSeatReclineSwitch` | — | Côté droit — 2row siège recline interrupteur |
| `VCRIGHT_btnWindowAutoDownRF` | — | Côté droit — Bouton vitre automatique descente avant droit |
| `VCRIGHT_btnWindowAutoDownRR` | — | Côté droit — Bouton vitre automatique descente arrière droit |
| `VCRIGHT_btnWindowAutoUpRF` | — | Côté droit — Bouton vitre automatique montée avant droit |
| `VCRIGHT_btnWindowAutoUpRR` | — | Côté droit — Bouton vitre automatique montée arrière droit |
| `VCRIGHT_btnWindowDownRF` | — | Côté droit — Bouton vitre descente avant droit |
| `VCRIGHT_btnWindowDownRR` | — | Côté droit — Bouton vitre descente arrière droit |
| `VCRIGHT_btnWindowSwPackAutoDwnLF` | — | Côté droit — Bouton vitre interrupteur pack batterie automatique dwn avant gauche |
| `VCRIGHT_btnWindowSwPackAutoDwnLR` | — | Côté droit — Bouton vitre interrupteur pack batterie automatique dwn arrière gauche |
| `VCRIGHT_btnWindowSwPackAutoDwnRR` | — | Côté droit — Bouton vitre interrupteur pack batterie automatique dwn arrière droit |
| `VCRIGHT_btnWindowSwPackAutoUpLF` | — | Côté droit — Bouton vitre interrupteur pack batterie automatique montée avant gauche |
| `VCRIGHT_btnWindowSwPackAutoUpLR` | — | Côté droit — Bouton vitre interrupteur pack batterie automatique montée arrière gauche |
| `VCRIGHT_btnWindowSwPackAutoUpRR` | — | Côté droit — Bouton vitre interrupteur pack batterie automatique montée arrière droit |
| `VCRIGHT_btnWindowSwPackDownLF` | — | Côté droit — Bouton vitre interrupteur pack batterie descente avant gauche |
| `VCRIGHT_btnWindowSwPackDownLR` | — | Côté droit — Bouton vitre interrupteur pack batterie descente arrière gauche |
| `VCRIGHT_btnWindowSwPackDownRR` | — | Côté droit — Bouton vitre interrupteur pack batterie descente arrière droit |
| `VCRIGHT_btnWindowSwPackUpLF` | — | Côté droit — Bouton vitre interrupteur pack batterie montée avant gauche |
| `VCRIGHT_btnWindowSwPackUpLR` | — | Côté droit — Bouton vitre interrupteur pack batterie montée arrière gauche |
| `VCRIGHT_btnWindowSwPackUpRR` | — | Côté droit — Bouton vitre interrupteur pack batterie montée arrière droit |
| `VCRIGHT_btnWindowUpRF` | — | Côté droit — Bouton vitre montée avant droit |
| `VCRIGHT_btnWindowUpRR` | — | Côté droit — Bouton vitre montée arrière droit |
| `VCRIGHT_frontBuckleSwitch` | — | Côté droit — Avant boucle de ceinture interrupteur. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontOccupancySwitch` | — | Côté droit — Avant occupancy interrupteur. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontSeatBackrestBack` | — | Recul du dossier du siège avant droit. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontSeatBackrestForward` | — | Côté droit — Avant siège dossier marche avant. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontSeatLiftDown` | — | Descente de l'assise du siège avant droit. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontSeatLiftUp` | — | Montée de l'assise du siège avant droit. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontSeatLumbarDown` | — | Côté droit — Avant siège soutien lombaire descente. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontSeatLumbarIn` | — | Côté droit — Avant siège soutien lombaire in. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontSeatLumbarOut` | — | Saillie du soutien lombaire du siège avant droit. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontSeatLumbarUp` | — | Côté droit — Avant siège soutien lombaire montée. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontSeatTiltDown` | — | Côté droit — Avant siège inclinaison descente. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontSeatTiltUp` | — | Côté droit — Avant siège inclinaison montée. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontSeatTrackBack` | — | Recul de la glissière du siège avant droit. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_frontSeatTrackForward` | — | Côté droit — Avant siège circuit marche avant. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_liftgateShutfaceSwitchPr` | — | État de l'interrupteur de fermeture du hayon (côté droit) |
| `VCRIGHT_rearCenterBuckleSwitch` | — | Côté droit — Arrière centre boucle de ceinture interrupteur. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_rearRightBuckleSwitch` | — | Côté droit — Arrière droite boucle de ceinture interrupteur. Valeurs : SWITCH_SNA (0), SWITCH_OFF (1), SWITCH_ON (2), SWITCH_FAULT (3) |
| `VCRIGHT_trunkExtReleasePressed` | — | Côté droit — Coffre extérieur libération appuyé |

### 0x3D2 — TotalChargeDischarge

**Totaux de charge et décharge : énergie AC/DC chargée, déchargée, récupérée** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `TotalDischargeKWh3D2` | kWh | Total décharge K wh3d2 |
| `TotalChargeKWh3D2` | kWh | Total charge K wh3d2 |

### 0x3D8 — levation

**Altitude (élévation) du véhicule** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `Elevation3D8` | M | Elevation3d8 |

### 0x3E2 — VCLEFT_lightStatus

**État de l'éclairage côté gauche : phares, clignotants, feux de position**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCLEFT_FLMapLightStatus` | — | Côté gauche — Avant gauche carte feu état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCLEFT_FLMapLightSwitchPressed` | — | Côté gauche — Avant gauche carte feu interrupteur appuyé |
| `VCLEFT_FRMapLightStatus` | — | Côté gauche — Avant droit carte feu état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCLEFT_FRMapLightSwitchPressed` | — | Côté gauche — Avant droit carte feu interrupteur appuyé |
| `VCLEFT_RLMapLightStatus` | — | Côté gauche — Arrière gauche carte feu état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCLEFT_RLMapLightSwitchPressed` | — | Côté gauche — Arrière gauche carte feu interrupteur appuyé |
| `VCLEFT_RRMapLightStatus` | — | Côté gauche — Arrière droit carte feu état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCLEFT_RRMapLightSwitchPressed` | — | Côté gauche — Arrière droit carte feu interrupteur appuyé |
| `VCLEFT_brakeLightStatus` | — | Côté gauche — Frein feu état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCLEFT_brakeTrailerLightStatus` | — | Côté gauche — Frein trailer feu état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCLEFT_dynamicBrakeLightStatus` | — | Côté gauche — Dynamique frein feu état. Valeurs : DYNAMIC_BRAKE_LIGHT_OFF (0), DYNAMIC_BRAKE_LIGHT_ACTIVE_LOW (1), DYNAMIC_BRAKE_LIGHT_ACTIVE_HIGH (2) |
| `VCLEFT_fogTrailerLightStatus` | — | Côté gauche — Antibrouillard trailer feu état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCLEFT_frontRideHeight` | mm | Côté gauche — Avant ride height. Valeurs : SNA (128) |
| `VCLEFT_leftTurnTrailerLightStatu` | — | Côté gauche — Gauche clignotant trailer feu statu. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCLEFT_rearRideHeight` | mm | Côté gauche — Arrière ride height. Valeurs : SNA (128) |
| `VCLEFT_reverseTrailerLightStatus` | — | Côté gauche — Marche arrière trailer feu état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCLEFT_rideHeightSensorFault` | — | Côté gauche — Ride height capteur défaut |
| `VCLEFT_rightTrnTrailerLightStatu` | — | Côté gauche — Droite trn trailer feu statu. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCLEFT_tailLightOutageStatus` | — | État de défaillance du feu arrière gauche |
| `VCLEFT_tailLightStatus` | — | État du feu arrière gauche. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCLEFT_tailTrailerLightStatus` | — | État du feu de remorque arrière gauche. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCLEFT_trailerDetected` | — | Côté gauche — Trailer détecté. Valeurs : TRAILER_LIGHT_DETECTION_SNA (0), TRAILER_LIGHT_DETECTION_FAULT (1), TRAILER_LIGHT_DETECTION_DETECTED (2), TRAILER_LIGHT_DETECTION_NOT_DETECTED (3) |
| `VCLEFT_turnSignalStatus` | — | État du clignotant gauche. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |

### 0x3E3 — VCRIGHT_lightStatus

**État de l'éclairage côté droit : phares, clignotants, feux de position**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCRIGHT_brakeLightStatus` | — | Côté droit — Frein feu état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCRIGHT_interiorTrunkLightStatus` | — | Côté droit — Interior coffre feu état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCRIGHT_rearFogLightStatus` | — | Côté droit — Arrière antibrouillard feu état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCRIGHT_reverseLightStatus` | — | Côté droit — Marche arrière feu état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCRIGHT_tailLightStatus` | — | État du feu arrière droit. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCRIGHT_turnSignalStatus` | — | État du clignotant droit. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |

### 0x3E9 — S_bodyControls

**Commandes de carrosserie par le système de conduite autonome : clignotants, essuie-glaces, klaxon**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DAS_bodyControlsChecksum` | — | Somme de contrôle du message de contrôle carrosserie |
| `DAS_bodyControlsCounter` | — | Compteur de séquence du message de contrôle carrosserie |
| `DAS_dynamicBrakeLightRequest` | — | Conduite autonome — Dynamique frein feu demande |
| `DAS_hazardLightRequest` | — | Demande d'activation des feux de détresse (DAS). Valeurs : DAS_REQUEST_HAZARDS_OFF (0), DAS_REQUEST_HAZARDS_ON (1), DAS_REQUEST_HAZARDS_UNUSED (2), DAS_REQUEST_HAZARDS_SNA (3) |
| `DAS_headlightRequest` | — | Conduite autonome — Phare demande. Valeurs : DAS_HEADLIGHT_REQUEST_OFF (0), DAS_HEADLIGHT_REQUEST_ON (1), DAS_HEADLIGHT_REQUEST_INVALID (3) |
| `DAS_heaterRequest` | — | Conduite autonome — Résistance de chauffage demande. Valeurs : DAS_HEATER_SNA (0), DAS_HEATER_OFF (1), DAS_HEATER_ON (2) |
| `DAS_highLowBeamDecision` | — | Conduite autonome — Haut bas faisceau décision. Valeurs : DAS_HIGH_BEAM_UNDECIDED (0), DAS_HIGH_BEAM_OFF (1), DAS_HIGH_BEAM_ON (2), DAS_HIGH_BEAM_SNA (3) |
| `DAS_highLowBeamOffReason` | — | Conduite autonome — Haut bas faisceau désactivé raison. Valeurs : HIGH_BEAM_ON (0), HIGH_BEAM_OFF_REASON_MOVING_VISION_TARGET (1), HIGH_BEAM_OFF_REASON_MOVING_RADAR_TARGET (2), HIGH_BEAM_OFF_REASON_AMBIENT_LIGHT (3), HIGH_BEAM_OFF_REASON_HEAD_LIGHT (4), HIGH_BEAM_OFF_REASON_SNA (5) |
| `DAS_turnIndicatorRequest` | — | Conduite autonome — Clignotant indicateur demande. Valeurs : DAS_TURN_INDICATOR_NONE (0), DAS_TURN_INDICATOR_LEFT (1), DAS_TURN_INDICATOR_RIGHT (2), DAS_TURN_INDICATOR_CANCEL (3), DAS_TURN_INDICATOR_DEFER (4) |
| `DAS_turnIndicatorRequestReason` | — | Conduite autonome — Clignotant indicateur demande raison. Valeurs possibles : DAS_NONE (0), DAS_ACTIVE_NAV_LANE_CHANGE (1), DAS_ACTIVE_SPEED_LANE_CHANGE (2), DAS_ACTIVE_FORK (3), DAS_CANCEL_LANE_CHANGE (4), DAS_CANCEL_FORK (5), DAS_ACTIVE_MERGE (6), DAS_CANCEL_MERGE (7), DAS_ACTIVE_COMMANDED_LANE_CHANGE (8), DAS_ACTIVE_INTERSECTION (9), DAS_CANCEL_INTERSECTION (10), DAS_ACTIVE_SUMMMON (11), DAS_CANCEL_SUMMMON (12) |
| `DAS_wiperSpeed` | — | Conduite autonome — Essuie-glace vitesse. Valeurs possibles : DAS_WIPER_SPEED_OFF (0), DAS_WIPER_SPEED_1 (1), DAS_WIPER_SPEED_2 (2), DAS_WIPER_SPEED_3 (3), DAS_WIPER_SPEED_4 (4), DAS_WIPER_SPEED_5 (5), DAS_WIPER_SPEED_6 (6), DAS_WIPER_SPEED_7 (7), DAS_WIPER_SPEED_8 (8), DAS_WIPER_SPEED_9 (9), DAS_WIPER_SPEED_10 (10), DAS_WIPER_SPEED_11 (11), DAS_WIPER_SPEED_12 (12), DAS_WIPER_SPEED_13 (13), DAS_WIPER_SPEED_14 (14), DAS_WIPER_SPEED_INVALID (15) |
| `DAS_radarHeaterRequest` | — | Conduite autonome — Radar résistance de chauffage demande |
| `DAS_ahlbOverride` | — | Conduite autonome — Ahlb forçage |
| `DAS_mirrorFoldRequest` | — | Demande de repliage des rétroviseurs (conduite autonome). Valeurs : NONE (0), FOLD (1), UNFOLD (2), SNA (3) |

### 0x3F2 — MSCounters

**Compteurs du BMS : cycles, événements, historique**

_Aucun signal défini._

### 0x3F5 — VCFRONT_lighting

**Éclairage géré par le contrôleur avant : phares, feux de jour, anti-brouillards**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCFRONT_DRLLeftStatus` | — | Contrôleur avant — Feux de jour (DRL) gauche état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_DRLRightStatus` | — | Contrôleur avant — Feux de jour (DRL) droite état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_ambientLightingBrightnes` | % | Luminosité de l'éclairage d'ambiance. Valeurs : SNA (255) |
| `VCFRONT_approachLightingRequest` | — | Demande d'éclairage à l'approche du véhicule |
| `VCFRONT_courtesyLightingRequest` | — | Demande d'éclairage de courtoisie |
| `VCFRONT_fogLeftStatus` | — | Contrôleur avant — Antibrouillard gauche état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_fogRightStatus` | — | Contrôleur avant — Antibrouillard droite état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_hazardLightRequest` | — | Demande d'activation des feux de détresse. Valeurs possibles : HAZARD_REQUEST_NONE (0), HAZARD_REQUEST_BUTTON (1), HAZARD_REQUEST_LOCK (2), HAZARD_REQUEST_UNLOCK (3), HAZARD_REQUEST_MISLOCK (4), HAZARD_REQUEST_CRASH (5), HAZARD_REQUEST_CAR_ALARM (6), HAZARD_REQUEST_DAS (7), HAZARD_REQUEST_UDS (8) |
| `VCFRONT_hazardSwitchBacklight` | — | Rétro-éclairage du bouton de feux de détresse |
| `VCFRONT_highBeamLeftStatus` | — | Contrôleur avant — Haut faisceau gauche état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_highBeamRightStatus` | — | Contrôleur avant — Haut faisceau droite état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_highBeamSwitchActive` | — | Contrôleur avant — Haut faisceau interrupteur actif |
| `VCFRONT_indicatorLeftRequest` | — | Contrôleur avant — Indicateur gauche demande. Valeurs : TURN_SIGNAL_OFF (0), TURN_SIGNAL_ACTIVE_LOW (1), TURN_SIGNAL_ACTIVE_HIGH (2) |
| `VCFRONT_indicatorRightRequest` | — | Contrôleur avant — Indicateur droite demande. Valeurs : TURN_SIGNAL_OFF (0), TURN_SIGNAL_ACTIVE_LOW (1), TURN_SIGNAL_ACTIVE_HIGH (2) |
| `VCFRONT_lowBeamLeftStatus` | — | Contrôleur avant — Bas faisceau gauche état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_lowBeamRightStatus` | — | Contrôleur avant — Bas faisceau droite état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_lowBeamsCalibrated` | — | Contrôleur avant — Bas faisceaux calibré |
| `VCFRONT_lowBeamsOnForDRL` | — | Feux de croisement allumés en remplacement des feux de jour |
| `VCFRONT_parkLeftStatus` | — | Contrôleur avant — Stationnement gauche état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_parkRightStatus` | — | Contrôleur avant — Stationnement droite état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_seeYouHomeLightingReq` | — | Demande d'éclairage d'accompagnement « See You Home » |
| `VCFRONT_sideMarkersStatus` | — | Contrôleur avant — Côté markers état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_sideRepeaterLeftStatus` | — | Contrôleur avant — Côté repeater gauche état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_sideRepeaterRightStatus` | — | Contrôleur avant — Côté repeater droite état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_simLatchingStalk` | — | Simulation du commodo à verrouillage. Valeurs : SIMULATED_LATCHING_STALK_IDLE (0), SIMULATED_LATCHING_STALK_LEFT (1), SIMULATED_LATCHING_STALK_RIGHT (2), SIMULATED_LATCHING_STALK_SNA (3) |
| `VCFRONT_switchLightingBrightness` | % | Luminosité de l'éclairage des interrupteurs. Valeurs : SNA (255) |
| `VCFRONT_turnSignalLeftStatus` | — | Contrôleur avant — Clignotant signal gauche état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |
| `VCFRONT_turnSignalRightStatus` | — | Contrôleur avant — Clignotant signal droite état. Valeurs : LIGHT_OFF (0), LIGHT_ON (1), LIGHT_FAULT (2), LIGHT_SNA (3) |

### 0x3FE — rakeTemps

**Températures des freins : disques avant et arrière** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `BrakeTempFL3FE` | C | Frein température FL3FE |
| `BrakeTempFR3FE` | C | Frein température FR3FE |
| `BrakeTempRL3FE` | C | Frein température RL3FE |
| `BrakeTempRR3FE` | C | Frein température RR3FE |

### 0x401 — rickVoltages

**Tensions individuelles des cellules (briques) de la batterie haute tension**

| Signal | Unité | Description |
|--------|-------|-------------|
| `StatusFlags` | — | État flags |

### 0x405 — VIN

**Numéro d'identification du véhicule (VIN)** | Cycle : 205 ms

_Aucun signal défini._

### 0x42A — VCSEC_TPMSConnectionData

**Données de connexion des capteurs de pression des pneus (TPMS)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCSEC_TPMSConnectionTypeCurrent0` | — | Type de connexion actuel du capteur TPMS nº 0. Valeurs : CONNECTIONTYPE_FAST (0), CONNECTIONTYPE_SLOW (1), CONNECTIONTYPE_UNKNOWN (2) |
| `VCSEC_TPMSConnectionTypeCurrent1` | — | Type de connexion actuel du capteur TPMS nº 1. Valeurs : CONNECTIONTYPE_FAST (0), CONNECTIONTYPE_SLOW (1), CONNECTIONTYPE_UNKNOWN (2) |
| `VCSEC_TPMSConnectionTypeCurrent2` | — | Type de connexion actuel du capteur TPMS nº 2. Valeurs : CONNECTIONTYPE_FAST (0), CONNECTIONTYPE_SLOW (1), CONNECTIONTYPE_UNKNOWN (2) |
| `VCSEC_TPMSConnectionTypeCurrent3` | — | Type de connexion actuel du capteur TPMS nº 3. Valeurs : CONNECTIONTYPE_FAST (0), CONNECTIONTYPE_SLOW (1), CONNECTIONTYPE_UNKNOWN (2) |
| `VCSEC_TPMSConnectionTypeDesired0` | — | Type de connexion souhaité du capteur TPMS nº 0. Valeurs : CONNECTIONTYPE_FAST (0), CONNECTIONTYPE_SLOW (1), CONNECTIONTYPE_UNKNOWN (2) |
| `VCSEC_TPMSConnectionTypeDesired1` | — | Type de connexion souhaité du capteur TPMS nº 1. Valeurs : CONNECTIONTYPE_FAST (0), CONNECTIONTYPE_SLOW (1), CONNECTIONTYPE_UNKNOWN (2) |
| `VCSEC_TPMSConnectionTypeDesired2` | — | Type de connexion souhaité du capteur TPMS nº 2. Valeurs : CONNECTIONTYPE_FAST (0), CONNECTIONTYPE_SLOW (1), CONNECTIONTYPE_UNKNOWN (2) |
| `VCSEC_TPMSConnectionTypeDesired3` | — | Type de connexion souhaité du capteur TPMS nº 3. Valeurs : CONNECTIONTYPE_FAST (0), CONNECTIONTYPE_SLOW (1), CONNECTIONTYPE_UNKNOWN (2) |
| `VCSEC_TPMSRSSI0` | dBm | Intensité du signal radio du capteur TPMS nº 0 |
| `VCSEC_TPMSRSSI1` | dBm | Intensité du signal radio du capteur TPMS nº 1 |
| `VCSEC_TPMSRSSI2` | dBm | Intensité du signal radio du capteur TPMS nº 2 |
| `VCSEC_TPMSRSSI3` | dBm | Intensité du signal radio du capteur TPMS nº 3 |
| `VCSEC_TPMSSensorState0` | — | État du capteur TPMS nº 0. Valeurs : SENSOR_NOT_PAIRED (0), SENSOR_WAIT_FOR_ADV (1), SENSOR_WAIT_FOR_CONN (2), SENSOR_CONNECTED (3), SENSOR_DISCONNECTING (4) |
| `VCSEC_TPMSSensorState1` | — | État du capteur TPMS nº 1. Valeurs : SENSOR_NOT_PAIRED (0), SENSOR_WAIT_FOR_ADV (1), SENSOR_WAIT_FOR_CONN (2), SENSOR_CONNECTED (3), SENSOR_DISCONNECTING (4) |
| `VCSEC_TPMSSensorState2` | — | État du capteur TPMS nº 2. Valeurs : SENSOR_NOT_PAIRED (0), SENSOR_WAIT_FOR_ADV (1), SENSOR_WAIT_FOR_CONN (2), SENSOR_CONNECTED (3), SENSOR_DISCONNECTING (4) |
| `VCSEC_TPMSSensorState3` | — | État du capteur TPMS nº 3. Valeurs : SENSOR_NOT_PAIRED (0), SENSOR_WAIT_FOR_ADV (1), SENSOR_WAIT_FOR_CONN (2), SENSOR_CONNECTED (3), SENSOR_DISCONNECTING (4) |

### 0x43D — P_chargeStatusLog

**Journal de l'état de charge : historique des sessions, erreurs**

| Signal | Unité | Description |
|--------|-------|-------------|
| `CP_acChargeCurrentLimit_log` | A | Port de charge — Courant alternatif (AC) charge courant limite journal |
| `CP_chargeShutdownRequest_log` | — | Port de charge — Charge arrêt demande journal. Valeurs : NO_SHUTDOWN_REQUESTED (0), GRACEFUL_SHUTDOWN_REQUESTED (1), EMERGENCY_SHUTDOWN_REQUESTED (2) |
| `CP_evseChargeType_log` | — | Port de charge — Borne de charge (EVSE) charge type journal. Valeurs : NO_CHARGER_PRESENT (0), DC_CHARGER_PRESENT (1), AC_CHARGER_PRESENT (2) |
| `CP_hvChargeStatus_log` | — | Port de charge — Haute tension charge état journal. Valeurs : CP_CHARGE_INACTIVE (0), CP_CHARGE_CONNECTED (1), CP_CHARGE_STANDBY (2), CP_EXT_EVSE_TEST_ACTIVE (3), CP_EVSE_TEST_PASSED (4), CP_CHARGE_ENABLED (5), CP_CHARGE_FAULTED (6) |
| `CP_internalMaxAcCurrentLimit_log` | A | Port de charge — Interne maximum courant alternatif (AC) courant limite journal |
| `CP_internalMaxDcCurrentLimit_log` | A | Port de charge — Interne maximum courant continu (DC) courant limite journal |
| `CP_vehicleIsoCheckRequired_log` | — | Journal : vérification d'isolation du véhicule requise |
| `CP_vehiclePrechargeRequired_log` | — | Port de charge — Véhicule précharge requis journal |

### 0x51E — _info

**Informations sur la charge rapide (DC) : version, identifiants, état**

_Aucun signal défini._

### 0x528 — UnixTime

**Horodatage Unix (secondes depuis le 1er janvier 1970)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UnixTimeSeconds528` | sec | Unix temps seconds528 |

### 0x541 — stChargeMaxLimits

**Limites maximales de la charge rapide (DC) : courant et tension absolus** | Cycle : 100 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `FCMaxPowerLimit541` | kW | Charge rapide (DC) maximum puissance limite |
| `FCMaxCurrentLimit541` | A | Charge rapide (DC) maximum courant limite |

### 0x556 — rontDItemps

**Températures de l'onduleur avant : semi-conducteurs, carte, liquide de refroidissement**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DIF_ph1Temp` | C | Onduleur avant — Ph1temp |
| `DIF_ph2Temp` | C | Onduleur avant — Ph2temp |
| `DIF_ph3Temp` | C | Onduleur avant — Ph3temp |
| `DIF_pcbTemp2` | C | Onduleur avant — Carte électronique température |
| `DIF_IGBTJunctTemp` | C | Onduleur avant — IGBT junct température. Valeurs : SNA (255) |
| `DIF_lashAngle` | Deg | Onduleur avant — Jeu mécanique angle |
| `DIF_lashCheckCount` | — | Compteur de vérification du jeu mécanique (onduleur avant) |

### 0x557 — rontThermalControl

**Contrôle thermique de l'onduleur avant : ventilateur, pompe, consignes**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DIS_activeInletTempReq` | C | DIS — Actif entrée température req |
| `DIS_coolantFlowReq` | LPM | DIS — Liquide de refroidissement débit req |
| `DIS_oilFlowReq` | LPM | DIS — Huile débit req |
| `DIS_passiveInletTempReq` | C | DIS — Passif entrée température req |

### 0x5D5 — RearDItemps

**Températures de l'onduleur arrière : semi-conducteurs, carte, liquide de refroidissement**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DI_ph1Temp` | C | Onduleur — Ph1temp |
| `DI_ph2Temp` | C | Onduleur — Ph2temp |
| `DI_ph3Temp` | C | Onduleur — Ph3temp |
| `DI_pcbTemp2` | C | Onduleur — Carte électronique température |
| `DI_IGBTJunctTemp` | C | Onduleur — IGBT junct température. Valeurs : SNA (255) |

### 0x5D7 — RearThermalControl

**Contrôle thermique de l'onduleur arrière : ventilateur, pompe, consignes**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DI_activeInletTempReq` | C | Onduleur — Actif entrée température req |
| `DI_coolantFlowReq` | LPM | Onduleur — Liquide de refroidissement débit req |
| `DI_oilFlowReq` | LPM | Onduleur — Huile débit req |
| `DI_passiveInletTempReq` | C | Onduleur — Passif entrée température req |

### 0x656 — rontDIinfo

**Informations de l'onduleur avant : versions, identifiants, calibrations**

_Aucun signal défini._

### 0x72A — MS_serialNumber

**Numéro de série complet du BMS**

_Aucun signal défini._

### 0x743 — VCRIGHT_recallStatus

**État des rappels de position côté droit : sièges, rétroviseurs, volant**

| Signal | Unité | Description |
|--------|-------|-------------|
| `VCRIGHT_mirrorRecallStatus` | — | Côté droit — Rétroviseur rappel de position état. Valeurs : RECALL_SNA (0), RECALL_IN_PROGRESS (1), RECALL_COMPLETE (2), RECALL_INTERRUPTED (3) |
| `VCRIGHT_seatRecallStatus` | — | Côté droit — Siège rappel de position état. Valeurs : RECALL_SNA (0), RECALL_IN_PROGRESS (1), RECALL_COMPLETE (2), RECALL_INTERRUPTED (3) |
| `VCRIGHT_systemRecallStatus` | — | Côté droit — Système rappel de position état. Valeurs : RECALL_SNA (0), RECALL_IN_PROGRESS (1), RECALL_COMPLETE (2), RECALL_INTERRUPTED (3) |

### 0x757 — IF_debug

**Données de débogage de l'onduleur avant : variables internes, diagnostics**

_Aucun signal défini._

### 0x75D — P_sensorData

**Données des capteurs du port de charge : température, tension pilote, proximité**

_Aucun signal défini._

### 0x7D5 — IR_debug

**Données de débogage de l'onduleur arrière : variables internes, diagnostics**

_Aucun signal défini._

### 0x7FF — rConfig

**Configuration du véhicule : options, équipements, identifiants** | Cycle : 100 ms

_Aucun signal défini._

## Bus châssis (Chassis Bus)

### 0x04F — GPSLatLong

**Position GPS du véhicule : latitude et longitude** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `GPSAccuracy04F` | m | GPS accuracy04f |
| `GPSLongitude04F` | Deg | GPS longitude04f |
| `GPSLatitude04F` | Deg | GPS latitude04f |

### 0x101 — RCM_inertial1

**Module de détection de collision (RCM) : accélérations et données inertielles (partie 1)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `RCM_inertial1Checksum` | — | Somme de contrôle du message inertiel nº 1 (RCM) |
| `RCM_inertial1Counter` | — | Compteur de séquence du message inertiel nº 1 (RCM) |
| `RCM_pitchRate` | rad/s | Détection de collision (RCM) — Pitch rate. Valeurs : SNA (16384) |
| `RCM_pitchRateQF` | — | Qualité de la mesure de tangage (pitch). Valeurs : INIT (0), VALID (1), TEMP_INVALID (2), FAULTED (3) |
| `RCM_rollRate` | rad/s | Détection de collision (RCM) — Roll rate. Valeurs : SNA (16384) |
| `RCM_rollRateQF` | — | Qualité de la mesure de roulis (roll). Valeurs : INIT (0), VALID (1), TEMP_INVALID (2), FAULTED (3) |
| `RCM_yawRate` | rad/s | Détection de collision (RCM) — Yaw rate. Valeurs : SNA (32768) |
| `RCM_yawRateQF` | — | Qualité de la mesure de lacet (yaw). Valeurs : FAULTED (0), NOT_FAULTED (1) |

### 0x111 — RCM_inertial2

**Module de détection de collision (RCM) : accélérations et données inertielles (partie 2)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `RCM_inertial2Checksum` | — | Somme de contrôle du message inertiel nº 2 (RCM) |
| `RCM_inertial2Counter` | — | Compteur de séquence du message inertiel nº 2 (RCM) |
| `RCM_lateralAccel` | m/s^2 | Accélération latérale mesurée (collision). Valeurs : SNA (32768) |
| `RCM_lateralAccelQF` | — | Qualité de la mesure d'accélération latérale (collision). Valeurs : FAULTED (0), NOT_FAULTED (1) |
| `RCM_longitudinalAccel` | m/s^2 | Accélération longitudinale mesurée (collision). Valeurs : SNA (32768) |
| `RCM_longitudinalAccelQF` | — | Qualité de la mesure d'accélération longitudinale (collision). Valeurs : FAULTED (0), NOT_FAULTED (1) |
| `RCM_verticalAccel` | m/s^2 | Détection de collision (RCM) — Vertical accélération. Valeurs : SNA (32768) |
| `RCM_verticalAccelQF` | — | Détection de collision (RCM) — Vertical accélération indicateur de qualité. Valeurs : FAULTED (0), NOT_FAULTED (1) |

### 0x116 — RCM_inertial2New

**Module de détection de collision (RCM) : données inertielles (nouveau format)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `RCM_inertial2Checksum` | — | Somme de contrôle du message inertiel nº 2 (RCM) |
| `RCM_inertial2Counter` | — | Compteur de séquence du message inertiel nº 2 (RCM) |
| `RCM_lateralAccel` | m/s^2 | Accélération latérale mesurée (collision) |
| `RCM_lateralAccelQF` | — | Qualité de la mesure d'accélération latérale (collision) |
| `RCM_longitudinalAccel` | m/s^2 | Accélération longitudinale mesurée (collision) |
| `RCM_longitudinalAccelQF` | — | Qualité de la mesure d'accélération longitudinale (collision) |
| `RCM_verticalAccel` | m/s^2 | Détection de collision (RCM) — Vertical accélération |
| `RCM_verticalAccelQF` | — | Détection de collision (RCM) — Vertical accélération indicateur de qualité |

### 0x145 — SP_status

**Stabilité électronique (ESP) : état, corrections, interventions de sécurité active**

| Signal | Unité | Description |
|--------|-------|-------------|
| `ESP_absBrakeEvent2` | — | Stabilité (ESP) — ABS frein event2. Valeurs : ABS_EVENT_NOT_ACTIVE (0), ABS_EVENT_ACTIVE_FRONT_REAR (1), ABS_EVENT_ACTIVE_FRONT (2), ABS_EVENT_ACTIVE_REAR (3) |
| `ESP_absFaultLamp` | — | Stabilité (ESP) — ABS défaut lamp. Valeurs : ABS_FAULT_LAMP_OFF (0), ABS_FAULT_LAMP_ON (1) |
| `ESP_brakeApply` | — | Stabilité (ESP) — Frein application. Valeurs : BLS_INACTIVE (0), BLS_ACTIVE (1) |
| `ESP_brakeDiscWipingActive` | — | Essuyage des disques de frein actif (ESP). Valeurs : BDW_INACTIVE (0), BDW_ACTIVE (1) |
| `ESP_brakeLamp` | — | Stabilité (ESP) — Frein lamp. Valeurs : LAMP_OFF (0), LAMP_ON (1) |
| `ESP_brakeTorqueTarget` | Nm | Stabilité (ESP) — Frein couple cible. Valeurs : SNA (8191) |
| `ESP_btcTargetState` | — | État cible du chauffage de batterie (BTC) demandé par l'ESP. Valeurs : OFF (0), BACKUP (1), ON (2), SNA (3) |
| `ESP_cdpStatus` | — | Stabilité (ESP) — Cdp état. Valeurs : CDP_IS_NOT_AVAILABLE (0), CDP_IS_AVAILABLE (1), ACTUATING_EPB_CDP (2), CDP_COMMAND_INVALID (3) |
| `ESP_driverBrakeApply` | — | Stabilité (ESP) — Conducteur frein application. Valeurs : NotInit_orOff (0), Not_Applied (1), Driver_applying_brakes (2), Faulty_SNA (3) |
| `ESP_ebdFaultLamp` | — | Stabilité (ESP) — Répartition électronique de freinage (EBD) défaut lamp. Valeurs : EBD_FAULT_LAMP_OFF (0), EBD_FAULT_LAMP_ON (1) |
| `ESP_ebrStandstillSkid` | — | Stabilité (ESP) — Ebr standstill skid. Valeurs : NO_STANDSTILL_SKID (0), STANDSTILL_SKID_DETECTED (1) |
| `ESP_ebrStatus` | — | Stabilité (ESP) — Ebr état. Valeurs : EBR_IS_NOT_AVAILABLE (0), EBR_IS_AVAILABLE (1), ACTUATING_DI_EBR (2), EBR_COMMAND_INVALID (3) |
| `ESP_espFaultLamp` | — | Stabilité (ESP) — Stabilité électronique (ESP) défaut lamp. Valeurs : ESP_FAULT_LAMP_OFF (0), ESP_FAULT_LAMP_ON (1) |
| `ESP_espLampFlash` | — | Stabilité (ESP) — Stabilité électronique (ESP) lamp clignotement. Valeurs : ESP_LAMP_OFF (0), ESP_LAMP_FLASH (1) |
| `ESP_espModeActive` | — | Stabilité (ESP) — Stabilité électronique (ESP) mode actif. Valeurs : ESP_MODE_00_NORMAL (0), ESP_MODE_01 (1), ESP_MODE_02 (2), ESP_MODE_03 (3) |
| `ESP_hydraulicBoostEnabled` | — | Assistance hydraulique de freinage activée |
| `ESP_lateralAccelQF` | — | Qualité de la mesure d'accélération latérale (ESP). Valeurs : UNDEFINABLE_ACCURACY (0), IN_SPEC (1) |
| `ESP_longitudinalAccelQF` | — | Qualité de la mesure d'accélération longitudinale (ESP). Valeurs : UNDEFINABLE_ACCURACY (0), IN_SPEC (1) |
| `ESP_ptcTargetState` | — | État cible de la résistance chauffante PTC demandé par l'ESP. Valeurs : FAULT (0), BACKUP (1), ON (2), SNA (3) |
| `ESP_stabilityControlSts2` | — | Stabilité (ESP) — Stability contrôle sts2. Valeurs : INIT (0), ON (1), ENGAGED (2), FAULTED (3) |
| `ESP_statusChecksum` | — | Stabilité (ESP) — État somme de contrôle |
| `ESP_statusCounter` | — | Stabilité (ESP) — État compteur |
| `ESP_steeringAngleQF` | — | Stabilité (ESP) — Direction angle indicateur de qualité. Valeurs : UNDEFINABLE_ACCURACY (0), IN_SPEC (1) |
| `ESP_yawRateQF` | — | Qualité de la mesure de lacet (yaw) — ESP. Valeurs : UNDEFINABLE_ACCURACY (0), IN_SPEC (1) |

### 0x155 — WheelAngles

**Angles de rotation des 4 roues en impulsions (encodeurs)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `WheelAngleTicsRR155` | deg | Angle en impulsions de la roue arrière droite |
| `WheelAngleTicsRL155` | deg | Angle en impulsions de la roue arrière gauche |
| `WheelAngleTicsFR155` | deg | Angle en impulsions de la roue avant droite |
| `WheelAngleTicsFL155` | deg | Angle en impulsions de la roue avant gauche |
| `ESP_WheelRotationReR` | — | Stabilité (ESP) — Roue rotation re R. Valeurs : WR_FORWARD (0), WR_BACKWARD (1), WR_STANDSTILL (2), WR_NOT_DEFINABLE (3) |
| `ESP_WheelRotationReL` | — | Stabilité (ESP) — Roue rotation re L. Valeurs : WR_FORWARD (0), WR_BACKWARD (1), WR_STANDSTILL (2), WR_NOT_DEFINABLE (3) |
| `ESP_WheelRotationFrR` | — | Stabilité (ESP) — Roue rotation avant droit R. Valeurs : WR_FORWARD (0), WR_BACKWARD (1), WR_STANDSTILL (2), WR_NOT_DEFINABLE (3) |
| `ESP_WheelRotationFrL` | — | Stabilité (ESP) — Roue rotation avant droit L. Valeurs : WR_FORWARD (0), WR_BACKWARD (1), WR_STANDSTILL (2), WR_NOT_DEFINABLE (3) |
| `ESP_wheelSpeedsQF` | — | Qualité de la mesure des vitesses de roue (ESP). Valeurs : ONE_OR_MORE_WSS_INVALID (0), ALL_WSS_VALID (1) |
| `ESP_vehicleStandstillSts` | — | Stabilité (ESP) — Véhicule standstill sts. Valeurs : NOT_STANDSTILL (0), STANDSTILL (1) |
| `ESP_vehicleSpeed` | kph | Stabilité (ESP) — Véhicule vitesse. Valeurs : ESP_VEHICLE_SPEED_SNA (1023) |
| `ESP_wheelRotationCounter` | — | Stabilité (ESP) — Roue rotation compteur |
| `ESP_wheelRotationChecksum` | — | Stabilité (ESP) — Roue rotation somme de contrôle |

### 0x175 — WheelSpeed

**Vitesse individuelle des 4 roues (avant gauche/droite, arrière gauche/droite)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `WheelSpeedRR175` | KPH | Vitesse de la roue arrière droite. Valeurs : SNA (8191) |
| `WheelSpeedRL175` | KPH | Vitesse de la roue arrière gauche. Valeurs : SNA (8191) |
| `WheelSpeedFR175` | KPH | Vitesse de la roue avant droite. Valeurs : SNA (8191) |
| `WheelSpeedFL175` | KPH | Vitesse de la roue avant gauche. Valeurs : SNA (8191) |
| `ESP_wheelSpeedsCounter` | — | Compteur de séquence des vitesses de roue (ESP) |
| `ESP_wheelSpeedsChecksum` | — | Somme de contrôle des vitesses de roue (ESP) |

### 0x185 — SP_brakeTorque

**Couple de freinage appliqué par l'ESP sur chaque roue**

| Signal | Unité | Description |
|--------|-------|-------------|
| `ESP_brakeTorqueFrL` | Nm | Stabilité (ESP) — Frein couple avant droit L. Valeurs : SNA (4095) |
| `ESP_brakeTorqueFrR` | Nm | Stabilité (ESP) — Frein couple avant droit R. Valeurs : SNA (4095) |
| `ESP_brakeTorqueReL` | Nm | Stabilité (ESP) — Frein couple re L. Valeurs : SNA (4095) |
| `ESP_brakeTorqueReR` | Nm | Stabilité (ESP) — Frein couple re R. Valeurs : SNA (4095) |
| `ESP_brakeTorqueQF` | — | Stabilité (ESP) — Frein couple indicateur de qualité. Valeurs : UNDEFINABLE_ACCURACY (0), IN_SPEC (1) |
| `ESP_brakeTorqueCounter` | — | Stabilité (ESP) — Frein couple compteur |
| `ESP_brakeTorqueChecksum` | — | Stabilité (ESP) — Frein couple somme de contrôle |

### 0x20E — PARK_sdiFront

**Capteurs de distance de stationnement avant (ultrason)**

| Signal | Unité | Description |
|--------|-------|-------------|
| `PARK_sdiFrontChecksum` | — | Radar de stationnement — Capteur de distance avant somme de contrôle |
| `PARK_sdiFrontCounter` | — | Radar de stationnement — Capteur de distance avant compteur |
| `PARK_sdiSensor1RawDistData` | cm | Radar de stationnement — Capteur de distance sensor1raw distance données. Valeurs : BLOCKED (0), NEAR_DETECTION (1), NO_OBJECT_DETECTED (500), SNA (511) |
| `PARK_sdiSensor2RawDistData` | cm | Radar de stationnement — Capteur de distance sensor2raw distance données. Valeurs : BLOCKED (0), NEAR_DETECTION (1), NO_OBJECT_DETECTED (500), SNA (511) |
| `PARK_sdiSensor3RawDistData` | cm | Radar de stationnement — Capteur de distance sensor3raw distance données. Valeurs : BLOCKED (0), NEAR_DETECTION (1), NO_OBJECT_DETECTED (500), SNA (511) |
| `PARK_sdiSensor4RawDistData` | cm | Radar de stationnement — Capteur de distance sensor4raw distance données. Valeurs : BLOCKED (0), NEAR_DETECTION (1), NO_OBJECT_DETECTED (500), SNA (511) |
| `PARK_sdiSensor5RawDistData` | cm | Radar de stationnement — Capteur de distance sensor5raw distance données. Valeurs : BLOCKED (0), NEAR_DETECTION (1), NO_OBJECT_DETECTED (500), SNA (511) |
| `PARK_sdiSensor6RawDistData` | cm | Radar de stationnement — Capteur de distance sensor6raw distance données. Valeurs : BLOCKED (0), NEAR_DETECTION (1), NO_OBJECT_DETECTED (500), SNA (511) |

### 0x219 — VCSEC_TPMSData

**Capteurs de pression des pneus (TPMS) : pression, température, tension batterie, signal radio de chaque capteur**

_Aucun signal défini._

### 0x238 — UI_driverAssistMapData

**Données cartographiques pour l'aide à la conduite : courbure de la route, limitations**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_acceptBottsDots` | — | Interface — Accepté plots (route) points |
| `UI_autosteerRestricted` | — | Interface — Direction automatique (Autosteer) restricted |
| `UI_controlledAccess` | — | Accès contrôlé (route à accès réglementé) |
| `UI_countryCode` | — | Interface — Country code. Valeurs : UNKNOWN (0), SNA (1023) |
| `UI_gpsRoadMatch` | — | Correspondance GPS avec la route (map matching) |
| `UI_inSuperchargerGeofence` | — | Véhicule dans la zone géographique d'un Superchargeur |
| `UI_mapDataChecksum` | — | Interface — Carte données somme de contrôle |
| `UI_mapDataCounter` | — | Interface — Carte données compteur |
| `UI_mapSpeedLimit` | — | Interface — Carte vitesse limite. Valeurs possibles : UNKNOWN (0), LESS_OR_EQ_5 (1), LESS_OR_EQ_7 (2), LESS_OR_EQ_10 (3), LESS_OR_EQ_15 (4), LESS_OR_EQ_20 (5), LESS_OR_EQ_25 (6), LESS_OR_EQ_30 (7), LESS_OR_EQ_35 (8), LESS_OR_EQ_40 (9), LESS_OR_EQ_45 (10), LESS_OR_EQ_50 (11), LESS_OR_EQ_55 (12), LESS_OR_EQ_60 (13), LESS_OR_EQ_65 (14), LESS_OR_EQ_70 (15), LESS_OR_EQ_75 (16), LESS_OR_EQ_80 (17), LESS_OR_EQ_85 (18), LESS_OR_EQ_90 (19), LESS_OR_EQ_95 (20), LESS_OR_EQ_100 (21), LESS_OR_EQ_105 (22), LESS_OR_EQ_110 (23), LESS_OR_EQ_115 (24), LESS_OR_EQ_120 (25), LESS_OR_EQ_130 (26), LESS_OR_EQ_140 (27), LESS_OR_EQ_150 (28), LESS_OR_EQ_160 (29), UNLIMITED (30), SNA (31) |
| `UI_mapSpeedLimitDependency` | — | Interface — Carte vitesse limite dependency. Valeurs : NONE (0), SCHOOL (1), RAIN (2), SNOW (3), TIME (4), SEASON (5), LANE (6), SNA (7) |
| `UI_mapSpeedLimitType` | — | Interface — Carte vitesse limite type. Valeurs : REGULAR (1), ADVISORY (2), DEPENDENT (3), BUMPS (4), UNKNOWN_SNA (7) |
| `UI_mapSpeedUnits` | — | Interface — Carte vitesse units. Valeurs : MPH (0), KPH (1) |
| `UI_navRouteActive` | — | Interface — Navigation itinéraire actif |
| `UI_nextBranchDist` | m | Distance jusqu'à la prochaine bifurcation. Valeurs : SNA (31) |
| `UI_nextBranchLeftOffRamp` | — | Prochaine bifurcation : sortie à gauche |
| `UI_nextBranchRightOffRamp` | — | Prochaine bifurcation : sortie à droite |
| `UI_parallelAutoparkEnabled` | — | Interface — Parallel stationnement automatique activé |
| `UI_perpendicularAutoparkEnabled` | — | Interface — Perpendicular stationnement automatique activé |
| `UI_pmmEnabled` | — | Interface — Pmm activé |
| `UI_rejectAutosteer` | — | Interface — Refus direction automatique (Autosteer) |
| `UI_rejectHPP` | — | Interface — Refus HPP |
| `UI_rejectHandsOn` | — | Rejet de la détection des mains sur le volant |
| `UI_rejectLeftFreeSpace` | — | Espace libre à gauche insuffisant (rejeté) |
| `UI_rejectLeftLane` | — | Interface — Refus gauche voie |
| `UI_rejectNav` | — | Interface — Refus navigation |
| `UI_rejectRightFreeSpace` | — | Espace libre à droite insuffisant (rejeté) |
| `UI_rejectRightLane` | — | Interface — Refus droite voie |
| `UI_roadClass` | — | Classe de la route (type de voie). Valeurs : UNKNOWN_INVALID_SNA (0), CLASS_1_MAJOR (1), CLASS_2 (2), CLASS_3 (3), CLASS_4 (4), CLASS_5 (5), CLASS_6_MINOR (6) |
| `UI_scaEnabled` | — | Interface — Sca activé |
| `UI_streetCount` | — | Interface — Street compteur |

### 0x239 — S_lanes

**Détection de voies par le système de conduite autonome : lignes, marquages, confiance**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DAS_lanesCounter` | — | Conduite autonome — Lanes compteur |
| `DAS_leftFork` | — | Conduite autonome — Gauche fork. Valeurs : LEFT_FORK_NONE (0), LEFT_FORK_AVAILABLE (1), LEFT_FORK_SELECTED (2), LEFT_FORK_UNAVAILABLE (3) |
| `DAS_leftLaneExists` | — | Conduite autonome — Gauche voie exists |
| `DAS_leftLineUsage` | — | Utilisation de la ligne de voie gauche par le système de conduite. Valeurs : REJECTED_UNAVAILABLE (0), AVAILABLE (1), FUSED (2), BLACKLISTED (3) |
| `DAS_rightFork` | — | Conduite autonome — Droite fork. Valeurs : RIGHT_FORK_NONE (0), RIGHT_FORK_AVAILABLE (1), RIGHT_FORK_SELECTED (2), RIGHT_FORK_UNAVAILABLE (3) |
| `DAS_rightLaneExists` | — | Conduite autonome — Droite voie exists |
| `DAS_rightLineUsage` | — | Utilisation de la ligne de voie droite par le système de conduite. Valeurs : REJECTED_UNAVAILABLE (0), AVAILABLE (1), FUSED (2), BLACKLISTED (3) |
| `DAS_virtualLaneC0` | m | Courbure de la voie virtuelle — coefficient C0 |
| `DAS_virtualLaneC1` | rad | Courbure de la voie virtuelle — coefficient C1 |
| `DAS_virtualLaneC2` | m-1 | Courbure de la voie virtuelle — coefficient C2 |
| `DAS_virtualLaneC3` | m-2 | Courbure de la voie virtuelle — coefficient C3 |
| `DAS_virtualLaneViewRange` | m | Portée de vision de la voie virtuelle |
| `DAS_virtualLaneWidth` | m | Largeur de la voie virtuelle |

### 0x24A — S_visualDebug

**Débogage visuel du système de conduite autonome**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DAS_accSmartSpeedActive` | — | Régulateur adaptatif intelligent (vitesse contextuelle) actif |
| `DAS_accSmartSpeedState` | — | État du régulateur adaptatif intelligent. Valeurs : NOT_ACTIVE (0), ACTIVE_OFFRAMP (1), ACTIVE_INTEGRATING (2), ACTIVE_ONRAMP (3), SET_SPEED_SET_REQUESTED (4), OFFRAMP_DELAY (5), SNA (7) |
| `DAS_autosteerBottsDotsUsage` | — | Utilisation des plots au sol (Botts' dots) par l'Autosteer. Valeurs : REJECTED_UNAVAILABLE (0), AVAILABLE (1), FUSED (2), BLACKLISTED (3) |
| `DAS_autosteerHPPUsage` | — | Utilisation de la prédiction de trajectoire (HPP) par l'Autosteer. Valeurs : REJECTED_UNAVAILABLE (0), AVAILABLE (1), FUSED (2), BLACKLISTED (3) |
| `DAS_autosteerHealthAnomalyLevel` | — | Niveau d'anomalie de santé de l'Autosteer |
| `DAS_autosteerHealthState` | — | Conduite autonome — Direction automatique (Autosteer) health état. Valeurs : HEALTH_UNAVAILABLE (0), HEALTH_NOMINAL (1), HEALTH_DEGRADED (2), HEALTH_SEVERELY_DEGRADED (3), HEALTH_ABORTING (4), HEALTH_FAULT (5) |
| `DAS_autosteerModelUsage` | — | Utilisation du modèle de conduite par l'Autosteer. Valeurs : REJECTED_UNAVAILABLE (0), AVAILABLE (1), FUSED (2), BLACKLISTED (3) |
| `DAS_autosteerNavigationUsage` | — | Utilisation de la navigation par l'Autosteer. Valeurs : REJECTED_UNAVAILABLE (0), AVAILABLE (1), FUSED (2), BLACKLISTED (3) |
| `DAS_autosteerVehiclesUsage` | — | Utilisation de la détection des véhicules par l'Autosteer. Valeurs : REJECTED_UNAVAILABLE (0), AVAILABLE (1), FUSED (2), BLACKLISTED (3) |
| `DAS_behaviorType` | — | Type de comportement du système de conduite autonome. Valeurs : DAS_BEHAVIOR_INVALID (0), DAS_BEHAVIOR_IN_LANE (1), DAS_BEHAVIOR_LANE_CHANGE_LEFT (2), DAS_BEHAVIOR_LANE_CHANGE_RIGHT (3) |
| `DAS_devAppInterfaceEnabled` | — | Conduite autonome — Dev application interface activé |
| `DAS_lastAutosteerAbortReason` | — | Dernière raison d'abandon de la direction automatique (Autosteer). Valeurs possibles : UI_ABORT_REASON_HM_LANE_VIEW_RANGE (0), UI_ABORT_REASON_HM_VIRTUAL_LANE_NO_INPUTS (1), UI_ABORT_REASON_HM_STEERING_ERROR (2), UI_ABORT_REASON_APP_ME_STATE_NOT_VISION (14), UI_ABORT_REASON_ME_MAIN_STATE_NOT_VISION (15), UI_ABORT_REASON_CAM_MSG_MIA (16), UI_ABORT_REASON_CAM_WATCHDOG (17), UI_ABORT_REASON_TRAILER_MODE (18), UI_ABORT_REASON_SIDE_COLLISION_IMMINENT (19), UI_ABORT_REASON_EPAS_EAC_DENIED (20), UI_ABORT_REASON_COMPONENT_MIA (21), UI_ABORT_REASON_CRUISE_FAULT (22), UI_ABORT_REASON_CID_SWITCH_DISABLED (23), UI_ABORT_REASON_DRIVING_OFF_NAV (24), UI_ABORT_REASON_VEHICLE_SPEED_ABOVE_MAX (25), UI_ABORT_REASON_FOLLOWER_OUTPUT_INVALID (26), UI_ABORT_REASON_PLANNER_OUTPUT_INVALID (27), UI_ABORT_REASON_EPAS_ERROR_CODE (28), UI_ABORT_REASON_ACC_CANCEL (29), UI_ABORT_REASON_CAMERA_FAILSAFES (30), UI_ABORT_REASON_NO_ABORT (31), UI_ABORT_REASON_AEB (32), UI_ABORT_REASON_SEATBELT_UNBUCKLED (33), UI_ABORT_REASON_USER_OVERRIDE_STRIKEOUT (34) |
| `DAS_lastLinePreferenceReason` | — | Dernière raison de préférence de ligne de voie. Valeurs : OTHER_LANE_DISAGREES_WITH_MODEL (0), AGREEMENT_WITH_NEIGHBOR_LANES (1), NEIGHBOR_LANE_PROBABILIY (2), NAVIGATION_BRANCH (3), AVOID_ONCOMING_LANES (4), COUNTRY_DRIVING_SIDE (5), NONE (15) |
| `DAS_navAvailable` | — | Conduite autonome — Navigation disponible. Valeurs : DAS_NAV_UNAVAILABLE (0), DAS_NAV_AVAILABLE (1) |
| `DAS_navDistance` | km | Conduite autonome — Navigation distance |
| `DAS_offsetSide` | — | Conduite autonome — Décalage côté. Valeurs : NO_OFFSET (0), OFFSET_RIGHT_OBJECT (1), OFFSET_LEFT_OBJECT (2), OFFSET_BOTH_OBJECTS (3) |
| `DAS_plannerState` | — | État du planificateur de trajectoire. Valeurs : TP_EXTSTATE_DISABLED (0), TP_EXTSTATE_VL (1), TP_EXTSTATE_FOLLOW (2), TP_EXTSTATE_LANECHANGE_REQUESTED (3), TP_EXTSTATE_LANECHANGE_IN_PROGRESS (4), TP_EXTSTATE_LANECHANGE_WAIT_FOR_SIDE_OBSTACLE (5), TP_EXTSTATE_LANECHANGE_WAIT_FOR_FWD_OBSTACLE (6), TP_EXTSTATE_LANECHANGE_ABORT (7) |
| `DAS_rearLeftVehDetectedCurrent` | — | Conduite autonome — Arrière gauche véhicule détecté courant. Valeurs : VEHICLE_NOT_DETECTED (0), VEHICLE_DETECTED (1) |
| `DAS_rearLeftVehDetectedTrip` | — | Conduite autonome — Arrière gauche véhicule détecté trajet. Valeurs : VEHICLE_NOT_DETECTED (0), VEHICLE_DETECTED (1) |
| `DAS_rearRightVehDetectedTrip` | — | Conduite autonome — Arrière droite véhicule détecté trajet. Valeurs : VEHICLE_NOT_DETECTED (0), VEHICLE_DETECTED (1) |
| `DAS_rearVehDetectedThisCycle` | — | Véhicule arrière détecté lors de ce cycle. Valeurs : VEHICLE_NOT_DETECTED (0), VEHICLE_DETECTED (1) |
| `DAS_roadSurfaceType` | — | Conduite autonome — Route surface type. Valeurs : ROAD_SURFACE_SNA (0), ROAD_SURFACE_NORMAL (1), ROAD_SURFACE_ENHANCED (2) |
| `DAS_ulcInProgress` | — | Changement de voie automatique en cours. Valeurs : ULC_INACTIVE (0), ULC_ACTIVE (1) |
| `DAS_ulcType` | — | Type de changement de voie automatique (ULC). Valeurs : ULC_TYPE_NONE (0), ULC_TYPE_NAV (1), ULC_TYPE_SPEED (2) |

### 0x25B — PP_environment

**Conditions environnementales détectées par le véhicule : pluie, neige**

| Signal | Unité | Description |
|--------|-------|-------------|
| `APP_environmentRainy` | — | Environnement détecté pluvieux |
| `APP_environmentSnowy` | — | Environnement détecté enneigé |

### 0x2B9 — S_control

**Commandes du système de conduite autonome : direction, accélération, freinage**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DAS_accState` | — | État du régulateur de vitesse adaptatif (ACC). Valeurs possibles : ACC_CANCEL_GENERIC (0), ACC_CANCEL_CAMERA_BLIND (1), ACC_CANCEL_RADAR_BLIND (2), ACC_HOLD (3), ACC_ON (4), APC_BACKWARD (5), APC_FORWARD (6), APC_COMPLETE (7), APC_ABORT (8), APC_PAUSE (9), APC_UNPARK_COMPLETE (10), APC_SELFPARK_START (11), ACC_CANCEL_PATH_NOT_CLEAR (12), ACC_CANCEL_GENERIC_SILENT (13), ACC_CANCEL_OUT_OF_CALIBRATION (14), FAULT_SNA (15) |
| `DAS_accelMax` | m/s^2 | Conduite autonome — Accélération maximum. Valeurs : SNA (511) |
| `DAS_accelMin` | m/s^2 | Conduite autonome — Accélération minimum. Valeurs : SNA (511) |
| `DAS_aebEvent` | — | Événement de freinage automatique d'urgence (AEB). Valeurs : AEB_NOT_ACTIVE (0), AEB_ACTIVE (1), AEB_FAULT (2), AEB_SNA (3) |
| `DAS_controlChecksum` | — | Conduite autonome — Contrôle somme de contrôle |
| `DAS_controlCounter` | — | Conduite autonome — Contrôle compteur |
| `DAS_jerkMax` | m/s^3 | Conduite autonome — Jerk maximum. Valeurs : SNA (255) |
| `DAS_jerkMin` | m/s^3 | Conduite autonome — Jerk minimum. Valeurs : SNA (511) |
| `DAS_setSpeed` | kph | Conduite autonome — Set vitesse. Valeurs : SNA (4095) |

### 0x2D3 — UI_solarData

**Données solaires : charge solaire mesurée, luminosité**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_isSunUp` | — | Indicateur de jour/nuit (soleil levé ou couché). Valeurs : SUN_DOWN (0), SUN_UP (1), SUN_SNA (3) |
| `UI_minsToSunrise` | min | Minutes restantes avant le lever du soleil |
| `UI_minsToSunset` | min | Minutes restantes avant le coucher du soleil |
| `UI_screenPCBTemperature` | C | Interface — Écran carte électronique température |
| `UI_solarAzimuthAngle` | deg | Interface — Solar azimuth angle. Valeurs : SNA (32768) |
| `UI_solarAzimuthAngleCarRef` | deg | Interface — Solar azimuth angle véhicule ref. Valeurs : SNA (255) |
| `UI_solarElevationAngle` | deg | Interface — Solar altitude angle. Valeurs : SNA (127) |

### 0x31F — TPMSsensors

**Données brutes des capteurs de pression des pneus (TPMS)** | Cycle : 1000 ms

| Signal | Unité | Description |
|--------|-------|-------------|
| `TPMSFLpressure31F` | bar | TPMSF lpressure31f |
| `TPMSFRpressure31F` | bar | TPMSF rpressure31f |
| `TPMSRLpressure31F` | bar | TPMSR lpressure31f |
| `TPMSRRpressure31F` | bar | TPMSR rpressure31f |
| `TPMSFLtemp31F` | C | TPMSF ltemp31f |
| `TPMSFRtemp31F` | C | TPMSF rtemp31f |
| `TPMSRLtemp31F` | C | TPMSR ltemp31f |
| `TPMSRRtemp31F` | C | TPMSR rtemp31f |

### 0x389 — S_status2

**État du système de conduite autonome (partie 2) : alertes, modes, confiance**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DAS_ACC_report` | — | Rapport du régulateur de vitesse adaptatif (ACC). Valeurs possibles : ACC_REPORT_TARGET_NONE (0), ACC_REPORT_TARGET_CIPV (1), ACC_REPORT_TARGET_IN_FRONT_OF_CIPV (2), ACC_REPORT_TARGET_MCVL (3), ACC_REPORT_TARGET_MCVR (4), ACC_REPORT_TARGET_CUTIN (5), ACC_REPORT_TARGET_TYPE_STOP_SIGN (6), ACC_REPORT_TARGET_TYPE_TRAFFIC_LIGHT (7), ACC_REPORT_TARGET_TYPE_IPSO (8), ACC_REPORT_TARGET_TYPE_FAULT (9), ACC_REPORT_CSA (10), ACC_REPORT_LC_HANDS_ON_REQD_STRUCK_OUT (11), ACC_REPORT_LC_EXTERNAL_STATE_ABORTING (12), ACC_REPORT_LC_EXTERNAL_STATE_ABORTED (13), ACC_REPORT_LC_EXTERNAL_STATE_ACTIVE_RESTRICTED (14), ACC_REPORT_RADAR_OBJ_ONE (15), ACC_REPORT_RADAR_OBJ_TWO (16), ACC_REPORT_TARGET_MCP (17), ACC_REPORT_FLEET_SPEEDS (18), ACC_REPORT_MCVLR_DPP (19), ACC_REPORT_MCVLR_IN_PATH (20), ACC_REPORT_CIPV_CUTTING_OUT (21), ACC_REPORT_RADAR_OBJ_FIVE (22), ACC_REPORT_CAMERA_ONLY (23), ACC_REPORT_BEHAVIOR_REPORT (24) |
| `DAS_accSpeedLimit` | mph | Limite de vitesse du régulateur adaptatif. Valeurs : NONE (0), SNA (1023) |
| `DAS_activationFailureStatus` | — | Conduite autonome — Activation failure état. Valeurs : LC_ACTIVATION_IDLE (0), LC_ACTIVATION_FAILED_1 (1), LC_ACTIVATION_FAILED_2 (2) |
| `DAS_csaState` | — | Conduite autonome — Csa état. Valeurs : CSA_EXTERNAL_STATE_UNAVAILABLE (0), CSA_EXTERNAL_STATE_AVAILABLE (1), CSA_EXTERNAL_STATE_ENABLE (2), CSA_EXTERNAL_STATE_HOLD (3) |
| `DAS_driverInteractionLevel` | — | Conduite autonome — Conducteur interaction niveau. Valeurs : DRIVER_INTERACTING (0), DRIVER_NOT_INTERACTING (1), CONTINUED_DRIVER_NOT_INTERACTING (2) |
| `DAS_longCollisionWarning` | — | Conduite autonome — Long collision alerte. Valeurs possibles : FCM_LONG_COLLISION_WARNING_NONE (0), FCM_LONG_COLLISION_WARNING_VEHICLE_UNKNOWN (1), FCM_LONG_COLLISION_WARNING_PEDESTRIAN (2), FCM_LONG_COLLISION_WARNING_IPSO (3), FCM_LONG_COLLISION_WARNING_STOPSIGN_STOPLINE (4), FCM_LONG_COLLISION_WARNING_TFL_STOPLINE (5), FCM_LONG_COLLISION_WARNING_VEHICLE_CIPV (6), FCM_LONG_COLLISION_WARNING_VEHICLE_CUTIN (7), FCM_LONG_COLLISION_WARNING_VEHICLE_MCVL (8), FCM_LONG_COLLISION_WARNING_VEHICLE_MCVL2 (9), FCM_LONG_COLLISION_WARNING_VEHICLE_MCVR (10), FCM_LONG_COLLISION_WARNING_VEHICLE_MCVR2 (11), FCM_LONG_COLLISION_WARNING_VEHICLE_CIPV2 (12), FCM_LONG_COLLISION_WARNING_SNA (15) |
| `DAS_pmmCameraFaultReason` | — | Conduite autonome — Pmm caméra défaut raison. Valeurs : PMM_CAMERA_NO_FAULT (0), PMM_CAMERA_BLOCKED_FRONT (1), PMM_CAMERA_INVALID_MIA (2) |
| `DAS_pmmLoggingRequest` | — | Demande d'enregistrement PMM (conduite autonome). Valeurs : FALSE (0), TRUE (1) |
| `DAS_pmmObstacleSeverity` | — | Conduite autonome — Pmm obstacle severity. Valeurs : PMM_NONE (0), PMM_IMMINENT_REAR (1), PMM_IMMINENT_FRONT (2), PMM_BRAKE_REQUEST (3), PMM_CRASH_REAR (4), PMM_CRASH_FRONT (5), PMM_ACCEL_LIMIT (6), PMM_SNA (7) |
| `DAS_pmmRadarFaultReason` | — | Conduite autonome — Pmm radar défaut raison. Valeurs : PMM_RADAR_NO_FAULT (0), PMM_RADAR_BLOCKED_FRONT (1), PMM_RADAR_INVALID_MIA (2) |
| `DAS_pmmSysFaultReason` | — | Conduite autonome — Pmm sys défaut raison. Valeurs : PMM_FAULT_NONE (0), PMM_FAULT_DAS_DISABLED (1), PMM_FAULT_SPEED (2), PMM_FAULT_DI_FAULT (3), PMM_FAULT_STEERING_ANGLE_RATE (4), PMM_FAULT_DISABLED_BY_USER (5), PMM_FAULT_ROAD_TYPE (6), PMM_FAULT_BRAKE_PEDAL_INHIBIT (7) |
| `DAS_pmmUltrasonicsFaultReason` | — | Conduite autonome — Pmm ultrasonics défaut raison. Valeurs : PMM_ULTRASONICS_NO_FAULT (0), PMM_ULTRASONICS_BLOCKED_FRONT (1), PMM_ULTRASONICS_BLOCKED_REAR (2), PMM_ULTRASONICS_BLOCKED_BOTH (3), PMM_ULTRASONICS_INVALID_MIA (4) |
| `DAS_ppOffsetDesiredRamp` | m | Décalage de pédale souhaité (rampe) pour la conduite autonome. Valeurs : PP_NO_OFFSET (128) |
| `DAS_radarTelemetry` | — | Télémétrie radar du système de conduite autonome. Valeurs : RADAR_TELEMETRY_IDLE (0), RADAR_TELEMETRY_NORMAL (1), RADAR_TELEMETRY_URGENT (2) |
| `DAS_relaxCruiseLimits` | — | Conduite autonome — Relax régulateur de vitesse limites |
| `DAS_robState` | — | Conduite autonome — Rob état. Valeurs : ROB_STATE_INHIBITED (0), ROB_STATE_MEASURE (1), ROB_STATE_ACTIVE (2), ROB_STATE_MAPLESS (3) |
| `DAS_status2Checksum` | — | Conduite autonome — Status2checksum |
| `DAS_status2Counter` | — | Conduite autonome — Status2counter |

### 0x399 — S_status

**État du système de conduite autonome : Autopilot, régulateur adaptatif, alertes**

| Signal | Unité | Description |
|--------|-------|-------------|
| `DAS_autoLaneChangeState` | — | État du changement de voie automatique. Valeurs possibles : ALC_UNAVAILABLE_DISABLED (0), ALC_UNAVAILABLE_NO_LANES (1), ALC_UNAVAILABLE_SONICS_INVALID (2), ALC_UNAVAILABLE_TP_FOLLOW (3), ALC_UNAVAILABLE_EXITING_HIGHWAY (4), ALC_UNAVAILABLE_VEHICLE_SPEED (5), ALC_AVAILABLE_ONLY_L (6), ALC_AVAILABLE_ONLY_R (7), ALC_AVAILABLE_BOTH (8), ALC_IN_PROGRESS_L (9), ALC_IN_PROGRESS_R (10), ALC_WAITING_FOR_SIDE_OBST_TO_PASS_L (11), ALC_WAITING_FOR_SIDE_OBST_TO_PASS_R (12), ALC_WAITING_FOR_FWD_OBST_TO_PASS_L (13), ALC_WAITING_FOR_FWD_OBST_TO_PASS_R (14), ALC_ABORT_SIDE_OBSTACLE_PRESENT_L (15), ALC_ABORT_SIDE_OBSTACLE_PRESENT_R (16), ALC_ABORT_POOR_VIEW_RANGE (17), ALC_ABORT_LC_HEALTH_BAD (18), ALC_ABORT_BLINKER_TURNED_OFF (19), ALC_ABORT_OTHER_REASON (20), ALC_UNAVAILABLE_SOLID_LANE_LINE (21), ALC_BLOCKED_VEH_TTC_L (22), ALC_BLOCKED_VEH_TTC_AND_USS_L (23), ALC_BLOCKED_VEH_TTC_R (24), ALC_BLOCKED_VEH_TTC_AND_USS_R (25), ALC_BLOCKED_LANE_TYPE_L (26), ALC_BLOCKED_LANE_TYPE_R (27), ALC_WAITING_HANDS_ON (28), ALC_ABORT_TIMEOUT (29), ALC_ABORT_MISSION_PLAN_INVALID (30), ALC_SNA (31) |
| `DAS_autoParked` | — | Conduite autonome — Automatique parked |
| `DAS_autoparkReady` | — | Conduite autonome — Stationnement automatique prêt. Valeurs : AUTOPARK_UNAVAILABLE (0), AUTOPARK_READY (1) |
| `DAS_autoparkWaitingForBrake` | — | Le stationnement automatique attend l'appui sur le frein |
| `DAS_autopilotHandsOnState` | — | État de la détection des mains sur le volant (Autopilot). Valeurs possibles : LC_HANDS_ON_NOT_REQD (0), LC_HANDS_ON_REQD_DETECTED (1), LC_HANDS_ON_REQD_NOT_DETECTED (2), LC_HANDS_ON_REQD_VISUAL (3), LC_HANDS_ON_REQD_CHIME_1 (4), LC_HANDS_ON_REQD_CHIME_2 (5), LC_HANDS_ON_REQD_SLOWING (6), LC_HANDS_ON_REQD_STRUCK_OUT (7), LC_HANDS_ON_SUSPENDED (8), LC_HANDS_ON_REQD_ESCALATED_CHIME_1 (9), LC_HANDS_ON_REQD_ESCALATED_CHIME_2 (10), LC_HANDS_ON_SNA (15) |
| `DAS_autopilotState` | — | Conduite autonome — Autopilot état. Valeurs possibles : DISABLED (0), UNAVAILABLE (1), AVAILABLE (2), ACTIVE_NOMINAL (3), ACTIVE_RESTRICTED (4), ACTIVE_NAV (5), ABORTING (8), ABORTED (9), FAULT (14), SNA (15) |
| `DAS_blindSpotRearLeft` | — | Alerte d'angle mort arrière gauche. Valeurs : NO_WARNING (0), WARNING_LEVEL_1 (1), WARNING_LEVEL_2 (2), SNA (3) |
| `DAS_blindSpotRearRight` | — | Alerte d'angle mort arrière droite. Valeurs : NO_WARNING (0), WARNING_LEVEL_1 (1), WARNING_LEVEL_2 (2), SNA (3) |
| `DAS_fleetSpeedState` | — | Conduite autonome — Fleet vitesse état. Valeurs : FLEETSPEED_UNAVAILABLE (0), FLEETSPEED_AVAILABLE (1), FLEETSPEED_ACTIVE (2), FLEETSPEED_HOLD (3) |
| `DAS_forwardCollisionWarning` | — | Alerte de collision frontale. Valeurs : NONE (0), FORWARD_COLLISION_WARNING (1), SNA (3) |
| `DAS_fusedSpeedLimit` | kph/mph | Limite de vitesse fusionnée (combinaison caméra + carte). Valeurs : UNKNOWN_SNA (0), NONE (31) |
| `DAS_heaterState` | — | Conduite autonome — Résistance de chauffage état. Valeurs : HEATER_OFF_SNA (0), HEATER_ON (1) |
| `DAS_laneDepartureWarning` | — | Conduite autonome — Voie départ alerte. Valeurs : NONE (0), LEFT_WARNING (1), RIGHT_WARNING (2), LEFT_WARNING_SEVERE (3), RIGHT_WARNING_SEVERE (4), SNA (5) |
| `DAS_lssState` | — | État du système de maintien dans la voie (LSS). Valeurs : LSS_STATE_FAULT (0), LSS_STATE_LDW (1), LSS_STATE_LKA (2), LSS_STATE_ELK (3), LSS_STATE_MONITOR (4), LSS_STATE_BLINDSPOT (5), LSS_STATE_ABORT (6), LSS_STATE_OFF (7) |
| `DAS_sideCollisionAvoid` | — | Conduite autonome — Côté collision évitement. Valeurs : NONE (0), AVOID_LEFT (1), AVOID_RIGHT (2), SNA (3) |
| `DAS_sideCollisionInhibit` | — | Inhibition de la protection anti-collision latérale. Valeurs : NO_INHIBIT (0), INHIBIT (1) |
| `DAS_sideCollisionWarning` | — | Conduite autonome — Côté collision alerte. Valeurs : NONE (0), WARN_LEFT (1), WARN_RIGHT (2), WARN_LEFT_RIGHT (3) |
| `DAS_statusChecksum` | — | Conduite autonome — État somme de contrôle |
| `DAS_statusCounter` | — | Conduite autonome — État compteur |
| `DAS_summonAvailable` | — | Convocation (Summon) disponible |
| `DAS_summonClearedGate` | — | Passage dégagé validé pour la convocation (Summon) |
| `DAS_summonFwdLeashReached` | — | Distance maximale avant atteinte pour la convocation (Summon) |
| `DAS_summonObstacle` | — | Obstacle détecté lors de la convocation (Summon) |
| `DAS_summonRvsLeashReached` | — | Distance maximale arrière atteinte pour la convocation (Summon) |
| `DAS_suppressSpeedWarning` | — | Conduite autonome — Suppress vitesse alerte. Valeurs : Do_Not_Suppress (0), Suppress_Speed_Warning (1) |
| `DAS_visionOnlySpeedLimit` | kph/mph | Limite de vitesse basée uniquement sur la vision (caméras). Valeurs : UNKNOWN_SNA (0), NONE (31) |

### 0x39D — IBST_status

**État du freinage iBooster : pression, mode, assistance d'urgence**

| Signal | Unité | Description |
|--------|-------|-------------|
| `IBST_driverBrakeApply` | — | Freinage iBooster — Conducteur frein application. Valeurs : NOT_INIT_OR_OFF (0), BRAKES_NOT_APPLIED (1), DRIVER_APPLYING_BRAKES (2), FAULT (3) |
| `IBST_iBoosterStatus` | — | Freinage iBooster — I assistance de freinage état. Valeurs : IBOOSTER_OFF (0), IBOOSTER_INIT (1), IBOOSTER_FAILURE (2), IBOOSTER_DIAGNOSTIC (3), IBOOSTER_ACTIVE_GOOD_CHECK (4), IBOOSTER_READY (5), IBOOSTER_ACTUATION (6) |
| `IBST_internalState` | — | Freinage iBooster — Interne état. Valeurs : NO_MODE_ACTIVE (0), PRE_DRIVE_CHECK (1), LOCAL_BRAKE_REQUEST (2), EXTERNAL_BRAKE_REQUEST (3), DIAGNOSTIC (4), TRANSITION_TO_IDLE (5), POST_DRIVE_CHECK (6) |
| `IBST_sInputRodDriver` | mm | Freinage iBooster — S entrée rod conducteur |
| `IBST_statusChecksum` | — | Freinage iBooster — État somme de contrôle |
| `IBST_statusCounter` | — | Freinage iBooster — État compteur |

### 0x3D9 — UI_gpsVehicleSpeed

**Vitesse du véhicule mesurée par le GPS**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_conditionalLimitActive` | — | Interface — Conditionnel limite actif |
| `UI_conditionalSpeedLimit` | kph/mph | Interface — Conditionnel vitesse limite. Valeurs : SNA (31) |
| `UI_gpsAntennaDisconnected` | — | Interface — GPS antenne déconnecté |
| `UI_gpsHDOP` | — | Interface — GPS HDOP |
| `UI_gpsNmeaMIA` | — | Interface — GPS nmea absent (MIA) |
| `UI_gpsVehicleHeading` | deg | Interface — GPS véhicule heading |
| `UI_gpsVehicleSpeed` | kph | Interface — GPS véhicule vitesse |
| `UI_mapSpeedLimitUnits` | — | Interface — Carte vitesse limite units. Valeurs : MPH (0), KPH (1) |
| `UI_mppSpeedLimit` | kph/mph | Interface — Mpp vitesse limite |
| `UI_userSpeedOffset` | kph/mph | Interface — User vitesse décalage |
| `UI_userSpeedOffsetUnits` | — | Unité du décalage de vitesse utilisateur. Valeurs : MPH (0), KPH (1) |

### 0x3F3 — UI_odo

**Odomètre affiché par l'interface utilisateur**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_odometer` | km | Interface — Compteur kilométrique. Valeurs : SNA (16777215) |

### 0x3F8 — UI_driverAssistControl

**Commandes de l'aide à la conduite : régulateur, maintien dans la voie, limites**

| Signal | Unité | Description |
|--------|-------|-------------|
| `UI_accFollowDistanceSetting` | — | Réglage de la distance de suivi du régulateur adaptatif. Valeurs : DISTANCE_SETTING_1 (0), DISTANCE_SETTING_2 (1), DISTANCE_SETTING_3 (2), DISTANCE_SETTING_4 (3), DISTANCE_SETTING_5 (4), DISTANCE_SETTING_6 (5), DISTANCE_SETTING_7 (6), DISTANCE_SETTING_SNA (7) |
| `UI_adaptiveSetSpeedEnable` | — | Activation de la vitesse cible adaptative |
| `UI_alcOffHighwayEnable` | — | Changement de voie automatique hors autoroute (activé/désactivé) |
| `UI_autoSummonEnable` | — | Activation de la convocation automatique (Summon) |
| `UI_autopilotControlRequest` | — | Demande du type de contrôle Autopilot. Valeurs : LEGACY_LAT_CTRL (0), NEXT_GEN_CTRL (1) |
| `UI_coastToCoast` | — | Mode trajet longue distance (coast-to-coast) |
| `UI_curvSpeedAdaptDisable` | — | Désactivation de l'adaptation de vitesse en courbe. Valeurs : CSA_ON (0), CSA_OFF (1) |
| `UI_curvatureDatabaseOnly` | — | Courbure de la route basée uniquement sur la base de données. Valeurs : OFF (0), ON (1) |
| `UI_dasDeveloper` | — | Mode développeur de la conduite autonome |
| `UI_driveOnMapsEnable` | — | Activation de la conduite assistée sur les routes cartographiées. Valeurs : DOM_OFF (0), DOM_ON (1) |
| `UI_drivingSide` | — | Côté de conduite (gauche ou droite). Valeurs : DRIVING_SIDE_LEFT (0), DRIVING_SIDE_RIGHT (1), DRIVING_SIDE_UNKNOWN (2) |
| `UI_enableBrakeLightPulse` | — | Activation du clignotement des feux de freinage (freinage d'urgence) |
| `UI_enableClipTelemetry` | — | Activation de la télémétrie des événements vidéo |
| `UI_enableRoadSegmentTelemetry` | — | Activation de la télémétrie par segment de route |
| `UI_enableTripTelemetry` | — | Activation de la télémétrie de trajet |
| `UI_enableVinAssociation` | — | Activation de l'association du numéro VIN |
| `UI_enableVisionOnlyStops` | — | Activation des arrêts basés uniquement sur la vision (caméras) |
| `UI_exceptionListEnable` | — | Activation de la liste d'exceptions. Valeurs : EXCEPTION_LIST_OFF (0), EXCEPTION_LIST_ON (1) |
| `UI_followNavRouteEnable` | — | Activation du suivi automatique de l'itinéraire de navigation. Valeurs : NAV_ROUTE_OFF (0), NAV_ROUTE_ON (1) |
| `UI_fuseHPPDisable` | — | Désactivation de la fusion HPP (prédiction de trajectoire). Valeurs : FUSE_HPP_ON (0), FUSE_HPP_OFF (1) |
| `UI_fuseLanesDisable` | — | Désactivation de la fusion des voies détectées. Valeurs : FUSE_LANES_ON (0), FUSE_LANES_OFF (1) |
| `UI_fuseVehiclesDisable` | — | Désactivation de la fusion des véhicules détectés. Valeurs : FUSE_VEH_ON (0), FUSE_VEH_OFF (1) |
| `UI_handsOnRequirementDisable` | — | Désactivation de l'obligation de garder les mains sur le volant. Valeurs : HANDS_ON_REQ_ON (0), HANDS_ON_REQ_OFF (1) |
| `UI_hasDriveOnNav` | — | Navigation avec conduite assistée disponible |
| `UI_lssElkEnabled` | — | Système anti-collision d'urgence (ELK) activé. Valeurs : ELK_OFF (0), ELK_ON (1) |
| `UI_lssLdwEnabled` | — | Avertissement de sortie de voie (LDW) activé. Valeurs : LDW_OFF (0), LDW_ON (1) |
| `UI_lssLkaEnabled` | — | Assistance au maintien dans la voie (LKA) activée. Valeurs : LKA_OFF (0), LKA_ON (1) |
| `UI_roadCheckDisable` | — | Désactivation de la vérification de l'état de la route. Valeurs : RC_ON (0), RC_OFF (1) |
| `UI_selfParkRequest` | — | Demande de stationnement automatique. Valeurs possibles : NONE (0), SELF_PARK_FORWARD (1), SELF_PARK_REVERSE (2), ABORT (3), PRIME (4), PAUSE (5), RESUME (6), AUTO_SUMMON_FORWARD (7), AUTO_SUMMON_REVERSE (8), AUTO_SUMMON_CANCEL (9), AUTO_SUMMON_PRIMED (10), SMART_SUMMON (11), SMART_SUMMON_NO_OP (12), SNA (15) |
| `UI_smartSummonType` | — | Type de convocation intelligente (Summon). Valeurs : PIN_DROP (0), FIND_ME (1), SMART_AUTOPARK (2) |
| `UI_source3D` | — | Source des données 3D. Valeurs : Z_FROM_MAP (0), Z_FROM_PATH_PREDICTION (1), XYZ_PREDICTION (2) |
| `UI_summonEntryType` | — | Type de manœuvre d'entrée pour la convocation (Summon). Valeurs : STRAIGHT (0), TURN_RIGHT (1), TURN_LEFT (2), SNA (3) |
| `UI_summonExitType` | — | Type de manœuvre de sortie pour la convocation (Summon). Valeurs : STRAIGHT (0), TURN_RIGHT (1), TURN_LEFT (2), SNA (3) |
| `UI_summonHeartbeat` | — | Signal de vie (heartbeat) de la convocation Summon |
| `UI_summonReverseDist` | — | Distance de marche arrière pour la convocation Summon. Valeurs : SNA (63) |
| `UI_ulcBlindSpotConfig` | — | Configuration des angles morts pour le changement de voie automatique. Valeurs : STANDARD (0), AGGRESSIVE (1), MAD_MAX (2) |
| `UI_ulcOffHighway` | — | Changement de voie automatique hors autoroute |
| `UI_ulcSpeedConfig` | — | Configuration de vitesse pour le changement de voie automatique. Valeurs : SPEED_BASED_ULC_DISABLED (0), SPEED_BASED_ULC_MILD (1), SPEED_BASED_ULC_AVERAGE (2), SPEED_BASED_ULC_MAD_MAX (3) |
| `UI_ulcStalkConfirm` | — | Confirmation du changement de voie par le commodo |
| `UI_undertakeAssistEnable` | — | Activation de l'assistance au dépassement par la droite |
| `UI_validationLoop` | — | Boucle de validation du système |
| `UI_visionSpeedType` | — | Type de détection de vitesse par vision (caméras). Valeurs : VISION_SPEED_DISABLED (0), VISION_SPEED_ONE_SECOND (1), VISION_SPEED_TWO_SECOND (2), VISION_SPEED_OPTIMIZED (3) |

### 0x3FD — UI_autopilotControl

**Commandes de l'Autopilot : activation, désactivation, vitesse cible, distance de suivi**

_Aucun signal défini._
