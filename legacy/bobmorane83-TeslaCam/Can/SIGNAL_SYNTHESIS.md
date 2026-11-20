# Tesla Model 3 — Synthèse Exhaustive des Signaux CAN

> **Date :** 11 avril 2026
> **Sources analysées :**
> - `tesla_can.dbc` (159 messages, ~2 752 signaux) — Base principale Model 3
> - `Model3CAN.dbc` — Copie identique de `tesla_can.dbc`
> - `tesla_can2.dbc` (19 messages, 210 signaux) — Base openpilot / commaai
> - `Tesla-Model 3.md` — Référence Racelogic VBOX (signaux simplifiés)

---

## Table des matières

1. [Vue d'ensemble](#1-vue-densemble)
2. [Nœuds CAN (ECUs)](#2-nœuds-can-ecus)
3. [Bus CAN](#3-bus-can)
4. [Signaux par catégorie fonctionnelle](#4-signaux-par-catégorie-fonctionnelle)
   - 4.1 [Dynamique véhicule (IMU / Inertiel)](#41-dynamique-véhicule-imu--inertiel)
   - 4.2 [Direction (EPAS / Steering)](#42-direction-epas--steering)
   - 4.3 [Groupe motopropulseur (Drive Inverter)](#43-groupe-motopropulseur-drive-inverter)
   - 4.4 [Freinage (ESP / iBooster / EPB)](#44-freinage-esp--ibooster--epb)
   - 4.5 [Vitesse des roues](#45-vitesse-des-roues)
   - 4.6 [Batterie et BMS](#46-batterie-et-bms)
   - 4.7 [Charge (AC / DC / PCS)](#47-charge-ac--dc--pcs)
   - 4.8 [Haute tension (HVP / Contacteurs)](#48-haute-tension-hvp--contacteurs)
   - 4.9 [Carrosserie (Portes / Coffre / Vitres)](#49-carrosserie-portes--coffre--vitres)
   - 4.10 [Éclairage](#410-éclairage)
   - 4.11 [TPMS (Pression / Température pneus)](#411-tpms-pression--température-pneus)
   - 4.12 [Environnement et capteurs](#412-environnement-et-capteurs)
   - 4.13 [GPS et navigation](#413-gps-et-navigation)
   - 4.14 [Autopilot / DAS (Driver Assistance)](#414-autopilot--das-driver-assistance)
   - 4.15 [Interface utilisateur (UI / MCU)](#415-interface-utilisateur-ui--mcu)
   - 4.16 [Commandes au volant (STW / SCCM)](#416-commandes-au-volant-stw--sccm)
   - 4.17 [Thermique et HVAC](#417-thermique-et-hvac)
   - 4.18 [État système / GTW](#418-état-système--gtw)
   - 4.19 [Capteurs de stationnement (PARK)](#419-capteurs-de-stationnement-park)
   - 4.20 [Alertes UI](#420-alertes-ui)
   - 4.21 [Divers](#421-divers)
5. [Tableau récapitulatif des messages](#5-tableau-récapitulatif-des-messages)
6. [Recoupements entre fichiers](#6-recoupements-entre-fichiers)
7. [Notes et conventions](#7-notes-et-conventions)

---

## 1. Vue d'ensemble

| Fichier | Messages | Signaux | Bus | Notes |
|---------|----------|---------|-----|-------|
| `tesla_can.dbc` | 159 | ~2 752 | VehicleBus, ChassisBus | Base la plus complète |
| `Model3CAN.dbc` | 159 | ~2 752 | VehicleBus, ChassisBus | Identique à tesla_can.dbc |
| `tesla_can2.dbc` | 19 | 210 | (NEO, MCU, GTW…) | Signaux openpilot / commaai |
| `Tesla-Model 3.md` | 10 IDs | ~30 | — | Référence VBOX simplifiée |

**Total unique :** ~170 messages distincts, ~2 900+ signaux uniques.

---

## 2. Nœuds CAN (ECUs)

### tesla_can.dbc / Model3CAN.dbc
| Nœud | Rôle |
|------|------|
| `VehicleBus` | Bus véhicule principal |
| `ChassisBus` | Bus châssis (dynamique, freinage, direction) |
| `PartyBus` | Bus accessoires |
| `Receiver` | Récepteur générique |

### tesla_can2.dbc
| Nœud | Rôle |
|------|------|
| `NEO` | Autopilot / openpilot device |
| `MCU` | Media Control Unit (écran central) |
| `GTW` | Gateway (passerelle inter-bus) |
| `EPAS` | Electronic Power Assisted Steering |
| `DI` | Drive Inverter (inverseur de traction) |
| `ESP` | Electronic Stability Program |
| `SBW` | Shift-By-Wire |
| `STW` | Steering Wheel (commandes au volant) |
| `EPB` | Electronic Parking Brake |

---

## 3. Bus CAN

| Bus | Débit typique | Rôle |
|-----|--------------|------|
| **ChassisBus** | 500 kbps | Dynamique véhicule, freinage, direction, capteurs inertiels, Autopilot DAS |
| **VehicleBus** | 500 kbps | Carrosserie, batterie, charge, UI, HVAC, inverseurs |
| **PartyBus** | Variable | Accessoires (non détaillé dans les DBC) |

---

## 4. Signaux par catégorie fonctionnelle

### 4.1 Dynamique véhicule (IMU / Inertiel)

#### 0x101 (257) — RCM_inertial1 / GTW_epasControl
**tesla_can.dbc :**
| Signal | Bits | Factor | Offset | Unité | Plage | Description |
|--------|------|--------|--------|-------|-------|-------------|
| RCM_yawRate | 0\|16 signed | 0.0001 | 0 | rad/s | ±3.2766 | Vitesse de lacet |
| RCM_pitchRate | 16\|15 signed | 0.00025 | 0 | rad/s | ±4.096 | Vitesse de tangage |
| RCM_rollRate | 31\|15 signed | 0.00025 | 0 | rad/s | ±4.096 | Vitesse de roulis |
| RCM_yawRateQF | 48\|1 | 1 | 0 | — | 0–1 | Qualité lacet |
| RCM_pitchRateQF | 50\|2 | 1 | 0 | — | 0–3 | Qualité tangage |
| RCM_rollRateQF | 46\|2 | 1 | 0 | — | 0–3 | Qualité roulis |
| RCM_inertial1Counter | 52\|4 | 1 | 0 | — | 0–15 | Compteur |
| RCM_inertial1Checksum | 56\|8 | 1 | 0 | — | 0–255 | Checksum |

**tesla_can2.dbc (même ID 257) :**
| Signal | Bits | Description |
|--------|------|-------------|
| GTW_epasControlChecksum | 23\|8 | Checksum contrôle EPAS |
| GTW_epasControlCounter | 11\|4 | Compteur |
| GTW_epasControlType | 15\|2 | Type de contrôle (WITHOUT/WITH_ANGLE/WITH_TORQUE/WITH_BOTH) |
| GTW_epasEmergencyOn | 7\|1 | Mode urgence |
| GTW_epasLDWEnabled | 12\|1 | LDW autorisé |
| GTW_epasPowerMode | 6\|4 | Mode alimentation |
| GTW_epasTuneRequest | 2\|3 | Réglage direction (Comfort/Sport/Standard) |

**VBOX (Tesla-Model 3.md) :**
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| Yaw_Rate | 0.0001 | rad | Vitesse de lacet |

#### 0x111 (273) — RCM_inertial2
| Signal | Bits | Factor | Unité | Plage | Description |
|--------|------|--------|-------|-------|-------------|
| RCM_longitudinalAccel | 0\|16 signed | 0.00125 | m/s² | ±40.96 | Accélération longitudinale |
| RCM_lateralAccel | 16\|16 signed | 0.00125 | m/s² | ±40.96 | Accélération latérale |
| RCM_verticalAccel | 32\|16 signed | 0.00125 | m/s² | ±40.96 | Accélération verticale |
| RCM_longitudinalAccelQF | 48\|1 | — | — | 0–1 | Qualité accel. longitudinale |
| RCM_lateralAccelQF | 49\|1 | — | — | 0–1 | Qualité accel. latérale |
| RCM_verticalAccelQF | 50\|1 | — | — | 0–1 | Qualité accel. verticale |
| Counter + Checksum | — | — | — | — | Sécurité trame |

#### 0x116 (278) — RCM_inertial2New
Même structure que RCM_inertial2 (accélérations 3 axes), version mise à jour.

---

### 4.2 Direction (EPAS / Steering)

#### 0x003 (3) — STW_ANGL_STAT *(tesla_can2.dbc)*
| Signal | Bits | Factor | Offset | Unité | Description |
|--------|------|--------|--------|-------|-------------|
| StW_Angl | 5\|14 | 0.5 | -2048 | deg | Angle volant |
| StW_AnglSpd | 21\|14 | 0.5 | -2048 | /s | Vitesse angulaire volant |
| StW_AnglSens_Stat | 33\|2 | — | — | — | État capteur (OK/INI/ERR/ERR_INI) |
| StW_AnglSens_Id | 35\|2 | — | — | — | ID capteur |
| MC_STW_ANGL_STAT | 55\|4 | — | — | — | Message Counter |
| CRC_STW_ANGL_STAT | 63\|8 | — | — | — | CRC |

#### 0x00E (14) — STW_ANGLHP_STAT *(tesla_can2.dbc)*
| Signal | Bits | Factor | Offset | Unité | Plage | Description |
|--------|------|--------|--------|-------|-------|-------------|
| StW_AnglHP | 5\|14 | 0.1 | -819.2 | deg | ±819° | Angle volant haute précision |
| StW_AnglHP_Spd | 21\|14 | 0.5 | -4096 | deg/s | ±4096 | Vitesse angulaire HP |
| StW_AnglHP_Sens_Stat | 33\|2 | — | — | — | — | État capteur HP |
| StW_AnglHP_Sens_Id | 35\|2 | — | — | — | — | ID capteur HP |

#### 0x129 (297) — SteeringAngle *(tesla_can.dbc)*
| Signal | Description |
|--------|-------------|
| SteeringAngle | Angle volant (5 signaux au total) |

#### 0x370 (880) — EPAS_sysStatus *(tesla_can2.dbc)*
| Signal | Bits | Factor | Offset | Unité | Description |
|--------|------|--------|--------|-------|-------------|
| EPAS_internalSAS | 37\|14 | 0.1 | -819.2 | deg | Angle capteur interne |
| EPAS_steeringRackForce | 1\|10 | 50 | -25575 | N | Force sur la crémaillère |
| EPAS_torsionBarTorque | 19\|12 | 0.01 | -20.5 | Nm | Couple barre de torsion |
| EPAS_handsOnLevel | 39\|2 | — | — | — | Niveau mains sur volant |
| EPAS_steeringFault | 2\|1 | — | — | — | Défaut direction |
| EPAS_steeringReduced | 3\|1 | — | — | — | Direction réduite |
| EPAS_eacErrorCode | 23\|4 | — | — | — | Code erreur EAC |
| EPAS_eacStatus | 55\|3 | — | — | — | État EAC |
| EPAS_currentTuneMode | 7\|4 | — | — | — | Mode réglage courant |

#### 0x488 (1160) — DAS_steeringControl *(tesla_can2.dbc)*
| Signal | Bits | Factor | Offset | Unité | Description |
|--------|------|--------|--------|-------|-------------|
| DAS_steeringAngleRequest | 6\|15 | 0.1 | -1638.35 | deg | Requête angle volant (Autopilot) |
| DAS_steeringControlType | 23\|2 | — | — | — | Type de contrôle |
| DAS_steeringHapticRequest | 7\|1 | — | — | — | Requête retour haptique |

**VBOX :**
| Signal | Factor | Offset | Unité | Description |
|--------|--------|--------|-------|-------------|
| Steering_Angle (0x488) | 0.1 | -1640 | ° | Angle volant (Motorola) |

#### 0x155 (341) — WheelAngles *(tesla_can.dbc)*
| Message | Signaux | Description |
|---------|---------|-------------|
| ID155WheelAngles | 13 signaux | Angles de braquage des roues |

---

### 4.3 Groupe motopropulseur (Drive Inverter)

#### 0x108 (264) — DI_torque1 / DIR_torque *(tesla_can2.dbc + tesla_can.dbc)*
**tesla_can2.dbc :**
| Signal | Bits | Factor | Unité | Plage | Description |
|--------|------|--------|-------|-------|-------------|
| DI_torqueDriver | 0\|13 signed | 0.25 | Nm | ±750 | Couple demandé par conducteur |
| DI_torqueMotor | 16\|13 signed | 0.25 | Nm | ±750 | Couple moteur effectif |
| DI_motorRPM | 32\|16 signed | 1 | RPM | ±17000 | Régime moteur |
| DI_pedalPos | 48\|8 | 0.4 | % | 0–100 | Position pédale accélérateur |
| DI_soptState | 29\|3 | — | — | — | État test SOPT |
| DI_torque1Counter | 13\|3 | — | — | — | Compteur |
| DI_torque1Checksum | 56\|8 | — | — | — | Checksum |

**VBOX :**
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| Engine_Torque | 0.2 | Nm | Couple moteur |
| Engine_Speed | 0.1 | rpm | Régime moteur |

#### 0x118 (280) — DI_torque2 / DriveSystemStatus *(tesla_can2.dbc + tesla_can.dbc)*
**tesla_can2.dbc :**
| Signal | Bits | Factor | Unité | Plage | Description |
|--------|------|--------|-------|-------|-------------|
| DI_torqueEstimate | 0\|12 signed | 0.5 | Nm | ±750 | Estimation couple |
| DI_gear | 12\|3 | — | — | — | Rapport engagé (P/R/N/D) |
| DI_brakePedal | 15\|1 | — | — | — | Pédale frein appuyée |
| DI_vehicleSpeed | 16\|12 | 0.05 | MPH | -25 à 179.75 | Vitesse véhicule |
| DI_gearRequest | 28\|3 | — | — | — | Rapport demandé |
| DI_torqueInterfaceFailure | 31\|1 | — | — | — | Défaut interface couple |
| DI_brakePedalState | 36\|2 | — | — | — | État pédale frein |
| DI_epbParkRequest | 38\|1 | — | — | — | Requête frein parking |
| DI_epbInterfaceReady | 39\|1 | — | — | — | Interface EPB prête |

**tesla_can.dbc (ID118 DriveSystemStatus) :**
14 signaux incluant état du système de traction.

**VBOX :**
| Signal | Description |
|--------|-------------|
| Accelerator_Pedal_Position | Position accélérateur (2 variantes) |
| Brake_Light | Témoin frein |

#### 0x186 (390) — DIF_torque *(tesla_can.dbc)*
7 signaux — Couple inverseur avant.

#### 0x1D8 (472) — RearTorque *(tesla_can.dbc)*
5 signaux — Couple inverseur arrière.

#### 0x1D4 (468) / 0x1D5 (469) — FrontTorque *(tesla_can.dbc)*
Couple avant (ancien et nouveau format).

#### 0x656 (1622) / 0x335 (821) — FrontDIinfo / RearDIinfo *(tesla_can.dbc)*
19 signaux chacun — Informations inverseur avant/arrière.

#### 0x266 (614) — RearInverterPower *(tesla_can.dbc)*
6 signaux — Puissance inverseur arrière.

#### 0x2E5 (741) — FrontInverterPower *(tesla_can.dbc)*
6 signaux — Puissance inverseur avant.

#### 0x2E6 (742) — PlaidFrontPower *(tesla_can.dbc)*
6 signaux — Puissance inverseur avant Plaid (3 moteurs).

#### 0x269 (617) / 0x27C (636) — LeftRearPower / RightRearPower *(tesla_can.dbc)*
6 signaux chacun — Puissance arrière gauche/droite (Plaid).

#### 0x376 (886) / 0x315 (789) — FrontInverterTemps / RearInverterTemps *(tesla_can.dbc)*
8 signaux chacun — Températures inverseurs.

#### 0x5D5 (1493) / 0x556 (1366) — RearDItemps / FrontDItemps *(tesla_can.dbc)*
5–7 signaux — Températures moteur/inverseur.

#### 0x557 (1367) / 0x5D7 (1495) — FrontThermalControl / RearThermalControl *(tesla_can.dbc)*
4 signaux chacun — Commande thermique inverseurs.

#### 0x396 (918) / 0x395 (917) — FrontOilPump / DIR_oilPump *(tesla_can.dbc)*
10 signaux chacun — Pompe à huile (refroidissement).

#### 0x7D5 (2005) / 0x757 (1879) — DIR_debug / DIF_debug *(tesla_can.dbc)*
131 signaux chacun — Debug inverseurs (multiplexé).

#### 0x1D6 (470) — DI_limits *(tesla_can.dbc)*
| Signal | Description |
|--------|-------------|
| DI_limitPCBTemp | Limite température PCB |
| DI_limitInverterTemp | Limite température inverseur |
| DI_limitStatorTemp | Limite température stator |
| DI_limitPoleTemp | Limite température pôles |
| DI_limitLimpMode | Mode limp actif |
| DI_limitDeltaFluidTemp | Limite delta température fluide |
| DI_limitDischargePower | Limite puissance décharge |
| DI_limitRegenPower | Limite puissance régénération |
| DI_limitMotorCurrent | Limite courant moteur |
| DI_limitMotorVoltage | Limite tension moteur |
| DI_limitDriveTorque | Limite couple traction |
| DI_limitRegenTorque | Limite couple régénération |
| DI_limitTCDrive / DI_limitTCRegen | Limites traction control |
| DI_limitShift | Limite changement rapport |
| DI_limitBaseSpeed | Limite vitesse de base |
| DI_limitIBat | Limite courant batterie |
| DI_limitDcCapTemp | Limite temp. condensateur DC |
| DI_limitVBatLow / DI_limitVBatHigh | Limites tension batterie |
| DI_limitMotorSpeed | Limite vitesse moteur |
| DI_limitVehicleSpeed | Limite vitesse véhicule |
| DI_limitDiff | Limite différentiel |
| DI_limitGracefulPowerOff | Arrêt progressif |
| DI_limitIGBTJunctTemp | Limite temp. jonction IGBT |
| DI_limitShockTorque | Limite couple choc |
| DI_limitOilPumpFluidTemp | Limite temp. fluide pompe |
| DI_limitStatorFrequency | Limite fréquence stator |
| DI_limitClutch | Limite embrayage |
| DI_limitObstacleDetection | Détection obstacle |
| DI_limitRotorTemp | Limite temp. rotor |
| + autres limites busbar/câble HV | — |
**38 signaux booléens au total.**

#### 0x368 (872) — DI_state *(tesla_can2.dbc)*
| Signal | Bits | Factor | Unité | Description |
|--------|------|--------|-------|-------------|
| DI_systemState | 0\|3 | — | — | État système DI |
| DI_vehicleHoldState | 3\|3 | — | — | État maintien véhicule |
| DI_proximity | 6\|1 | — | — | Proximité clé |
| DI_driveReady | 7\|1 | — | — | Prêt à rouler |
| DI_regenLight | 8\|1 | — | — | Témoin régénération |
| DI_state | 9\|3 | — | — | État DI (UNAVAILABLE/STANDBY/FAULT/ABORT/ENABLE) |
| DI_cruiseState | 12\|4 | — | — | État régulateur (OFF/STANDBY/ENABLED/STANDSTILL/OVERRIDE/FAULT…) |
| DI_analogSpeed | 16\|12 | 0.1 | speed | Vitesse analogique |
| DI_immobilizerState | 28\|3 | — | — | État antidémarrage |
| DI_speedUnits | 31\|1 | — | — | Unité vitesse (MPH/KPH) |
| DI_cruiseSet | 32\|9 | 0.5 | speed | Consigne régulateur |
| DI_aebState | 41\|3 | — | — | État AEB |
| DI_digitalSpeed | 48\|8 | 1 | — | Vitesse numérique |
| DI_stateCounter/Checksum | — | — | — | Sécurité trame |

#### 0x257 (599) — DIspeed *(tesla_can.dbc)*
6 signaux — Vitesse DI.

#### 0x267 (615) — DI_vehicleEstimates *(tesla_can.dbc)*
| Signal | Bits | Factor | Unité | Description |
|--------|------|--------|-------|-------------|
| DI_mass | 0\|10 | 5 | kg | Masse estimée (1900–7010 kg) |
| DI_tireFitment | 10\|2 | — | — | Type pneumatiques |
| DI_trailerDetected | 12\|1 | — | — | Remorque détectée |
| DI_relativeTireTreadDepth | 16\|6 signed | 0.2 | mm | Profondeur restante pneu |
| DI_massConfidence | 32\|1 | — | — | Confiance estimation masse |
| DI_gradeEst | 33\|7 signed | 1 | % | Estimation pente |
| DI_gradeEstInternal | 48\|7 signed | 1 | % | Estimation pente (interne) |
| DI_steeringAngleOffset | 56\|8 signed | 0.2 | Deg | Offset angle volant |

#### 0x2B6 (694) — DI_chassisControlStatus *(tesla_can.dbc)*
8 signaux — État du contrôle châssis.

---

### 4.4 Freinage (ESP / iBooster / EPB)

#### 0x145 (325) — ESP_status *(tesla_can.dbc)* + VBOX Brake_Position
| Signal | Bits | Factor | Unité | Description |
|--------|------|--------|-------|-------------|
| ESP_statusChecksum | 0\|8 | — | — | Checksum |
| ESP_statusCounter | 8\|4 | — | — | Compteur |
| ESP_espModeActive | 12\|2 | — | — | Mode ESP actif |
| ESP_stabilityControlSts2 | 14\|2 | — | — | État contrôle stabilité |
| ESP_ebdFaultLamp | 16\|1 | — | — | Témoin défaut EBD |
| ESP_absFaultLamp | 17\|1 | — | — | Témoin défaut ABS |
| ESP_espFaultLamp | 18\|1 | — | — | Témoin défaut ESP |
| ESP_hydraulicBoostEnabled | 19\|1 | — | — | Boost hydraulique actif |
| ESP_espLampFlash | 20\|1 | — | — | Flash témoin ESP |
| ESP_brakeLamp | 21\|1 | — | — | Témoin frein |
| ESP_absBrakeEvent2 | 22\|2 | — | — | Événement freinage ABS |
| ESP_longitudinalAccelQF | 24\|1 | — | — | Qualité accel. longitudinale |
| ESP_lateralAccelQF | 25\|1 | — | — | Qualité accel. latérale |
| ESP_yawRateQF | 26\|1 | — | — | Qualité lacet |
| ESP_steeringAngleQF | 27\|1 | — | — | Qualité angle volant |
| ESP_brakeDiscWipingActive | 28\|1 | — | — | Essuyage disques actif |
| ESP_driverBrakeApply | 29\|2 | — | — | Freinage conducteur |
| ESP_brakeApply | 31\|1 | — | — | Freinage actif |
| ESP_cdpStatus | 34\|2 | — | — | État CDP |
| ESP_ptcTargetState | 36\|2 | — | — | État cible PTC |
| ESP_btcTargetState | 38\|2 | — | — | État cible BTC |
| ESP_ebrStandstillSkid | 48\|1 | — | — | Patinage arrêt EBR |
| ESP_ebrStatus | 49\|2 | — | — | État EBR |
| ESP_brakeTorqueTarget | 51\|13 | 2 | Nm | Cible couple frein (0–16382) |

#### 0x135 (309) — ESP_135h *(tesla_can2.dbc)*
| Signal | Description |
|--------|-------------|
| ESP_absBrakeEvent | Événement freinage ABS |
| ESP_brakeDiscWipingActive | Essuyage disques |
| ESP_brakeLamp | Témoin frein |
| ESP_espFaultLamp / ESP_espLampFlash | Témoins ESP |
| ESP_hillStartAssistActive | Aide au démarrage en côte |
| ESP_stabilityControlSts | État contrôle stabilité (OFF/ON/ENGAGED/FAULTED…) |
| ESP_tcLampFlash / ESP_tcOffLamp | Témoins traction control |
| ESP_absFaultLamp / ESP_espOffLamp | Témoins défaut |
| ESP_messagePumpService/Failure | Messages pompe |
| ESP_messageEBDFailure | Défaut EBD |
| ESP_tcDisabledByFault | TC désactivé par défaut |
| ESP_hydraulicBoostEnabled | Boost hydraulique |
| + Counter/Checksum | Sécurité trame |

#### 0x155 (341) — ESP_B *(tesla_can2.dbc)*
| Signal | Bits | Factor | Unité | Description |
|--------|------|--------|-------|-------------|
| ESP_vehicleSpeed | 47\|16 | 0.01 | kph | Vitesse véhicule ESP |
| ESP_vehicleSpeedQF | 57\|2 | — | — | Qualité vitesse |
| ESP_wheelPulseCountFrL | 7\|8 | 1 | — | Compteur impulsions AV gauche |
| ESP_wheelPulseCountFrR | 15\|8 | 1 | — | Compteur impulsions AV droit |
| ESP_wheelPulseCountReL | 23\|8 | 1 | — | Compteur impulsions AR gauche |
| ESP_wheelPulseCountReR | 31\|8 | 1 | — | Compteur impulsions AR droit |

#### 0x185 (389) — ESP_brakeTorque *(tesla_can.dbc)*
7 signaux — Couple de freinage ESP.

**VBOX :** Brake_Position (0x145 : 10 bits, factor 0.125 ; 0x185 : 12 bits, factor 0.08).

#### 0x39D (925) — IBST_status *(tesla_can.dbc)*
| Signal | Bits | Factor | Unité | Description |
|--------|------|--------|-------|-------------|
| IBST_statusChecksum | 0\|8 | — | — | Checksum |
| IBST_statusCounter | 8\|4 | — | — | Compteur |
| IBST_iBoosterStatus | 12\|3 | — | — | État iBooster |
| IBST_driverBrakeApply | 16\|2 | — | — | Freinage conducteur |
| IBST_internalState | 18\|3 | — | — | État interne |
| IBST_sInputRodDriver | 21\|12 | 0.015625 | mm | Course tige d'entrée (-5 à 47 mm) |

#### 0x214 (532) — EPB_epasControl *(tesla_can2.dbc)*
3 signaux — Contrôle EPB pour EPAS.

#### 0x228 (552) / 0x288 (648) — EPBrightStatus / EPBleftStatus *(tesla_can.dbc)*
15 signaux chacun — État frein de stationnement électrique gauche/droit.

---

### 4.5 Vitesse des roues

#### 0x175 (373) — WheelSpeed *(tesla_can.dbc)*
| Signal | Description |
|--------|-------------|
| 6 signaux | Vitesses roues (4 roues + compteur/checksum) |

**VBOX (4 variantes) :**
| Variante | Factor | Unité |
|----------|--------|-------|
| km/h (factor 0.025) | 0.025 | km/h |
| km/h (factor 0.04) | 0.04 | km/h |
| mph (factor 0.015525) | 0.015525 | mph |
| mph (factor 0.02484) | 0.02484 | mph |

Chaque variante fournit : `Wheel_Speed_FL`, `Wheel_Speed_FR`, `Wheel_Speed_RL`, `Wheel_Speed_RR` (13 bits chacun, unsigned, Intel).

---

### 4.6 Batterie et BMS

#### 0x300 (768) — BMS_info *(tesla_can.dbc)*
15 signaux — Informations batterie principales.

#### 0x212 (530) — BMS_status *(tesla_can.dbc)*
22 signaux — État BMS.

#### 0x232 (562) — BMS_contactorRequest *(tesla_can.dbc)*
| Signal | Bits | Factor | Unité | Description |
|--------|------|--------|-------|-------------|
| BMS_fcContactorRequest | 0\|3 | — | — | Requête contacteur fast charge |
| BMS_packContactorRequest | 3\|3 | — | — | Requête contacteur pack |
| BMS_gpoHasCompleted | 6\|1 | — | — | GPO terminé |
| BMS_ensShouldBeActiveForDrive | 7\|1 | — | — | ENS actif pour conduite |
| BMS_pcsPwmDisable | 8\|1 | — | — | Désactivation PWM PCS |
| BMS_internalHvilSenseV | 16\|16 | 0.001 | V | Tension HVIL interne |
| BMS_fcLinkOkToEnergizeRequest | 32\|2 | — | — | OK pour énergiser lien FC |

#### 0x292 (658) — BMS_SOC *(tesla_can.dbc)*
6 signaux — État de charge batterie.

#### 0x252 (594) — BMS_powerAvailable *(tesla_can.dbc)*
6 signaux — Puissance disponible batterie.

#### 0x2D2 (722) — BMSVAlimits *(tesla_can.dbc)*
4 signaux — Limites tension/courant BMS.

#### 0x312 (786) — BMSthermal *(tesla_can.dbc)*
9 signaux — Thermique batterie.

#### 0x332 (818) — BattBrickMinMax *(tesla_can.dbc)*
11 signaux — Min/max tensions briques batterie.

#### 0x401 (1025) — BrickVoltages *(tesla_can.dbc)*
139 signaux (multiplexé) — Tension individuelle de chaque brique.

#### 0x3F2 (1010) — BMSCounters *(tesla_can.dbc)*
21 signaux — Compteurs BMS.

#### 0x3D2 (978) — TotalChargeDischarge *(tesla_can.dbc)*
2 signaux — Énergie totale chargée/déchargée.

#### 0x132 (306) — HVBattAmpVolt *(tesla_can.dbc)*
4 signaux — Courant et tension batterie HV.

#### 0x352 (850) — BMS_energyStatus *(tesla_can.dbc)*
7 signaux — État énergétique batterie.

#### 0x392 (914) — BMS_packConfig *(tesla_can.dbc)*
5 signaux — Configuration pack batterie.

#### 0x72A (1834) — BMS_serialNumber *(tesla_can.dbc)*
15 signaux — Numéro de série BMS (multiplexé).

#### 0x016 (22) — DI_bmsRequest *(tesla_can.dbc)*
2 signaux — Requête DI vers BMS (ouverture contacteurs, version interface).

---

### 4.7 Charge (AC / DC / PCS)

#### 0x204 (516) — PCS_chgStatus *(tesla_can.dbc)*
| Signal | Bits | Factor | Unité | Description |
|--------|------|--------|-------|-------------|
| PCS_chgMainState | 0\|4 | — | — | État principal charge |
| PCS_hvChargeStatus | 4\|2 | — | — | État charge HV |
| PCS_gridConfig | 6\|2 | — | — | Configuration réseau |
| PCS_chgPHAEnable / B / C | 8–10\|1 | — | — | Activation phases A/B/C |
| PCS_chgInstantAcPowerAvailable | 16\|8 | 0.1 | kW | Puissance AC instantanée |
| PCS_chgMaxAcPowerAvailable | 24\|8 | 0.1 | kW | Puissance AC max |
| PCS_chgPHA/B/C LineCurrentRequest | 32–48\|8 | 0.1 | A | Courant demandé par phase |
| PCS_chgPwmEnableLine | 56\|1 | — | — | Ligne pilote PWM |
| PCS_chargeShutdownRequest | 57\|2 | — | — | Requête arrêt charge |
| PCS_hwVariantType | 59\|2 | — | — | Type variante hardware |

#### 0x27D (637) — CP_dcChargeLimits *(tesla_can.dbc)*
| Signal | Bits | Factor | Unité | Description |
|--------|------|--------|-------|-------------|
| CP_evseMaxDcCurrentLimit | 0\|13 | 0.0732 | A | Limite courant DC max EVSE |
| CP_evseMinDcCurrentLimit | 13\|13 | 0.0732 | A | Limite courant DC min EVSE |
| CP_evseMaxDcVoltageLimit | 26\|13 | 0.0732 | V | Limite tension DC max EVSE |
| CP_evseMinDcVoltageLimit | 39\|13 | 0.0732 | V | Limite tension DC min EVSE |
| CP_evseInstantDcCurrentLimit | 52\|12 | 0.1465 | A | Limite courant DC instantané |

#### 0x2BD (701) — CP_dcPowerLimits *(tesla_can.dbc)*
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| CP_evseInstantDcPowerLimit | 0.0623 | kW | Limite puissance DC instantanée (max 510 kW) |
| CP_evseMaxDcPowerLimit | 0.0623 | kW | Limite puissance DC max |

#### 0x29D (669) — CP_dcChargeStatus *(tesla_can.dbc)*
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| CP_evseOutputDcCurrent | 0.0732 | A | Courant DC sortie EVSE |
| CP_evseOutputDcVoltage | 0.0732 | V | Tension DC sortie EVSE |
| CP_evseOutputDcCurrentStale | — | — | Donnée périmée |

#### 0x25D (605) — CP_status *(tesla_can.dbc)*
29 signaux — État complet du port de charge (porte, câble, latch, LED, UHF, température…).

#### 0x23D (573) / 0x13D (317) / 0x43D (1085) — CP_chargeStatus *(tesla_can.dbc)*
6–8 signaux chacun — État de charge (différents intervalles).

#### 0x21D (541) — CP_evseStatus *(tesla_can.dbc)*
19 signaux — État EVSE.

#### 0x37D (893) — CP_thermalStatus *(tesla_can.dbc)*
5 signaux — État thermique port de charge.

#### 0x2B4 (692) — PCS_dcdcRailStatus *(tesla_can.dbc)*
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| PCS_dcdcLvBusVolt | 0.0391 | V | Tension bus 12V |
| PCS_dcdcHvBusVolt | 0.1465 | V | Tension bus HV |
| PCS_dcdcLvOutputCurrent | 0.1 | A | Courant sortie 12V (max 400 A) |

#### 0x224 (548) — PCSDCDCstatus *(tesla_can.dbc)*
17 signaux — État complet convertisseur DC-DC.

#### 0x31C (796) — CC_chgStatus *(tesla_can.dbc)*
9 signaux — État charge CC.

#### 0x264 (612) — ChargeLineStatus *(tesla_can.dbc)*
4 signaux — État ligne de charge.

#### 0x541 (1345) — FastChargeMaxLimits *(tesla_can.dbc)*
2 signaux — Limites max charge rapide.

#### 0x244 (580) — FastChargeLimits *(tesla_can.dbc)*
4 signaux — Limites charge rapide.

#### 0x214 (532) — FastChargeVA *(tesla_can.dbc)*
9 signaux — Tension/courant charge rapide.

#### 0x215 (533) — FCisolation *(tesla_can.dbc)*
1 signal — Isolation charge rapide.

#### 0x217 (535) — FC_status3 *(tesla_can.dbc)*
8 signaux — État charge rapide.

#### 0x51E (1310) — FC_info *(tesla_can.dbc)*
43 signaux — Informations charge rapide (multiplexé).

#### 0x75D (1885) — CP_sensorData *(tesla_can.dbc)*
44 signaux — Données capteurs port de charge (multiplexé).

#### 0x333 (819) — UI_chargeRequest *(tesla_can.dbc)*
11 signaux — Requêtes charge depuis UI.

---

### 4.8 Haute tension (HVP / Contacteurs)

#### 0x20A (522) — HVP_contactorState *(tesla_can.dbc)*
| Signal | Description |
|--------|-------------|
| HVP_packContNegativeState | État contacteur négatif pack |
| HVP_packContPositiveState | État contacteur positif pack |
| HVP_packContactorSetState | État ensemble contacteurs pack |
| HVP_fcContNegativeState / fcContPositiveState | Contacteurs fast charge |
| HVP_fcContactorSetState | Ensemble contacteurs FC |
| HVP_packCtrsClosingAllowed / OpenRequested / OpenNowRequested | Contrôle contacteurs pack |
| HVP_fcCtrsClosingAllowed / OpenRequested / OpenNowRequested | Contrôle contacteurs FC |
| HVP_hvilStatus | État HVIL (High Voltage Interlock Loop) |
| HVP_dcLinkAllowedToEnergize | Lien DC autorisé |
| HVP_pyroTestInProgress | Test pyrotechnique en cours |
**22 signaux au total.**

#### 0x22A (554) — HVP_pcsControl *(tesla_can.dbc)*
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| HVP_dcLinkVoltageRequest | 0.1 | V | Tension DC-link demandée |
| HVP_dcLinkVoltageFiltered | 1 | V | Tension DC-link filtrée |
| HVP_pcsControlRequest | — | — | Requête contrôle PCS |
| HVP_pcsChargeHwEnabled | — | — | Hardware charge activé |
| HVP_pcsDcdcHwEnabled | — | — | Hardware DC-DC activé |

#### 0x126 (294) / 0x1A5 (421) — RearHVStatus / FrontHVStatus *(tesla_can.dbc)*
6 signaux chacun — État HV arrière/avant.

#### 0x127 (295) / 0x12A (298) — LeftRearHVStatus / RightRearHVStatus *(tesla_can.dbc)*
6 signaux chacun — État HV arrière gauche/droit (Plaid).

---

### 4.9 Carrosserie (Portes / Coffre / Vitres)

#### 0x318 (792) — GTW_carState / SystemTimeUTC *(tesla_can2.dbc + tesla_can.dbc)*
**tesla_can2.dbc :**
| Signal | Bits | Factor | Offset | Unité | Description |
|--------|------|--------|--------|-------|-------------|
| DOOR_STATE_FL | 13\|2 | — | — | — | Porte AV gauche (closed/open/Init/SNA) |
| DOOR_STATE_FR | 15\|2 | — | — | — | Porte AV droite |
| DOOR_STATE_RL | 23\|2 | — | — | — | Porte AR gauche |
| DOOR_STATE_RR | 30\|2 | — | — | — | Porte AR droite |
| DOOR_STATE_FrontTrunk | 51\|2 | — | — | — | Frunk |
| BOOT_STATE | 47\|2 | — | — | — | Coffre |
| YEAR | 6\|7 | 1 | 2000 | Year | Année |
| MONTH | 11\|4 | — | — | Month | Mois |
| DAY | 36\|5 | — | — | — | Jour |
| Hour | 28\|5 | — | — | h | Heure |
| MINUTE | 45\|6 | — | — | min | Minute |
| SECOND | 21\|6 | — | — | s | Seconde |
| CERRD | 7\|1 | — | — | — | Erreur CAN détectée |
| GTW_updateInProgress | 49\|2 | — | — | — | Mise à jour en cours |
| MCU_factoryMode | 52\|1 | — | — | — | Mode usine |
| MCU_transportModeOn | 53\|1 | — | — | — | Mode transport |

**tesla_can.dbc (0x318) :** SystemTimeUTC — 6 signaux horodatage.

#### 0x102 (258) — VCLEFT_doorStatus *(tesla_can.dbc)*
| Signal | Description |
|--------|-------------|
| VCLEFT_frontLatchStatus | État loquet porte AV gauche |
| VCLEFT_rearLatchStatus | État loquet porte AR gauche |
| VCLEFT_frontLatchSwitch / rearLatchSwitch | Contacteur loquet |
| VCLEFT_frontHandlePulled / rearHandlePulled | Poignée tirée |
| VCLEFT_frontHandlePWM / rearHandlePWM | PWM poignée (%) |
| VCLEFT_frontIntSwitchPressed / rearIntSwitchPressed | Bouton intérieur |
| VCLEFT_frontRelActuatorSwitch / rearRelActuatorSwitch | Actionneur déverrouillage |
| VCLEFT_mirror* | 7 signaux miroir (état, pliage, chauffage, position X/Y) |
| VCLEFT_frontHandlePulledPersist | Poignée tirée persistant |
**20 signaux.**

#### 0x103 (259) — VCRIGHT_doorStatus *(tesla_can.dbc)*
Structure similaire côté droit (21 signaux) + `VCRIGHT_trunkLatchStatus`.

#### 0x122 (290) — VCLEFT_doorStatus2 *(tesla_can.dbc)*
12 signaux — État détaillé portes gauches (état porte, poignée brute/anti-rebond, véhicule en mouvement…).

#### 0x142 (322) — VCLEFT_liftgateStatus *(tesla_can.dbc)*
13 signaux (multiplexé) — Hayon motorisé (position en degrés, vitesse, courant vérin, état, source commande).

#### 0x119 (281) — VCSEC_windowRequests *(tesla_can.dbc)*
7 signaux — Commandes vitres (LF/LR/RF/RR, pourcentage, type, protection écran HVAC).

#### 0x283 (643) — BODY_R1 *(tesla_can2.dbc)*
| Signal | Bits | Factor | Offset | Unité | Description |
|--------|------|--------|--------|-------|-------------|
| AirTemp_Insd | 47\|8 | 0.25 | 0 | °C | Température intérieure |
| AirTemp_Outsd | 63\|8 | 0.5 | -40 | °C | Température extérieure |
| DrRLtch_FL/FR/RL/RR_Stat | — | — | — | — | État loquet 5 portes |
| DL_RLtch_Stat | 9\|2 | — | — | — | État loquet hayon |
| EngHd_Stat | 11\|2 | — | — | — | État capot |
| Bckl_Sw_RL/RM/RR_Stat | — | — | — | — | Ceintures arrière |
| LoBm_On_Rq / HiBm_On | — | — | — | — | Feux croisement/route |
| Hrn_On | 26\|1 | — | — | — | Klaxon |
| LgtSens_* | — | — | — | — | Capteur luminosité (crépuscule, nuit, tunnel) |
| MPkBrk_Stat | 28\|1 | — | — | — | Frein de parking |
| RevGr_Engg | — | — | — | — | Marche arrière |
| StW_Cond_Stat | — | — | — | — | État conditionnel volant |
| + divers | — | — | — | — | Remorque, phares, faults |
**31 signaux.**

---

### 4.10 Éclairage

#### 0x3F5 (1013) — VCFRONT_lighting *(tesla_can.dbc)*
| Signal | Description |
|--------|-------------|
| VCFRONT_indicatorLeftRequest / indicatorRightRequest | Clignotants |
| VCFRONT_hazardLightRequest | Warning |
| VCFRONT_lowBeamLeftStatus / lowBeamRightStatus | Feux de croisement |
| VCFRONT_highBeamLeftStatus / highBeamRightStatus | Feux de route |
| VCFRONT_DRLLeftStatus / DRLRightStatus | Feux diurnes |
| VCFRONT_fogLeftStatus / fogRightStatus | Antibrouillards |
| VCFRONT_sideMarkersStatus | Feux de position |
| VCFRONT_sideRepeaterLeftStatus / sideRepeaterRightStatus | Répéteurs latéraux |
| VCFRONT_turnSignalLeftStatus / turnSignalRightStatus | Clignotants status |
| VCFRONT_parkLeftStatus / parkRightStatus | Feux de parking |
| VCFRONT_ambientLightingBrightnes | Luminosité éclairage ambiant (%) |
| VCFRONT_switchLightingBrightness | Luminosité commutateurs (%) |
| VCFRONT_approachLightingRequest | Éclairage d'approche |
| VCFRONT_courtesyLightingRequest | Éclairage de courtoisie |
| VCFRONT_seeYouHomeLightingReq | « See You Home » |
| VCFRONT_lowBeamsCalibrated / lowBeamsOnForDRL | Calibration / DRL |
| VCFRONT_highBeamSwitchActive | Commutateur feux route |
| VCFRONT_simLatchingStalk | Simulation manette |
**28 signaux.**

#### 0x3E2 (994) — VCLEFT_lightStatus *(tesla_can.dbc)*
| Signal | Description |
|--------|-------------|
| VCLEFT_brakeLightStatus | État feux stop |
| VCLEFT_tailLightStatus | État feux arrière |
| VCLEFT_turnSignalStatus | État clignotants |
| VCLEFT_FLMapLightStatus / FR / RL / RR | Liseuses (4 positions) |
| VCLEFT_dynamicBrakeLightStatus | Feux stop dynamiques |
| VCLEFT_frontRideHeight / rearRideHeight | Hauteur de caisse (mm) |
| VCLEFT_trailer* | Status feux remorque (5 signaux) |
| VCLEFT_trailerDetected | Remorque détectée |
**23 signaux.**

#### 0x3E3 (995) — VCRIGHT_lightStatus *(tesla_can.dbc)*
6 signaux — Feux côté droit.

#### 0x3E9 (1001) — DAS_bodyControls *(tesla_can.dbc)*
| Signal | Description |
|--------|-------------|
| DAS_headlightRequest | Requête phares |
| DAS_hazardLightRequest | Requête warning |
| DAS_wiperSpeed | Vitesse essuie-glaces |
| DAS_turnIndicatorRequest | Requête clignotant (+ raison) |
| DAS_highLowBeamDecision | Décision feux route/croisement |
| DAS_dynamicBrakeLightRequest | Feux stop dynamiques |
| DAS_heaterRequest | Requête chauffage |
| DAS_radarHeaterRequest | Chauffage radar |
| DAS_mirrorFoldRequest | Pliage rétroviseurs |
**14 signaux.**

---

### 4.11 TPMS (Pression / Température pneus)

#### 0x219 (537) — VCSEC_TPMSData *(tesla_can.dbc)* — Multiplexé (index 0–3)
| Signal pattern | Factor | Unité | Description |
|----------------|--------|-------|-------------|
| VCSEC_TPMSPressure0–3 | 0.025 | bar | Pression (0–6.35 bar) |
| VCSEC_TPMSTemperature0–3 | 1 (offset -40) | °C | Température (-40 à 214°C) |
| VCSEC_TPMSBatVoltage0–3 | 0.01 (offset 1.5) | V | Tension batterie capteur |
| VCSEC_TPMSLocation0–3 | 1 | — | Position roue |
**17 signaux.**

#### 0x42A (1066) — VCSEC_TPMSConnectionData *(tesla_can.dbc)*
| Signal | Description |
|--------|-------------|
| VCSEC_TPMSSensorState0–3 | État capteur (4 roues) |
| VCSEC_TPMSRSSI0–3 | Force signal BLE (dBm) |
| VCSEC_TPMSConnectionTypeCurrent0–3 | Type connexion actuel |
| VCSEC_TPMSConnectionTypeDesired0–3 | Type connexion désiré |
**16 signaux.**

#### 0x31F (799) — TPMSsensors *(tesla_can.dbc)*
8 signaux — Pression et température 4 pneus (direct).

**VBOX :**
| Signal | Factor | Offset | Unité | Description |
|--------|--------|--------|-------|-------------|
| Tyre_Pressure_FL/FR/RL/RR | 0.025 | 0 | bar | Pression pneu |
| Tyre_Temperature_FL/FR/RL/RR | 1 | -40 | °C | Température pneu |

---

### 4.12 Environnement et capteurs

#### 0x321 (801) — VCFRONT_sensors *(tesla_can.dbc)*
9 signaux — Capteurs avant (température extérieure incluse).

**VBOX :** Air_Temperature (0x321, factor 0.5, offset -40, °C).

#### 0x25B (603) — APP_environment *(tesla_can.dbc)*
| Signal | Description |
|--------|-------------|
| APP_environmentRainy | Pluie détectée |
| APP_environmentSnowy | Neige détectée |

#### 0x2D3 (723) — UI_solarData *(tesla_can.dbc)*
| Signal | Unité | Description |
|--------|-------|-------------|
| UI_solarAzimuthAngle | deg | Azimut solaire |
| UI_solarAzimuthAngleCarRef | deg | Azimut solaire réf. véhicule |
| UI_solarElevationAngle | deg | Élévation solaire |
| UI_isSunUp | — | Soleil levé |
| UI_minsToSunrise / minsToSunset | min | Minutes avant lever/coucher |
| UI_screenPCBTemperature | °C | Température PCB écran |

#### 0x3D8 (984) — Elevation / MCU_locationStatus
**tesla_can.dbc :** 1 signal — Altitude.
**tesla_can2.dbc :**
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| MCU_latitude | 1E-006 | deg | Latitude (±134°) |
| MCU_longitude | 1E-006 | deg | Longitude (±268°) |
| MCU_gpsAccuracy | 0.2 | m | Précision GPS |

---

### 4.13 GPS et navigation

#### 0x3D9 (985) — UI_gpsVehicleSpeed *(tesla_can.dbc)*
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| UI_gpsHDOP | 0.1 | — | Dilution de précision horizontale |
| UI_gpsVehicleHeading | 0.0078125 | deg | Cap véhicule GPS |
| UI_gpsVehicleSpeed | 0.00390625 | kph | Vitesse GPS |
| UI_mppSpeedLimit | 5 | kph/mph | Limite vitesse carte |
| UI_userSpeedOffset | 1 (offset -30) | kph/mph | Offset vitesse utilisateur |
| UI_conditionalSpeedLimit / conditionalLimitActive | — | — | Limite conditionnelle |
| UI_gpsAntennaDisconnected / gpsNmeaMIA | — | — | Diagnostic GPS |

#### 0x2F8 (760) — MCU_gpsVehicleSpeed *(tesla_can2.dbc)*
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| MCU_gpsHDOP | 0.1 | — | HDOP |
| MCU_gpsVehicleHeading | 0.00781 | deg | Cap GPS |
| MCU_gpsVehicleSpeed | 0.00391 | km/hr | Vitesse GPS |
| MCU_mppSpeedLimit | 5 | kph/mph | Limite vitesse |
| MCU_speedLimitUnits | — | — | Unité (MPH/KPH) |
| MCU_userSpeedOffset | 1 (offset -30) | kph/mph | Offset vitesse |

#### 0x04F (79) — GPSLatLong *(tesla_can.dbc)*
3 signaux — Latitude/Longitude GPS.

#### 0x3F3 (1011) — UI_odo *(tesla_can.dbc)*
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| UI_odometer | 0.1 | km | Compteur kilométrique (max 1 677 720 km) |

#### 0x3B6 (950) — odometer *(tesla_can.dbc)*
1 signal — Odomètre (autre format).

---

### 4.14 Autopilot / DAS (Driver Assistance)

#### 0x2B9 (697) — DAS_control *(tesla_can.dbc)*
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| DAS_setSpeed | 0.1 | kph | Vitesse consigne |
| DAS_accState | — | — | État ACC |
| DAS_aebEvent | — | — | Événement AEB |
| DAS_jerkMin / jerkMax | — | m/s³ | Limites de jerk |
| DAS_accelMin / accelMax | 0.04 (offset -15) | m/s² | Limites accélération |

#### 0x399 (921) — DAS_status *(tesla_can.dbc)*
| Signal | Description |
|--------|-------------|
| DAS_autopilotState | État Autopilot (0–15) |
| DAS_blindSpotRearLeft / blindSpotRearRight | Angle mort AR |
| DAS_fusedSpeedLimit / visionOnlySpeedLimit | Limites vitesse fusionnées |
| DAS_forwardCollisionWarning | Alerte collision avant |
| DAS_laneDepartureWarning | Alerte sortie de voie |
| DAS_sideCollisionWarning / sideCollisionAvoid | Collision latérale |
| DAS_autoLaneChangeState | Changement de voie auto |
| DAS_autopilotHandsOnState | Mains sur volant |
| DAS_autoparkReady / autoParked | Autopark |
| DAS_summonAvailable / summonObstacle | Summon |
| DAS_lssState | État Lane Safety System |
| DAS_suppressSpeedWarning | Suppression alerte vitesse |
**26 signaux.**

#### 0x389 (905) — DAS_status2 *(tesla_can.dbc)*
18 signaux — État DAS étendu (ACC report, obstacles PMM, télémétrie radar, collision longitudinale…).

#### 0x239 (569) — DAS_lanes *(tesla_can.dbc)*
13 signaux — Voies détectées (existence gauche/droite, coefficients polynomiaux C0/C1/C2/C3, largeur, portée…).

#### 0x309 (777) — DAS_object *(tesla_can.dbc)* — Multiplexé
57 signaux — Objets détectés (véhicule devant, gauche, droite, cut-in, panneaux).
Inclut pour chaque objet : distance Dx, offset latéral Dy, vitesse relative, type, heading, ID.

#### 0x24A (586) — DAS_visualDebug *(tesla_can.dbc)*
24 signaux — Debug visuel DAS (usage voies/HPP/modèles, état santé, surface route, ULC…).

#### 0x238 (568) — UI_driverAssistMapData *(tesla_can.dbc)*
30 signaux — Données cartographiques assistance (limite vitesse carte, classe route, code pays, Supercharger, autopark…).

#### 0x3F8 (1016) — UI_driverAssistControl *(tesla_can.dbc)*
42 signaux — Contrôle aide à la conduite (LDW/LKA/ELK enable, summon, autopark, ULC blind spot config…).

#### 0x3FD (1021) — UI_autopilotControl *(tesla_can.dbc)* — Multiplexé
38 signaux — Contrôle Autopilot avancé (caméras disable, FSD visualization, cabin camera, blind spot config…).

#### 0x488 (1160) — DAS_steeringControl *(tesla_can2.dbc)*
Voir section Direction.

---

### 4.15 Interface utilisateur (UI / MCU)

#### 0x00C (12) / 0x353 (851) — UI_status *(tesla_can.dbc)*
25 signaux identiques sur deux IDs :
| Signal | Unité | Description |
|--------|-------|-------------|
| UI_touchActive / audioActive / bluetoothActive | — | États interface |
| UI_cellActive / cellConnected / cellSignalBars | — | Réseau cellulaire |
| UI_cellNetworkTechnology | — | Techno réseau (2G/3G/4G/5G) |
| UI_cellReceiverPower | dB | Puissance signal |
| UI_cpuTemperature / pcbTemperature | °C | Températures MCU |
| UI_displayOn / displayReady / readyForDrive | — | État écran/conduite |
| UI_wifiActive / wifiConnected / gpsActive | — | Connectivité |
| UI_screenshotActive / cameraActive | — | Capture/caméra |
| UI_developmentCar / factoryReset / autopilotTrial | — | Modes spéciaux |
| UI_falseTouchCounter | — | Compteur faux contacts |

#### 0x388 (904) — MCU_clusterBacklightRequest *(tesla_can2.dbc)*
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| MCU_clusterBacklightOn | — | — | Rétroéclairage cluster ON |
| MCU_clusterBrightnessLevel | 0.5 | % | Niveau luminosité |
| MCU_clusterReadyForDrive | — | — | Prêt pour conduite |
| MCU_clusterReadyForPowerOff | — | — | Prêt extinction |

#### 0x273 (627) — UI_vehicleControl *(tesla_can.dbc)*
38 signaux — Commandes véhicule depuis UI :
| Signal | Description |
|--------|-------------|
| UI_lockRequest / unlockOnPark / walkAwayLock / walkUpUnlock | Verrouillage |
| UI_mirrorFoldRequest / mirrorHeatRequest / mirrorDipOnReverse | Rétroviseurs |
| UI_wiperRequest / wiperMode | Essuie-glaces |
| UI_frontLeftSeatHeatReq / frontRightSeatHeatReq | Sièges chauffants avant |
| UI_rearLeft/Center/RightSeatHeatReq | Sièges chauffants arrière |
| UI_displayBrightnessLevel | Luminosité (%) |
| UI_honkHorn / frunkRequest / summonActive | Klaxon, frunk, summon |
| UI_domeLightSwitch / frontFogSwitch / rearFogSwitch | Éclairage |
| UI_childDoorLockOn / alarmEnabled / intrusionSensorOn | Sécurité |
| UI_autoFoldMirrorsOn / autoHighBeamEnabled / ambientLightingEnabled | Automatismes |
| UI_powerOff / accessoryPowerRequest / driveStateRequest | Alimentation |
| UI_remoteStartRequest / remoteClosureRequest | Démarrage distant |

#### 0x3B3 (947) — UI_vehicleControl2 *(tesla_can.dbc)*
24 signaux — Commandes véhicule supplémentaires.

#### 0x334 (820) — UI_powertrainControl *(tesla_can.dbc)*
14 signaux — Contrôle groupe motopropulseur depuis UI.

#### 0x33A (826) — UI_rangeSOC *(tesla_can.dbc)*
5 signaux — Autonomie et SoC affichés.

#### 0x3BB (955) — UI_power *(tesla_can.dbc)*
2 signaux — Puissance UI.

#### 0x082 (130) — UI_tripPlanning *(tesla_can.dbc)*
| Signal | Factor | Unité | Description |
|--------|--------|-------|-------------|
| UI_tripPlanningActive | — | — | Planification trajet active |
| UI_navToSupercharger | — | — | Navigation vers Supercharger |
| UI_requestActiveBatteryHeating | — | — | Requête préchauffage |
| UI_predictedEnergy | 0.01 | kWh | Énergie prédite |
| UI_hindsightEnergy | 0.01 | kWh | Énergie rétrospective |
| UI_energyAtDestination | 0.01 | kWh | Énergie à destination |

#### 0x284 (644) — UIvehicleModes *(tesla_can.dbc)*
12 signaux — Modes véhicule.

#### 0x293 (659) — UI_chassisControl *(tesla_can.dbc)*
26 signaux — Contrôle châssis.

#### 0x313 (787) — UI_trackModeSettings *(tesla_can.dbc)*
7 signaux — Réglages mode piste.

---

### 4.16 Commandes au volant (STW / SCCM)

#### 0x045 (69) — STW_ACTN_RQ *(tesla_can2.dbc)*
| Signal | Description |
|--------|-------------|
| SpdCtrlLvr_Stat | Levier régulateur (IDLE/FWD/RWD/UP_1ST/UP_2ND/DN_1ST/DN_2ND) |
| VSL_Enbl_Rq | Requête activation limiteur |
| DTR_Dist_Rq | Distance ACC (1–7) |
| TurnIndLvr_Stat | Clignotant (LEFT/RIGHT/IDLE) |
| HiBmLvr_Stat | Appel de phares |
| WprWashSw_Psd | Lave-glace (TIPWIPE/WASH) |
| WprWash_R_Sw_Posn_V2 | Essuyage arrière |
| StW_Lvr_Stat | Manette volant (UP/DOWN/FWD/BACK) |
| StW_Cond_Psd / StW_Cond_Flt | Molette volant |
| HrnSw_Psd | Klaxon |
| StW_Sw00_Psd à StW_Sw15_Psd | 16 boutons volant |
| WprSw6Posn | Position essuie-glace (6 niveaux) |
**31 signaux.**

#### 0x06D (109) — SBW_RQ_SCCM *(tesla_can2.dbc)*
| Signal | Description |
|--------|-------------|
| StW_Sw_Stat3 | État commutateur 3 (PLUS/MINUS/PLUS_MINUS) |
| MsgTxmtId | ID transmetteur (EWM/SCCM) |
| TSL_RND_Posn_StW | Position sélecteur R/N/D (IDLE/R/N_UP/N_DOWN/INI/D) |
| TSL_P_Psd_StW | Bouton P appuyé |
**6 signaux.**

#### 0x249 (585) — SCCMLeftStalk *(tesla_can.dbc)*
6 signaux — Manette gauche (Model 3 refresh / Y).

#### 0x229 (553) — GearLever *(tesla_can.dbc)*
2 signaux — Sélecteur de rapport.

---

### 4.17 Thermique et HVAC

#### 0x282 (642) — VCLEFT_hvacBlowerFeedback *(tesla_can.dbc)* — Multiplexé
25 signaux — Retour ventilateur HVAC (température FET, courants phases, couple, état…).

#### 0x2F3 (755) — UI_hvacRequest *(tesla_can.dbc)*
11 signaux — Requêtes HVAC depuis UI.

#### 0x243 (579) — VCRIGHT_hvacStatus *(tesla_can.dbc)*
25 signaux — État HVAC côté droit.

#### 0x20C (524) — VCRIGHT_hvacRequest *(tesla_can.dbc)*
13 signaux — Requêtes HVAC côté droit.

#### 0x241 (577) — VCFRONT_coolant *(tesla_can.dbc)*
10 signaux — Circuit de refroidissement avant.

#### 0x227 (551) — CMP_state *(tesla_can.dbc)*
16 signaux — État compresseur climatisation.

#### 0x2A8 (680) — CMPD_state *(tesla_can.dbc)*
10 signaux — État compresseur (variante D).

#### 0x287 (647) — PTCcabinHeatSensorStatus *(tesla_can.dbc)*
7 signaux — Capteurs chauffage PTC habitacle.

#### 0x383 (899) — VCRIGHT_thsStatus *(tesla_can.dbc)*
7 signaux — État système thermique droit.

---

### 4.18 État système / GTW

#### 0x348 (840) — GTW_status *(tesla_can2.dbc)*
| Signal | Description |
|--------|-------------|
| GTW_driveRailReq / accRailReq / hvacRailReq | Requêtes rails alimentation |
| GTW_driveGoingDown / accGoingDown / hvacGoingDown | Rails en extinction |
| GTW_brakePressed | Frein appuyé |
| GTW_driverPresent / driverIsLeaving | Présence conducteur |
| GTW_notEnough12VForDrive | 12V insuffisant |
| GTW_icPowerOff | Extinction cluster |
| GTW_preconditionRequest | Requête préconditionning |
**14 signaux.**

#### 0x113 (275) — GTW_bmpDebug *(tesla_can.dbc)*
10 signaux — Debug BMP gateway (PMIC, PGOOD, reset…).

#### 0x3A1 (929) — VCFRONT_vehicleStatus *(tesla_can.dbc)*
25 signaux — État véhicule (ceintures, présence conducteur/passager, porte, 12V, état DI, charge HV…).

#### 0x2E1 (737) — VCFRONT_status *(tesla_can.dbc)*
74 signaux — État complet VCFRONT (multiplexé).

#### 0x381 (897) — VCFRONT_logging1Hz *(tesla_can.dbc)*
168 signaux — Logging à 1 Hz (multiplexé).

#### 0x301 (769) — VCFRONT_info *(tesla_can.dbc)*
21 signaux — Informations VCFRONT.

#### 0x201 (513) — VCFRONT_loggingAndVitals10Hz *(tesla_can.dbc)*
48 signaux — Logging/vitals à 10 Hz (multiplexé).

#### 0x221 (545) — VCFRONT_LVPowerState *(tesla_can.dbc)*
28 signaux — État alimentation basse tension avant.

#### 0x225 (549) — VCRIGHT_LVPowerState *(tesla_can.dbc)*
12 signaux — État alimentation basse tension droit.

#### 0x242 (578) — VCLEFT_LVPowerState *(tesla_can.dbc)*
8 signaux — État alimentation basse tension gauche.

#### 0x2F1 (753) — VCFRONT_eFuseDebugStatus *(tesla_can.dbc)*
99 signaux — Debug e-fuses (multiplexé).

#### 0x268 (616) — SystemPower *(tesla_can.dbc)*
5 signaux — Puissance système.

#### 0x261 (609) — 12vBattStatus *(tesla_can.dbc)*
29 signaux — État batterie 12V complet.

#### 0x528 (1320) — UnixTime *(tesla_can.dbc)*
1 signal — Timestamp Unix.

#### 0x405 (1029) — VIN *(tesla_can.dbc)*
4 signaux — Numéro d'identification véhicule (multiplexé).

#### 0x7FF (2047) — carConfig *(tesla_can.dbc)*
73 signaux — Configuration véhicule complète (multiplexé).

#### 0x743 (1859) — VCRIGHT_recallStatus *(tesla_can.dbc)*
3 signaux — État rappels.

---

### 4.19 Capteurs de stationnement (PARK)

#### 0x20E (526) — PARK_sdiFront *(tesla_can.dbc)*
| Signal | Unité | Description |
|--------|-------|-------------|
| PARK_sdiSensor1–6RawDistData | cm | Distance brute capteurs 1–6 avant (0–511 cm) |
| PARK_sdiFrontCounter | — | Compteur |
| PARK_sdiFrontChecksum | — | Checksum |

#### 0x22E (558) — PARK_sdiRear *(tesla_can.dbc)*
| Signal | Unité | Description |
|--------|-------|-------------|
| PARK_sdiSensor7–12RawDistData | cm | Distance brute capteurs 7–12 arrière (0–511 cm) |
| PARK_sdiRearCounter / Checksum | — | Sécurité trame |

---

### 4.20 Alertes UI

#### 0x123 (291) — UI_alertMatrix1 *(tesla_can.dbc)*
**64 alertes booléennes** (1 bit chacune) :
| Signal | Description |
|--------|-------------|
| UI_a001_DriverDoorOpen | Porte conducteur ouverte |
| UI_a002_DoorOpen | Porte ouverte |
| UI_a003_TrunkOpen | Coffre ouvert |
| UI_a004_FrunkOpen | Frunk ouvert |
| UI_a005_HeadlightsOnDoorOpen | Phares ON + porte ouverte |
| UI_a013–017_TPMS* | Alertes TPMS (Hard/Soft/Over/Temp/Fault) |
| UI_a018_SlipStartOn | Démarrage glissant |
| UI_a019_ParkBrakeFault | Défaut frein parking |
| UI_a020_SteeringReduced | Direction réduite |
| UI_a021_RearSeatbeltUnbuckled | Ceinture AR non bouclée |
| UI_a027_AutoSteerMIA | Autosteer absent |
| UI_a031_BrakeFluidLow | Liquide frein bas |
| UI_a046_BackupCameraStreamError | Erreur flux caméra recul |
| UI_a049_BrakeShiftRequired | Frein requis pour P/R/N/D |
| UI_a052_KernelPanicReported | Kernel panic |
| UI_a055–060_ECall* | Alertes eCall |
| UI_a057–058_AntennaDisconnected | Antennes déconnectées |
| + 30 autres alertes système | … |

---

### 4.21 Divers

#### 0x3C2 (962) — VCLEFT_switchStatus *(tesla_can.dbc)*
61 signaux — État de tous les interrupteurs côté gauche.

#### 0x3C3 (963) — VCRIGHT_switchStatus *(tesla_can.dbc)*
39 signaux — État interrupteurs côté droit.

#### 0x336 (822) — MaxPowerRating *(tesla_can.dbc)*
3 signaux — Puissance max nominale.

#### 0x3FE (1022) — brakeTemps *(tesla_can.dbc)*
4 signaux — Températures de freins.

#### 0x281 (641) — VCFRONT_CMPRequest *(tesla_can.dbc)*
4 signaux — Requête compresseur.

---

## 5. Tableau récapitulatif des messages

### tesla_can.dbc / Model3CAN.dbc (159 messages)

| CAN ID (hex) | CAN ID (dec) | Nom | DLC | Bus | Signaux | Catégorie |
|:---:|:---:|---|:---:|:---:|:---:|---|
| 0x00C | 12 | UI_status | 8 | Vehicle | 25 | UI |
| 0x016 | 22 | DI_bmsRequest | 1 | Vehicle | 2 | BMS |
| 0x04F | 79 | GPSLatLong | 8 | Chassis | 3 | GPS |
| 0x082 | 130 | UI_tripPlanning | 8 | Vehicle | 6 | UI/Navigation |
| 0x101 | 257 | RCM_inertial1 | 8 | Chassis | 8 | IMU |
| 0x102 | 258 | VCLEFT_doorStatus | 8 | Vehicle | 20 | Carrosserie |
| 0x103 | 259 | VCRIGHT_doorStatus | 8 | Vehicle | 21 | Carrosserie |
| 0x108 | 264 | DIR_torque | 8 | Vehicle | 7 | Moteur AR |
| 0x111 | 273 | RCM_inertial2 | 8 | Chassis | 8 | IMU |
| 0x113 | 275 | GTW_bmpDebug | 3 | Vehicle | 10 | Système |
| 0x116 | 278 | RCM_inertial2New | 8 | Chassis | 8 | IMU |
| 0x118 | 280 | DriveSystemStatus | 8 | Vehicle | 14 | Traction |
| 0x119 | 281 | VCSEC_windowRequests | 2 | Vehicle | 7 | Carrosserie |
| 0x122 | 290 | VCLEFT_doorStatus2 | 6 | Vehicle | 12 | Carrosserie |
| 0x123 | 291 | UI_alertMatrix1 | 8 | Vehicle | 64 | Alertes |
| 0x126 | 294 | RearHVStatus | 7 | Vehicle | 6 | HV |
| 0x127 | 295 | LeftRearHVStatus | 7 | Vehicle | 6 | HV |
| 0x129 | 297 | SteeringAngle | 8 | Vehicle | 5 | Direction |
| 0x12A | 298 | RightRearHVStatus | 7 | Vehicle | 6 | HV |
| 0x132 | 306 | HVBattAmpVolt | 8 | Vehicle | 4 | Batterie |
| 0x13D | 317 | CP_chargeStatus | 6 | Vehicle | 8 | Charge |
| 0x142 | 322 | VCLEFT_liftgateStatus | 8 | Vehicle | 13 | Carrosserie |
| 0x145 | 325 | ESP_status | 8 | Chassis | 24 | Freinage |
| 0x154 | 340 | RearTorqueOld | 8 | Vehicle | 1 | Moteur AR |
| 0x155 | 341 | WheelAngles | 8 | Chassis | 13 | Direction |
| 0x175 | 373 | WheelSpeed | 8 | Chassis | 6 | Roues |
| 0x185 | 389 | ESP_brakeTorque | 8 | Chassis | 7 | Freinage |
| 0x186 | 390 | DIF_torque | 8 | Vehicle | 7 | Moteur AV |
| 0x1A5 | 421 | FrontHVStatus | 7 | Vehicle | 6 | HV |
| 0x1D4 | 468 | FrontTorqueOld | 8 | Vehicle | 1 | Moteur AV |
| 0x1D5 | 469 | FrontTorque | 8 | Vehicle | 2 | Moteur AV |
| 0x1D6 | 470 | DI_limits | 5 | Vehicle | 38 | Limites DI |
| 0x1D8 | 472 | RearTorque | 8 | Vehicle | 5 | Moteur AR |
| 0x201 | 513 | VCFRONT_loggingAndVitals10Hz | 8 | Vehicle | 48 | Système |
| 0x204 | 516 | PCS_chgStatus | 8 | Vehicle | 14 | Charge |
| 0x20A | 522 | HVP_contactorState | 6 | Vehicle | 22 | HV |
| 0x20C | 524 | VCRIGHT_hvacRequest | 8 | Vehicle | 13 | HVAC |
| 0x20E | 526 | PARK_sdiFront | 8 | Chassis | 8 | Parking |
| 0x212 | 530 | BMS_status | 8 | Vehicle | 22 | BMS |
| 0x214 | 532 | FastChargeVA | 8 | Vehicle | 9 | Charge rapide |
| 0x215 | 533 | FCisolation | 1 | Vehicle | 1 | Charge rapide |
| 0x217 | 535 | FC_status3 | 8 | Vehicle | 8 | Charge rapide |
| 0x219 | 537 | VCSEC_TPMSData | 5 | Chassis | 17 | TPMS |
| 0x21D | 541 | CP_evseStatus | 8 | Vehicle | 19 | Charge |
| 0x221 | 545 | VCFRONT_LVPowerState | 8 | Vehicle | 28 | Système |
| 0x224 | 548 | PCSDCDCstatus | 8 | Vehicle | 17 | DC-DC |
| 0x225 | 549 | VCRIGHT_LVPowerState | 3 | Vehicle | 12 | Système |
| 0x227 | 551 | CMP_state | 8 | Vehicle | 16 | HVAC |
| 0x228 | 552 | EPBrightStatus | 8 | Vehicle | 15 | EPB |
| 0x229 | 553 | GearLever | 3 | Vehicle | 2 | Traction |
| 0x22A | 554 | HVP_pcsControl | 4 | Vehicle | 5 | HV |
| 0x22E | 558 | PARK_sdiRear | 8 | Vehicle | 8 | Parking |
| 0x232 | 562 | BMS_contactorRequest | 8 | Vehicle | 7 | BMS |
| 0x238 | 568 | UI_driverAssistMapData | 8 | Chassis | 30 | Autopilot |
| 0x239 | 569 | DAS_lanes | 8 | Chassis | 13 | Autopilot |
| 0x241 | 577 | VCFRONT_coolant | 7 | Vehicle | 10 | Thermique |
| 0x242 | 578 | VCLEFT_LVPowerState | 2 | Vehicle | 8 | Système |
| 0x243 | 579 | VCRIGHT_hvacStatus | 8 | Vehicle | 25 | HVAC |
| 0x244 | 580 | FastChargeLimits | 8 | Vehicle | 4 | Charge rapide |
| 0x249 | 585 | SCCMLeftStalk | 4 | Vehicle | 6 | Commandes |
| 0x24A | 586 | DAS_visualDebug | 8 | Chassis | 24 | Autopilot |
| 0x252 | 594 | BMS_powerAvailable | 8 | Vehicle | 6 | BMS |
| 0x257 | 599 | DIspeed | 8 | Vehicle | 6 | Vitesse |
| 0x25B | 603 | APP_environment | 1 | Chassis | 2 | Environnement |
| 0x25D | 605 | CP_status | 8 | Vehicle | 29 | Charge |
| 0x261 | 609 | 12vBattStatus | 8 | Vehicle | 29 | Système |
| 0x264 | 612 | ChargeLineStatus | 8 | Vehicle | 4 | Charge |
| 0x266 | 614 | RearInverterPower | 8 | Vehicle | 6 | Puissance |
| 0x267 | 615 | DI_vehicleEstimates | 8 | Vehicle | 10 | Dynamique |
| 0x268 | 616 | SystemPower | 5 | Vehicle | 5 | Système |
| 0x269 | 617 | LeftRearPower | 8 | Vehicle | 6 | Puissance |
| 0x273 | 627 | UI_vehicleControl | 8 | Vehicle | 38 | UI |
| 0x27C | 636 | RightRearPower | 8 | Vehicle | 6 | Puissance |
| 0x27D | 637 | CP_dcChargeLimits | 8 | Vehicle | 5 | Charge DC |
| 0x281 | 641 | VCFRONT_CMPRequest | 8 | Vehicle | 4 | HVAC |
| 0x282 | 642 | VCLEFT_hvacBlowerFeedback | 8 | Vehicle | 25 | HVAC |
| 0x284 | 644 | UIvehicleModes | 5 | Vehicle | 12 | UI |
| 0x287 | 647 | PTCcabinHeatSensorStatus | 8 | Vehicle | 7 | HVAC |
| 0x288 | 648 | EPBleftStatus | 8 | Vehicle | 15 | EPB |
| 0x292 | 658 | BMS_SOC | 8 | Vehicle | 6 | BMS |
| 0x293 | 659 | UI_chassisControl | 8 | Vehicle | 26 | Châssis |
| 0x29D | 669 | CP_dcChargeStatus | 4 | Vehicle | 3 | Charge DC |
| 0x2A8 | 680 | CMPD_state | 8 | Vehicle | 10 | HVAC |
| 0x2B4 | 692 | PCS_dcdcRailStatus | 5 | Vehicle | 3 | DC-DC |
| 0x2B6 | 694 | DI_chassisControlStatus | 2 | Vehicle | 8 | Châssis |
| 0x2B9 | 697 | DAS_control | 8 | Chassis | 9 | Autopilot |
| 0x2BD | 701 | CP_dcPowerLimits | 4 | Vehicle | 2 | Charge DC |
| 0x2D2 | 722 | BMSVAlimits | 8 | Vehicle | 4 | BMS |
| 0x2D3 | 723 | UI_solarData | 8 | Chassis | 7 | Environnement |
| 0x2E1 | 737 | VCFRONT_status | 8 | Vehicle | 74 | Système |
| 0x2E5 | 741 | FrontInverterPower | 8 | Vehicle | 6 | Puissance |
| 0x2E6 | 742 | PlaidFrontPower | 8 | Vehicle | 6 | Puissance |
| 0x2F1 | 753 | VCFRONT_eFuseDebugStatus | 8 | Vehicle | 99 | Système |
| 0x2F3 | 755 | UI_hvacRequest | 5 | Vehicle | 11 | HVAC |
| 0x300 | 768 | BMS_info | 8 | Vehicle | 15 | BMS |
| 0x301 | 769 | VCFRONT_info | 8 | Vehicle | 21 | Système |
| 0x309 | 777 | DAS_object | 8 | Vehicle | 57 | Autopilot |
| 0x312 | 786 | BMSthermal | 8 | Vehicle | 9 | BMS |
| 0x313 | 787 | UI_trackModeSettings | 8 | Vehicle | 7 | UI |
| 0x315 | 789 | RearInverterTemps | 8 | Vehicle | 8 | Thermique |
| 0x318 | 792 | SystemTimeUTC | 8 | Vehicle | 6 | Temps |
| 0x31C | 796 | CC_chgStatus | 8 | Vehicle | 9 | Charge |
| 0x31F | 799 | TPMSsensors | 8 | Chassis | 8 | TPMS |
| 0x321 | 801 | VCFRONT_sensors | 8 | Vehicle | 9 | Capteurs |
| 0x332 | 818 | BattBrickMinMax | 6 | Vehicle | 11 | BMS |
| 0x333 | 819 | UI_chargeRequest | 5 | Vehicle | 11 | Charge |
| 0x334 | 820 | UI_powertrainControl | 8 | Vehicle | 14 | UI |
| 0x335 | 821 | RearDIinfo | 8 | Vehicle | 19 | Moteur AR |
| 0x336 | 822 | MaxPowerRating | 4 | Vehicle | 3 | Système |
| 0x33A | 826 | UI_rangeSOC | 8 | Vehicle | 5 | UI |
| 0x352 | 850 | BMS_energyStatus | 8 | Vehicle | 7 | BMS |
| 0x353 | 851 | UI_status (2) | 8 | Vehicle | 25 | UI |
| 0x376 | 886 | FrontInverterTemps | 8 | Vehicle | 8 | Thermique |
| 0x37D | 893 | CP_thermalStatus | 7 | Vehicle | 5 | Charge |
| 0x381 | 897 | VCFRONT_logging1Hz | 8 | Vehicle | 168 | Système |
| 0x383 | 899 | VCRIGHT_thsStatus | 8 | Vehicle | 7 | HVAC |
| 0x389 | 905 | DAS_status2 | 8 | Chassis | 18 | Autopilot |
| 0x392 | 914 | BMS_packConfig | 8 | Vehicle | 5 | BMS |
| 0x395 | 917 | DIR_oilPump | 8 | Vehicle | 10 | Moteur AR |
| 0x396 | 918 | FrontOilPump | 8 | Vehicle | 10 | Moteur AV |
| 0x399 | 921 | DAS_status | 8 | Chassis | 26 | Autopilot |
| 0x39D | 925 | IBST_status | 5 | Chassis | 6 | Freinage |
| 0x3A1 | 929 | VCFRONT_vehicleStatus | 8 | Vehicle | 25 | Système |
| 0x3B3 | 947 | UI_vehicleControl2 | 4 | Vehicle | 24 | UI |
| 0x3B6 | 950 | odometer | 4 | Vehicle | 1 | Odomètre |
| 0x3BB | 955 | UI_power | 2 | Vehicle | 2 | UI |
| 0x3C2 | 962 | VCLEFT_switchStatus | 8 | Vehicle | 61 | Commandes |
| 0x3C3 | 963 | VCRIGHT_switchStatus | 7 | Vehicle | 39 | Commandes |
| 0x3D2 | 978 | TotalChargeDischarge | 8 | Vehicle | 2 | BMS |
| 0x3D8 | 984 | Elevation | 2 | Vehicle | 1 | GPS |
| 0x3D9 | 985 | UI_gpsVehicleSpeed | 8 | Chassis | 11 | GPS |
| 0x3E2 | 994 | VCLEFT_lightStatus | 7 | Vehicle | 23 | Éclairage |
| 0x3E3 | 995 | VCRIGHT_lightStatus | 2 | Vehicle | 6 | Éclairage |
| 0x3E9 | 1001 | DAS_bodyControls | 8 | Vehicle | 14 | Autopilot |
| 0x3F2 | 1010 | BMSCounters | 8 | Vehicle | 21 | BMS |
| 0x3F3 | 1011 | UI_odo | 3 | Chassis | 1 | Odomètre |
| 0x3F5 | 1013 | VCFRONT_lighting | 8 | Vehicle | 28 | Éclairage |
| 0x3F8 | 1016 | UI_driverAssistControl | 8 | Chassis | 42 | Autopilot |
| 0x3FD | 1021 | UI_autopilotControl | 8 | Chassis | 38 | Autopilot |
| 0x3FE | 1022 | brakeTemps | 5 | Vehicle | 4 | Freinage |
| 0x401 | 1025 | BrickVoltages | 8 | Vehicle | 139 | BMS |
| 0x405 | 1029 | VIN | 8 | Vehicle | 4 | Système |
| 0x42A | 1066 | VCSEC_TPMSConnectionData | 8 | Vehicle | 16 | TPMS |
| 0x43D | 1085 | CP_chargeStatusLog | 6 | Vehicle | 8 | Charge |
| 0x51E | 1310 | FC_info | 8 | Vehicle | 43 | Charge rapide |
| 0x528 | 1320 | UnixTime | 4 | Vehicle | 1 | Temps |
| 0x541 | 1345 | FastChargeMaxLimits | 8 | Vehicle | 2 | Charge rapide |
| 0x556 | 1366 | FrontDItemps | 7 | Vehicle | 7 | Thermique |
| 0x557 | 1367 | FrontThermalControl | 4 | Vehicle | 4 | Thermique |
| 0x5D5 | 1493 | RearDItemps | 5 | Vehicle | 5 | Thermique |
| 0x5D7 | 1495 | RearThermalControl | 4 | Vehicle | 4 | Thermique |
| 0x656 | 1622 | FrontDIinfo | 8 | Vehicle | 19 | Moteur AV |
| 0x72A | 1834 | BMS_serialNumber | 8 | Vehicle | 15 | BMS |
| 0x743 | 1859 | VCRIGHT_recallStatus | 1 | Vehicle | 3 | Système |
| 0x757 | 1879 | DIF_debug | 8 | Vehicle | 131 | Debug |
| 0x75D | 1885 | CP_sensorData | 8 | Vehicle | 44 | Charge |
| 0x7D5 | 2005 | DIR_debug | 8 | Vehicle | 131 | Debug |
| 0x7FF | 2047 | carConfig | 8 | Vehicle | 73 | Configuration |

### tesla_can2.dbc (19 messages supplémentaires/différents)

| CAN ID (hex) | CAN ID (dec) | Nom | DLC | Émetteur | Signaux | Catégorie |
|:---:|:---:|---|:---:|:---:|:---:|---|
| 0x003 | 3 | STW_ANGL_STAT | 8 | STW | 6 | Direction |
| 0x00E | 14 | STW_ANGLHP_STAT | 8 | STW | 6 | Direction |
| 0x045 | 69 | STW_ACTN_RQ | 8 | STW | 31 | Commandes volant |
| 0x06D | 109 | SBW_RQ_SCCM | 4 | STW | 6 | Sélecteur |
| 0x101 | 257 | GTW_epasControl | 3 | NEO | 7 | Direction |
| 0x108 | 264 | DI_torque1 | 8 | DI | 7 | Moteur |
| 0x118 | 280 | DI_torque2 | 6 | DI | 11 | Moteur/Vitesse |
| 0x135 | 309 | ESP_135h | 5 | ESP | 19 | Freinage |
| 0x155 | 341 | ESP_B | 8 | ESP | 8 | Vitesse/ESP |
| 0x214 | 532 | EPB_epasControl | 3 | EPB | 3 | Frein parking |
| 0x283 | 643 | BODY_R1 | 8 | GTW | 31 | Carrosserie |
| 0x2F8 | 760 | MCU_gpsVehicleSpeed | 8 | MCU | 7 | GPS |
| 0x318 | 792 | GTW_carState | 8 | GTW | 16 | État véhicule |
| 0x348 | 840 | GTW_status | 8 | GTW | 14 | Système |
| 0x368 | 872 | DI_state | 8 | DI | 15 | État DI |
| 0x370 | 880 | EPAS_sysStatus | 8 | EPAS | 11 | Direction |
| 0x388 | 904 | MCU_clusterBacklightRequest | 3 | NEO | 4 | UI |
| 0x3D8 | 984 | MCU_locationStatus | 8 | MCU | 3 | GPS |
| 0x488 | 1160 | DAS_steeringControl | 4 | NEO | 5 | Direction |

---

## 6. Recoupements entre fichiers

### IDs communs entre tesla_can.dbc et tesla_can2.dbc

| CAN ID | tesla_can.dbc | tesla_can2.dbc | Notes |
|:---:|---|---|---|
| 0x101 (257) | RCM_inertial1 (IMU) | GTW_epasControl | **Contenu différent** — même ID, bus différents |
| 0x108 (264) | DIR_torque | DI_torque1 | Signaux similaires, nommage différent |
| 0x118 (280) | DriveSystemStatus | DI_torque2 | Signaux similaires, nommage différent |
| 0x145 (325) | ESP_status | — (VBOX uniquement) | Structures compatibles |
| 0x155 (341) | WheelAngles | ESP_B | **Contenu différent** |
| 0x214 (532) | FastChargeVA | EPB_epasControl | **Contenu différent** |
| 0x318 (792) | SystemTimeUTC | GTW_carState | Complémentaires (temps + état) |
| 0x3D8 (984) | Elevation | MCU_locationStatus | **Contenu différent** |

> **Important :** Les CAN IDs identiques entre les deux DBC correspondent souvent à des signaux sur des bus différents (ChassisBus vs VehicleBus). Il ne s'agit pas de conflits mais de multiplexage inter-bus via le Gateway.

### Signaux VBOX vs DBC

Le fichier `Tesla-Model 3.md` fournit des versions simplifiées de signaux existants dans les DBC :
- Yaw_Rate → RCM_yawRate (0x101)
- Engine_Torque → DI_torqueMotor (0x108)
- Engine_Speed → DI_motorRPM (0x108)
- Wheel_Speed_* → WheelSpeed (0x175)
- Brake_Position → ESP_status/brakeTorque (0x145/0x185)
- Door_Open_* → GTW_carState (0x318)
- Tyre_* → TPMSsensors (0x31F)
- Air_Temperature → VCFRONT_sensors (0x321)
- Steering_Angle → DAS_steeringControl (0x488)
- Accelerator_Pedal_Position → DI_pedalPos (0x108)

---

## 7. Notes et conventions

### Format des signaux DBC
```
SG_ <nom> : <bitStart>|<bitLength>@<endian><signed> (<factor>,<offset>) [<min>|<max>] "<unité>" <récepteurs>
```
- `@1+` = Little-Endian, unsigned
- `@1-` = Little-Endian, signed
- `@0+` = Big-Endian (Motorola), unsigned
- Valeur physique = `raw × factor + offset`

### Signaux multiplexés
Certains messages utilisent le multiplexage (signal M = multiplexeur, m0/m1/m2… = valeurs multiplexées) :
- `DAS_object` (0x309) : 6 index → objets lead, left, right, cutin, road signs, headings
- `BrickVoltages` (0x401) : 139 signaux pour toutes les tensions de briques
- `carConfig` (0x7FF) : 73 signaux de configuration
- `DIR_debug` / `DIF_debug` : 131 signaux chacun
- `VCFRONT_logging1Hz` : 168 signaux

### Checksums et compteurs
Quasi tous les messages critiques incluent un couple `Counter` (3–4 bits) + `Checksum` (8 bits) pour la sécurité de la communication CAN.

### Particularités Tesla Model 3
- **Pas de bus CAN OBD-II standard** — accès via le port diagnostique ou interception sur les bus internes
- **DLC = 8 octets** systématiquement pour la majorité des messages
- **Littoral Endian (Intel)** dominant, quelques exceptions en Motorola (Big-Endian)
- **Signaux redondants** — certains signaux existent sur plusieurs messages/bus pour la tolérance aux pannes
- Les vitesses de roue existent en **4 facteurs différents** dans la référence VBOX
- Support **Model 3 Plaid** via messages de puissance séparés (LeftRear/RightRear/PlaidFront)
