#pragma once

/**
 * @file firmware/lib/transport/can/handler/variant/bits.h
 * @brief Named bit-position constants for FSD/DAS, UI driver assist, nag suppress,
 *        ISA speed chime, and HW4 speed offset fields.
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 *
 * Bit positions are shared across HW3, HW4, and Legacy variant handlers. Defining
 * them here avoids magic numbers in the variant dispatch code and makes the
 * intent of each setBit()/bit-mask operation self-documenting.
 *
 * Source of truth for numeric values is the variant files under
 * firmware/lib/vehicle/can/handler/variant/.
 */

// 0x3FD (FSD mux 0) bit indices
#define FSD_BIT_AP_ACTIVE              38
#define FSD_BIT_CONTINUE_ON_GREEN      39
#define FSD_BIT_EAP                    46
#define FSD_BIT_EVD                    59
#define FSD_BIT_DAS_DEV                60

// 0x3FD (FSD mux 1) nag suppress bit indices
#define FSD_BIT_NAG_ECE_R79            20
#define FSD_BIT_NAG_ORGANIC            47	 // also acts as "Summon EU unlock"

// TLSSC autopilot tier value written into FSD frame byte 0 lower 6 bits
#define FSD_BIT_TLSSC                  0x1B	 // SELF_DRIVING tier (v12/v13 spec)

// 0x3FD/0x3F8 UI driver assist bit indices
#define UI_BIT_DAS_DEVELOPER            5
#define UI_BIT_DRIVE_ON_MAPS           13
#define UI_BIT_HANDS_ON_REQ_DISABLE    14
#define UI_BIT_LANE_GRAPH              45
#define UI_BIT_TRIP_TELEMETRY          43
#define UI_BIT_HAS_DRIVE_ON_NAV        48
#define UI_BIT_FOLLOW_NAV_ROUTE        49

// 0x3FD nag suppress
#define NAG_BIT_HANDS_ON_REQUIREMENT   19

// 0x3FD driving-side bit
#define FSD_BIT_DRIVING_SIDE           41

// 0x399 ISA speed chime suppression
#define ISA_CHIME_SUPPRESS_BYTE         1
#define ISA_CHIME_SUPPRESS_BIT          5
#define ISA_CHIME_SUPPRESS_MASK         0x20

// HW4 speed offset field encoding in FSD frame byte 1
#define HW4_OFFSET_PRESERVE_MASK        0xC0
#define HW4_OFFSET_FIELD_MASK           0x3F

// 0x370 EPAS torque spoofing (nag killer echo)
#define EPAS_TORQUE_BYTE_2              2
#define EPAS_TORQUE_BYTE_3              3
