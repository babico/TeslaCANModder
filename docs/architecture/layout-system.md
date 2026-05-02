---
title: Layout System
description: Layout specification for each breakpoint, theme, and display mode of the client/ drive cluster
category: architecture
folder: architecture
tags: [responsive, layout, ui]
order: 18
---

# Responsive Drive Layout Variants (D-15)

This document specifies the concrete layout contract for each breakpoint × display-mode combination in `DriveScreen.tsx`, `AppExperience.tsx`, and the component library. It references `client/src/design/tokens.ts` for all spacing, radius, and breakpoint values, and `useBreakpoint` for responsive switching.

---

## 1. Canonical Layout Zones

All variants share the same named zones. Their sizes and arrangements change per breakpoint.

```
┌────────────────────────────────────────────────────────┐
│ TOP RAIL   │ status badge │ AP indicator │ variant tag  │
├────────────────────────────────────────────────────────┤
│ LEFT PANEL │ CENTER CLUSTER             │ RIGHT PANEL  │
│ power/regen│ speedometer + gear + pedal │ SOC bar      │
├────────────────────────────────────────────────────────┤
│ LOWER BAND │ BMS quick stats │ drive mode badge         │
└────────────────────────────────────────────────────────┘
```

---

## 2. Breakpoint Definitions

From `tokens.breakpoints`:

| Name      | Min Width (px) | Description                                  |
| --------- | -------------- | -------------------------------------------- |
| `phone`   | 0              | Phone portrait, single column                |
| `phoneLS` | 568            | Phone landscape, compact two-column          |
| `cluster` | 480            | Embedded cluster display (fixed 480px width) |
| `tablet`  | 768            | Tablet, two-column + sidebar visible         |
| `desktop` | 980            | Desktop / wide — three columns               |

---

## 3. Layout Variants

### 3.1 `phone` — Single Column (< 568px)

```
┌──────────────────────────┐
│       TOP RAIL           │
├──────────────────────────┤
│     CENTER CLUSTER       │
│  speedometer (260dp dia) │
│   gear row     pedal     │
├──────────────────────────┤
│ stat chips (1×3 row)     │
│ [power]  [SOC]  [range]  │
├──────────────────────────┤
│       LOWER BAND         │
└──────────────────────────┘
```

**Constraints:**

- `DriveScreen` root: `flex: 1`, `backgroundColor: dashTokens.bg`
- `topRail`: height 44dp, horizontal padding `spacing.md` (12dp)
- `centerCluster`: `alignItems: center`, `paddingVertical: spacing.xl` (24dp)
- Speedometer diameter: **260dp** — controlled by `gaugeSize` prop (default for phone)
- `statChips`: `flexDirection: row`, `flexWrap: wrap`, gap `spacing.sm` (8dp)
- Left panel / right panel are collapsed into stat chips below center cluster
- `lowerBand`: height 48dp, `flexDirection: row`, padding `spacing.md`

---

### 3.2 `phoneLS` — Phone Landscape (568–767px)

```
┌────────────────────────────────────────────┐
│               TOP RAIL                     │
├──────────┬─────────────────┬───────────────┤
│  LEFT    │  CENTER CLUSTER │  RIGHT        │
│ (power)  │  speedo 200dp   │  (SOC+range)  │
│          │  gear + pedal   │               │
├──────────┴─────────────────┴───────────────┤
│              LOWER BAND                    │
└────────────────────────────────────────────┘
```

**Constraints:**

- Three-column `flexDirection: row` layout
- Left panel width: 80dp, right panel width: 80dp, center: `flex: 1`
- Speedometer diameter: **200dp**
- `lowerBand`: height 40dp (compact padding `spacing.sm`)
- No stat-chips row (panels are visible)

---

### 3.3 `cluster` — Embedded Display (≥ 480px, fixed aspect ≈ 480×320)

```
┌──────────────────────────────────────────────┐
│  TOP RAIL (condensed, 36dp)                  │
├───────┬──────────────────────────┬───────────┤
│ LEFT  │   CENTER  speedo 180dp   │  RIGHT    │
│ 64dp  │   gear badges (compact)  │  64dp     │
├───────┴──────────────────────────┴───────────┤
│  LOWER BAND (32dp, dense density)            │
└──────────────────────────────────────────────┘
```

**Constraints:**

- Fixed container: `width: 480, height: 320` — embedded in a vehicle HMI frame
- Density tier: `density.dense` (label 10px, value 11px, padding xs)
- Speedometer diameter: **180dp**
- Font scale: `font.size.xs` for all labels, `font.size.sm` for values
- No BSM/turn-signal animations (cluster mode disables pulse timers to save CPU)
- Top rail height: 36dp

---

### 3.4 `tablet` — Two-Column + Visible Sidebar (768–979px)

```
┌────────────────────────────────────────────────────────────┐
│                      TOP RAIL (48dp)                       │
├────────────────────────────────────────────────────────────┤
│       MAIN PANEL (flex:1)           │   SIDEBAR (240dp)    │
│ ┌─────────────────────────────────┐ │   [ Controls card  ] │
│ │  LEFT   CENTER speedo  RIGHT    │ │   [ Battery stats  ] │
│ │ 100dp   280dp        100dp     │ │   [ Signal chart   ] │
│ └─────────────────────────────────┘ │                      │
├─────────────────────────────────────┴──────────────────────┤
│                    LOWER BAND (52dp)                       │
└────────────────────────────────────────────────────────────┘
```

**Constraints:**

- Root: `flexDirection: row`
- `mainPanel`: `flex: 1`
- `sidebar`: `width: 240`, `paddingLeft: spacing.md`, separate scroll area
- Speedometer diameter: **280dp**
- Density tier: `density.compact`
- Sidebar renders a `<Card>` per logical group (controls, battery, signal chart)
- Sidebar is hidden if `sidebarVisible` prop is false (default true on tablet)

---

### 3.5 `desktop` — Three-Column (≥ 980px)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                             TOP RAIL (52dp)                                 │
├───────────────────┬─────────────────────────────────┬───────────────────────┤
│  LEFT PANEL       │       CENTER CLUSTER             │   RIGHT PANEL         │
│  (200dp)          │  speedo 340dp + gear + pedal     │   (280dp)             │
│  - power gauge    │  centered with large whitespace  │   - SOC bar           │
│  - regen arc      │                                  │   - range             │
│  - kW badge       │                                  │   - battery temp      │
├───────────────────┴─────────────────────────────────┴───────────────────────┤
│                             LOWER BAND (56dp)                               │
│  drive mode │ BSM indicators │ turn signals │ AP tier │ variant badge        │
└─────────────────────────────────────────────────────────────────────────────┘
```

**Constraints:**

- Root: `flexDirection: row`
- `leftPanel`: `width: 200`, `paddingRight: spacing.md`
- `centerCluster`: `flex: 1`, max-width 560dp (centred)
- `rightPanel`: `width: 280`, `paddingLeft: spacing.md`
- Speedometer diameter: **340dp**
- Density tier: `density.default` (most comfortable reading distance)
- Lower band shows all status indicators in a horizontal row

---

## 4. Gauge Mode Variants

The `useGaugeMode()` hook returns one of: `"full"`, `"speed-only"`, `"numeric"`.

| Mode         | Speedometer Rendered         | Notes                                      |
| ------------ | ---------------------------- | ------------------------------------------ |
| `full`       | Arc + needle + legend zones  | Default                                    |
| `speed-only` | Arc + needle, no zone legend | Compact cluster / embedded                 |
| `numeric`    | Large number + unit only     | Accessibility preference or very low width |

Gauge mode is independent of breakpoint but defaults per breakpoint:

- `phone`: `full`
- `cluster`: `speed-only`
- `tablet` / `desktop`: `full`

---

## 5. Theme Variants

Both light and dark themes use the same layout dimensions; only colours change.

| Token group   | Dark                | Light               |
| ------------- | ------------------- | ------------------- |
| `bg`          | `navy900` (#0a1628) | `slate50` (#f8fafc) |
| `bgCard`      | `navy850` (#0d1e35) | `white`             |
| `value`       | `white`             | `slate900`          |
| `label`       | `#8baec8`           | `slate500`          |
| `dashPrimary` | `cyan400`           | `cyan600`           |

Switching is controlled by `useTheme()` which reads system preference + user override.

---

## 6. Implementation Notes for `DriveScreen.tsx`

1. **`useBreakpoint()`** returns one of: `"phone" | "phoneLS" | "cluster" | "tablet" | "desktop"`. Pass it to a `layoutForBreakpoint()` helper that returns a `LayoutConfig` object.

2. **`LayoutConfig`** should contain:

```ts
interface LayoutConfig {
    gaugeSize: number; // speedometer diameter
    showSidebar: boolean;
    showLeftPanel: boolean;
    showStatChips: boolean;
    densityTier: keyof typeof density;
    topRailHeight: number;
    lowerBandHeight: number;
}
```

1. All panel width values come from `tokens.spacing` or are hardcoded as local constants — never inline magic numbers in component JSX.

2. Transition between breakpoints must NOT cause layout thrash — use `useMemo` on `LayoutConfig` derived from breakpoint.

3. `cluster` mode adds `pointerEvents: "none"` to the root (no touch interactions in embedded HMI context).

---

## 7. Completion Criteria

- [ ] `useBreakpoint()` returns correct breakpoint for all 5 values
- [ ] `DriveScreen` renders without overflow at each breakpoint (snapshot)
- [ ] `LayoutConfig` object documented and tested in `DriveScreen.responsive.test.tsx`
- [ ] `density` tier applied correctly per breakpoint
- [ ] `theme` applies correct background/foreground per mode
- [ ] Visual regression signs off the relevant VR rows in `docs/checklists/testing-plan.md#ui-1-visual-regression-coverage`

---

## References

- `client/src/screens/DriveScreen.tsx` — primary implementation
- `client/src/state/useBreakpoint.ts` — breakpoint detection hook
- `client/src/design/tokens.ts` — `breakpoints`, `spacing`, `density`, `dashTokens`
- `docs/checklists/testing-plan.md#ui-1-visual-regression-coverage` — VR test IDs cross-referenced above
- `docs/reference/state-fields.md` — signal-to-component reference for current board-state fields
