# Client UI Redesign Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Redesign all TeslaCANModder user-facing interfaces (Expo client + ESP32 dashboard + gamepad UX) using shadcn components, split monolithic files, and improve overall UX.

**Architecture:** Add `nativewind` + `react-native-reusables` (shadcn for RN/web). Replace custom `ui/` components with shadcn equivalents. Split `AppExperience.tsx` (1130 lines) into focused tab connectors. Split `BoardConnectionContext.tsx` (334 lines) into 3 focused contexts. Lazy-load all 5 tabs with error boundaries. Redesign ESP32 dashboard HTML (1257 lines) with modern CSS grid + dark mode. Full responsive: mobile-first with phone/tablet/desktop breakpoints.

**Tech Stack:** Expo SDK 54, React 19, TypeScript, Tailwind CSS, nativewind, react-native-reusables (shadcn), react-native-safe-area-context, react-native-svg, Jest + @testing-library/react-native

**Testing:** Every new component/screen/context gets a test file. Every file listed in "Create" has a matching test in `tests/`. Use `@testing-library/react-native` with `render()` + `fireEvent`. shadcn components tested for: render with variants, press handlers, disabled state, accessibility labels. Screens tested for: renders without crash, shows correct data for board state, toggles fire commands.

**Responsive breakpoints** (Tailwind defaults, mirrored in `useBreakpoint`):
| Prefix | Width | Layout |
|--------|-----------|---------------------------------------|
| _default_ | < 640px | Single column, bottom tabs, compact |
| `sm:` | 640-768px | Single column, wider cards |
| `md:` | 768-1024px | 2-column grid, side nav in console |
| `lg:` | 1024px+ | 3-column grid, expanded sidebars |

---

## File Structure Map

### Files to CREATE

| File                                                      | Purpose                                                   |
| --------------------------------------------------------- | --------------------------------------------------------- |
| `client/nativewind-env.d.ts`                              | NativeWind type declarations                              |
| `client/src/ui/shadcn/button.tsx`                         | shadcn Button (replaces ui/Button.tsx)                    |
| `client/src/ui/shadcn/card.tsx`                           | shadcn Card (replaces ui/Card.tsx)                        |
| `client/src/ui/shadcn/input.tsx`                          | shadcn Input (replaces ui/Input.tsx)                      |
| `client/src/ui/shadcn/label.tsx`                          | shadcn Label (replaces ui/Label.tsx)                      |
| `client/src/ui/shadcn/select.tsx`                         | shadcn Select (replaces ui/Select.tsx)                    |
| `client/src/ui/shadcn/badge.tsx`                          | shadcn Badge (replaces ui/Badge.tsx)                      |
| `client/src/ui/shadcn/sheet.tsx`                          | shadcn Sheet (replaces ui/Sheet.tsx)                      |
| `client/src/ui/shadcn/alert.tsx`                          | shadcn Alert (replaces ui/Alert.tsx)                      |
| `client/src/ui/shadcn/tabs.tsx`                           | shadcn Tabs (replaces ui/Tabs.tsx)                        |
| `client/src/ui/shadcn/separator.tsx`                      | shadcn Separator (replaces ui/Divider.tsx)                |
| `client/src/ui/shadcn/skeleton.tsx`                       | shadcn Skeleton for loading states                        |
| `client/src/ui/shadcn/index.ts`                           | Barrel export for all shadcn components                   |
| `client/src/ui/cn.ts`                                     | clsx + tailwind-merge utility                             |
| `client/src/components/dashboard/DashboardTab.tsx`        | Tab connector: DashboardScreen + hooks                    |
| `client/src/components/dashboard/ControlsTab.tsx`         | Tab connector: ControlsScreen + hooks                     |
| `client/src/components/dashboard/ConsoleTab.tsx`          | Tab connector: ConsoleScreen + hooks                      |
| `client/src/components/dashboard/FlasherTab.tsx`          | Tab connector: FlasherScreen + hooks                      |
| `client/src/components/dashboard/DocsTab.tsx`             | Tab connector: DocsScreen + hooks                         |
| `client/src/components/TabErrorBoundary.tsx`              | Per-tab error boundary with retry                         |
| `client/src/components/transport/TransportPicker.tsx`     | Transport type selector (from ConnectionHeader)           |
| `client/src/components/transport/ConnectSheet.tsx`        | Connection config sheet (from ConnectionHeader)           |
| `client/src/components/transport/ConnectionStatusBar.tsx` | Connection status indicator (from ConnectionHeader)       |
| `client/src/state/TransportContext.tsx`                   | Transport selection + connection state context            |
| `client/src/state/BoardStateContext.tsx`                  | Board state + message handling context                    |
| `client/src/state/CommandContext.tsx`                     | Command lifecycle context                                 |
| `client/src/hooks/useDashboardPolling.ts`                 | Dashboard-specific auto-polling hook                      |
| `client/src/hooks/useControlsState.ts`                    | Controls tab state management                             |
| —                                                         | **TEST FILES**                                            |
| `tests/ui/button.test.tsx`                                | shadcn Button: variants, disabled, press                  |
| `tests/ui/card.test.tsx`                                  | shadcn Card: header/content/footer slots                  |
| `tests/ui/badge.test.tsx`                                 | shadcn Badge: variants, label                             |
| `tests/ui/input.test.tsx`                                 | shadcn Input: value, placeholder, onChange                |
| `tests/ui/sheet.test.tsx`                                 | shadcn Sheet: open/close, title, children                 |
| `tests/ui/tabs.test.tsx`                                  | shadcn Tabs: switch tab, content visibility               |
| `tests/ui/select.test.tsx`                                | shadcn Select: open, select option, onChange              |
| `tests/ui/skeleton.test.tsx`                              | shadcn Skeleton: renders, animate class exists            |
| `tests/ui/alert.test.tsx`                                 | shadcn Alert: default/destructive variants                |
| `tests/components/TabErrorBoundary.test.tsx`              | Error boundary: catches crash, retry button               |
| `tests/components/transport/TransportPicker.test.tsx`     | TransportPicker: render options, select fires cb          |
| `tests/components/transport/ConnectSheet.test.tsx`        | ConnectSheet: open/close, URL input, connect btn          |
| `tests/components/transport/ConnectionStatusBar.test.tsx` | StatusBar: connected/disconnected/connecting states       |
| `tests/components/dashboard/DashboardTab.test.tsx`        | DashboardTab: renders screen, passes props                |
| `tests/components/dashboard/ControlsTab.test.tsx`         | ControlsTab: renders screen, passes props                 |
| `tests/components/dashboard/ConsoleTab.test.tsx`          | ConsoleTab: renders screen, passes props                  |
| `tests/components/dashboard/FlasherTab.test.tsx`          | FlasherTab: lazy loads, renders screen                    |
| `tests/components/dashboard/DocsTab.test.tsx`             | DocsTab: lazy loads, renders screen                       |
| `tests/components/controls/FsdPanel.test.tsx`             | FsdPanel: toggles fsd, force, nag, profile, offset        |
| `tests/components/controls/BmsPanel.test.tsx`             | BmsPanel: renders telemetry, query button works           |
| `tests/components/controls/DasPanel.test.tsx`             | DasPanel: drive on/off, speed limit, cancel               |
| `tests/components/controls/GamepadPanel.test.tsx`         | GamepadPanel: scan flow, pair, binding table              |
| `tests/components/controls/SpeedTuningPanel.test.tsx`     | SpeedTuning: profile/offset selectors                     |
| `tests/state/TransportContext.test.tsx`                   | TransportContext: select type, connect/disconnect         |
| `tests/state/BoardStateContext.test.tsx`                  | BoardStateContext: fetch status, merge payload            |
| `tests/state/CommandContext.test.tsx`                     | CommandContext: run command, lifecycle tracking           |
| `tests/hooks/useDashboardPolling.test.ts`                 | Polling: interval respects config, pauses when hidden     |
| `tests/screens/DashboardScreen.responsive.test.tsx`       | Dashboard: single col mobile, 2-col tablet, 3-col desktop |
| `tests/screens/ControlsScreen.responsive.test.tsx`        | Controls: stack mobile, grid desktop                      |
| `tests/screens/DriveScreen.responsive.test.tsx`           | Drive: portrait vs landscape layouts                      |

### Files to MODIFY

| File                                           | What changes                                                     |
| ---------------------------------------------- | ---------------------------------------------------------------- |
| `client/package.json`                          | Add nativewind, react-native-reusables, tailwindcss-animate      |
| `client/tailwind.config.js`                    | Add shadcn theme tokens, content paths for nativewind            |
| `client/tsconfig.json`                         | Add nativewind paths                                             |
| `client/babel.config.js`                       | Add nativewind/babel plugin                                      |
| `client/index.ts`                              | Wrap with nativewind StyleSheet (if needed)                      |
| `client/src/global.css`                        | Add shadcn CSS variables + dark mode                             |
| `client/src/AppExperience.tsx`                 | ~1130 lines → ~150 lines (delegate to tab connectors)            |
| `client/src/AppView.tsx`                       | Wrap with TransportProvider, BoardStateProvider, CommandProvider |
| `client/src/state/BoardConnectionContext.tsx`  | 334 lines → split into 3 contexts                                |
| `client/src/components/ConnectionHeader.tsx`   | 440 lines → delegate to transport/ sub-components                |
| `client/src/components/MenuHeader.tsx`         | Redesign with shadcn Tabs                                        |
| `client/src/screens/DashboardScreen.tsx`       | Redesign with shadcn Card + useDashboardPolling hook             |
| `client/src/screens/ControlsScreen.tsx`        | 1157 lines → split into panel components                         |
| `client/src/screens/DriveScreen.tsx`           | Redesign gauges/dials with shadcn styling                        |
| `client/src/screens/ConsoleScreen.tsx`         | Clean up, use shadcn components                                  |
| `client/src/screens/FlasherScreen.tsx`         | Polish with shadcn components                                    |
| `client/src/screens/DocsScreen.tsx`            | Polish sidebar nav                                               |
| `client/src/design/tokens.ts`                  | Align with shadcn CSS variable names                             |
| `firmware/lib/client/dashboard/dashboard.html` | 1257 lines → full redesign with modern CSS + dark mode           |

### Files to REMOVE

| File                         | Replaced by                                  |
| ---------------------------- | -------------------------------------------- |
| `client/src/ui/Alert.tsx`    | `ui/shadcn/alert.tsx`                        |
| `client/src/ui/Badge.tsx`    | `ui/shadcn/badge.tsx`                        |
| `client/src/ui/Button.tsx`   | `ui/shadcn/button.tsx`                       |
| `client/src/ui/Card.tsx`     | `ui/shadcn/card.tsx`                         |
| `client/src/ui/Divider.tsx`  | `ui/shadcn/separator.tsx`                    |
| `client/src/ui/Input.tsx`    | `ui/shadcn/input.tsx`                        |
| `client/src/ui/Label.tsx`    | `ui/shadcn/label.tsx`                        |
| `client/src/ui/Select.tsx`   | `ui/shadcn/select.tsx`                       |
| `client/src/ui/Sheet.tsx`    | `ui/shadcn/sheet.tsx`                        |
| `client/src/ui/Tabs.tsx`     | `ui/shadcn/tabs.tsx`                         |
| `client/src/ui/StatChip.tsx` | Inline in consumers (not a shadcn primitive) |

---

## Phase A: Foundation — NativeWind + shadcn setup

### Task A1: Install NativeWind + shadcn dependencies

**Files:**

- Modify: `client/package.json`

- [ ] **Step 1: Add nativewind and shadcn packages**

```bash
cd client && npx expo install nativewind tailwindcss-react-native tailwindcss-animate class-variance-authority clsx tailwind-merge lucide-react-native
```

And for shadcn components:

```bash
cd client && npm install react-native-reusables
```

- [ ] **Step 2: Verify install**

```bash
cd client && npm ls nativewind react-native-reusables
```

Expected: `nativewind@latest`, `react-native-reusables@latest` in dependency tree.

- [ ] **Step 3: Commit**

```bash
git add client/package.json client/package-lock.json
git commit -m "chore: add nativewind and react-native-reusables (shadcn) dependencies"
```

### Task A2: Configure NativeWind + Tailwind

**Files:**

- Modify: `client/tailwind.config.js`
- Modify: `client/babel.config.js`
- Create: `client/nativewind-env.d.ts`
- Modify: `client/src/styles/global.css`

- [ ] **Step 1: Update tailwind.config.js with shadcn theme**

```js
const { hairlineWidth } = require("nativewind/theme");

/** @type {import('tailwindcss').Config} */
module.exports = {
    darkMode: "class",
    content: ["./src/**/*.{ts,tsx}", "./node_modules/react-native-reusables/**/*.{js,ts,tsx}"],
    presets: [require("nativewind/preset")],
    theme: {
        extend: {
            colors: {
                border: "hsl(var(--border))",
                input: "hsl(var(--input))",
                ring: "hsl(var(--ring))",
                background: "hsl(var(--background))",
                foreground: "hsl(var(--foreground))",
                primary: {
                    DEFAULT: "hsl(var(--primary))",
                    foreground: "hsl(var(--primary-foreground))",
                },
                secondary: {
                    DEFAULT: "hsl(var(--secondary))",
                    foreground: "hsl(var(--secondary-foreground))",
                },
                destructive: {
                    DEFAULT: "hsl(var(--destructive))",
                    foreground: "hsl(var(--destructive-foreground))",
                },
                muted: {
                    DEFAULT: "hsl(var(--muted))",
                    foreground: "hsl(var(--muted-foreground))",
                },
                accent: {
                    DEFAULT: "hsl(var(--accent))",
                    foreground: "hsl(var(--accent-foreground))",
                },
                card: {
                    DEFAULT: "hsl(var(--card))",
                    foreground: "hsl(var(--card-foreground))",
                },
                // Tesla red accent
                tesla: {
                    DEFAULT: "#d73c1f",
                    dark: "#b93118",
                },
            },
            borderWidth: { hairline: hairlineWidth() },
            borderRadius: {
                lg: "var(--radius)",
                md: "calc(var(--radius) - 2px)",
                sm: "calc(var(--radius) - 4px)",
            },
        },
    },
    plugins: [require("tailwindcss-animate")],
};
```

- [ ] **Step 2: Update babel.config.js**

```js
module.exports = function (api) {
    api.cache(true);
    return {
        presets: [["babel-preset-expo", { jsxImportSource: "nativewind" }], "nativewind/babel"],
    };
};
```

- [ ] **Step 3: Create nativewind-env.d.ts**

```ts
/// <reference types="nativewind/types" />
```

- [ ] **Step 4: Update global.css with shadcn CSS variables**

```css
@tailwind base;
@tailwind components;
@tailwind utilities;

@layer base {
    :root {
        --background: 0 0% 100%;
        --foreground: 222.2 84% 4.9%;
        --card: 0 0% 100%;
        --card-foreground: 222.2 84% 4.9%;
        --primary: 222.2 47.4% 11.2%;
        --primary-foreground: 210 40% 98%;
        --secondary: 210 40% 96.1%;
        --secondary-foreground: 222.2 47.4% 11.2%;
        --muted: 210 40% 96.1%;
        --muted-foreground: 215.4 16.3% 46.9%;
        --accent: 210 40% 96.1%;
        --accent-foreground: 222.2 47.4% 11.2%;
        --destructive: 0 84.2% 60.2%;
        --destructive-foreground: 210 40% 98%;
        --border: 214.3 31.8% 91.4%;
        --input: 214.3 31.8% 91.4%;
        --ring: 222.2 84% 4.9%;
        --radius: 0.5rem;
    }

    .dark {
        --background: 222.2 84% 4.9%;
        --foreground: 210 40% 98%;
        --card: 222.2 84% 4.9%;
        --card-foreground: 210 40% 98%;
        --primary: 210 40% 98%;
        --primary-foreground: 222.2 47.4% 11.2%;
        --secondary: 217.2 32.6% 17.5%;
        --secondary-foreground: 210 40% 98%;
        --muted: 217.2 32.6% 17.5%;
        --muted-foreground: 215 20.2% 65.1%;
        --accent: 217.2 32.6% 17.5%;
        --accent-foreground: 210 40% 98%;
        --destructive: 0 62.8% 30.6%;
        --destructive-foreground: 210 40% 98%;
        --border: 217.2 32.6% 17.5%;
        --input: 217.2 32.6% 17.5%;
        --ring: 212.7 26.8% 83.9%;
    }
}
```

- [ ] **Step 5: Verify Tailwind + NativeWind builds**

```bash
cd client && npx tailwindcss -i src/styles/global.css --dry-run
```

Expected: No errors.

- [ ] **Step 6: Commit**

```bash
git add client/tailwind.config.js client/babel.config.js client/nativewind-env.d.ts client/src/styles/global.css
git commit -m "feat: configure nativewind with shadcn theme tokens"
```

### Task A3: Create shadcn base components

**Files:**

- Create: `client/src/ui/shadcn/button.tsx`
- Create: `client/src/ui/shadcn/card.tsx`
- Create: `client/src/ui/shadcn/input.tsx`
- Create: `client/src/ui/shadcn/label.tsx`
- Create: `client/src/ui/shadcn/badge.tsx`
- Create: `client/src/ui/shadcn/select.tsx`
- Create: `client/src/ui/shadcn/separator.tsx`
- Create: `client/src/ui/shadcn/sheet.tsx`
- Create: `client/src/ui/shadcn/tabs.tsx`
- Create: `client/src/ui/shadcn/alert.tsx`
- Create: `client/src/ui/shadcn/skeleton.tsx`
- Create: `client/src/ui/shadcn/index.ts`

- [ ] **Step 1: Create shadcn Button**

```tsx
// client/src/ui/shadcn/button.tsx
import { cva, type VariantProps } from "class-variance-authority";
import { Text, Pressable, type PressableProps } from "react-native";
import { cn } from "../cn";

const buttonVariants = cva("flex-row items-center justify-center rounded-md px-4 py-2", {
    variants: {
        variant: {
            default: "bg-primary",
            destructive: "bg-destructive",
            outline: "border border-input bg-transparent",
            secondary: "bg-secondary",
            ghost: "bg-transparent",
            link: "bg-transparent",
        },
        size: {
            default: "h-10",
            sm: "h-8 px-3",
            lg: "h-12 px-8",
            icon: "h-10 w-10",
        },
    },
    defaultVariants: { variant: "default", size: "default" },
});

const buttonTextVariants = cva("text-sm font-medium", {
    variants: {
        variant: {
            default: "text-primary-foreground",
            destructive: "text-destructive-foreground",
            outline: "text-foreground",
            secondary: "text-secondary-foreground",
            ghost: "text-foreground",
            link: "text-primary underline",
        },
    },
    defaultVariants: { variant: "default" },
});

interface ButtonProps extends PressableProps, VariantProps<typeof buttonVariants> {
    label: string;
}

export function Button({ variant, size, label, className, disabled, ...props }: ButtonProps) {
    return (
        <Pressable
            className={cn(buttonVariants({ variant, size }), disabled && "opacity-50", className)}
            disabled={disabled}
            {...props}
        >
            <Text className={cn(buttonTextVariants({ variant }))}>{label}</Text>
        </Pressable>
    );
}
```

- [ ] **Step 2: Create shadcn Card**

```tsx
// client/src/ui/shadcn/card.tsx
import { View, Text, type ViewProps, type TextProps } from "react-native";
import { cn } from "../cn";

export function Card({ className, ...props }: ViewProps) {
    return (
        <View className={cn("rounded-lg border border-border bg-card p-4", className)} {...props} />
    );
}

export function CardHeader({ className, ...props }: ViewProps) {
    return <View className={cn("gap-1.5 pb-3", className)} {...props} />;
}

export function CardTitle({ className, ...props }: TextProps) {
    return (
        <Text className={cn("text-lg font-semibold text-card-foreground", className)} {...props} />
    );
}

export function CardDescription({ className, ...props }: TextProps) {
    return <Text className={cn("text-sm text-muted-foreground", className)} {...props} />;
}

export function CardContent({ className, ...props }: ViewProps) {
    return <View className={cn("", className)} {...props} />;
}

export function CardFooter({ className, ...props }: ViewProps) {
    return <View className={cn("flex-row items-center pt-3", className)} {...props} />;
}
```

- [ ] **Step 3: Create shadcn Badge**

```tsx
// client/src/ui/shadcn/badge.tsx
import { cva, type VariantProps } from "class-variance-authority";
import { Text, View, type ViewProps } from "react-native";
import { cn } from "../cn";

const badgeVariants = cva("inline-flex items-center rounded-full px-2.5 py-0.5", {
    variants: {
        variant: {
            default: "bg-primary",
            secondary: "bg-secondary",
            destructive: "bg-destructive",
            outline: "border border-border bg-transparent",
        },
    },
    defaultVariants: { variant: "default" },
});

const badgeTextVariants = cva("text-xs font-semibold", {
    variants: {
        variant: {
            default: "text-primary-foreground",
            secondary: "text-secondary-foreground",
            destructive: "text-destructive-foreground",
            outline: "text-foreground",
        },
    },
    defaultVariants: { variant: "default" },
});

interface BadgeProps extends ViewProps, VariantProps<typeof badgeVariants> {
    label: string;
}

export function Badge({ variant, label, className, ...props }: BadgeProps) {
    return (
        <View className={cn(badgeVariants({ variant }), className)} {...props}>
            <Text className={cn(badgeTextVariants({ variant }))}>{label}</Text>
        </View>
    );
}
```

- [ ] **Step 4: Create cn utility**

```ts
// client/src/ui/cn.ts
import { clsx, type ClassValue } from "clsx";
import { twMerge } from "tailwind-merge";
export function cn(...inputs: ClassValue[]) {
    return twMerge(clsx(inputs));
}
```

- [ ] **Step 5: Create shadcn Input**

```tsx
// client/src/ui/shadcn/input.tsx
import { TextInput, type TextInputProps } from "react-native";
import { cn } from "../cn";

export function Input({ className, ...props }: TextInputProps) {
    return (
        <TextInput
            className={cn(
                "h-10 rounded-md border border-input bg-background px-3 py-2 text-sm text-foreground placeholder:text-muted-foreground",
                className,
            )}
            {...props}
        />
    );
}
```

- [ ] **Step 6: Create shadcn Label**

```tsx
// client/src/ui/shadcn/label.tsx
import { Text, type TextProps } from "react-native";
import { cn } from "../cn";

export function Label({ className, ...props }: TextProps) {
    return <Text className={cn("text-sm font-medium text-foreground", className)} {...props} />;
}
```

- [ ] **Step 7: Create shadcn Separator**

```tsx
// client/src/ui/shadcn/separator.tsx
import { View, type ViewProps } from "react-native";
import { cn } from "../cn";

export function Separator({ className, ...props }: ViewProps) {
    return <View className={cn("h-px bg-border", className)} {...props} />;
}
```

- [ ] **Step 8: Create shadcn Skeleton**

```tsx
// client/src/ui/shadcn/skeleton.tsx
import { View, type ViewProps } from "react-native";
import { cn } from "../cn";

export function Skeleton({ className, ...props }: ViewProps) {
    return <View className={cn("h-4 rounded bg-muted animate-pulse", className)} {...props} />;
}
```

- [ ] **Step 9: Create shadcn Sheet**

```tsx
// client/src/ui/shadcn/sheet.tsx
import { Pressable, View, Text, Modal, type ModalProps } from "react-native";
import { cn } from "../cn";
import { Button } from "./button";

interface SheetProps extends ModalProps {
    open: boolean;
    onClose: () => void;
    title?: string;
}

export function Sheet({ open, onClose, title, children, ...props }: SheetProps) {
    return (
        <Modal visible={open} transparent animationType="slide" onRequestClose={onClose} {...props}>
            <Pressable className="flex-1 bg-black/50" onPress={onClose} />
            <View className="rounded-t-xl border-t border-border bg-card p-4 gap-4 max-h-[80%]">
                <View className="flex-row items-center justify-between">
                    {title ? (
                        <Text className="text-lg font-semibold text-card-foreground">{title}</Text>
                    ) : (
                        <View />
                    )}
                    <Button label="Done" variant="ghost" size="sm" onPress={onClose} />
                </View>
                {children}
            </View>
        </Modal>
    );
}
```

- [ ] **Step 10: Create shadcn Tabs**

```tsx
// client/src/ui/shadcn/tabs.tsx
import { createContext, useContext, useState, type ReactNode } from "react";
import { View, Text, Pressable } from "react-native";
import { cn } from "../cn";

const TabsCtx = createContext<{ value: string; onChange: (v: string) => void } | null>(null);

export function Tabs({
    value,
    onChange,
    children,
}: {
    value: string;
    onChange: (v: string) => void;
    children: ReactNode;
}) {
    return <TabsCtx.Provider value={{ value, onChange }}>{children}</TabsCtx.Provider>;
}

export function TabsList({ className, children }: { className?: string; children: ReactNode }) {
    return <View className={cn("flex-row border-b border-border", className)}>{children}</View>;
}

export function TabsTrigger({
    value,
    label,
    className,
}: {
    value: string;
    label: string;
    className?: string;
}) {
    const ctx = useContext(TabsCtx);
    if (!ctx) return null;
    const active = ctx.value === value;
    return (
        <Pressable
            onPress={() => ctx.onChange(value)}
            className={cn(
                "px-4 py-2 border-b-2",
                active ? "border-primary" : "border-transparent",
                className,
            )}
        >
            <Text
                className={cn(
                    "text-sm",
                    active ? "text-primary font-medium" : "text-muted-foreground",
                )}
            >
                {label}
            </Text>
        </Pressable>
    );
}

export function TabsContent({ value, children }: { value: string; children: ReactNode }) {
    const ctx = useContext(TabsCtx);
    if (!ctx || ctx.value !== value) return null;
    return <>{children}</>;
}
```

- [ ] **Step 11: Create shadcn Alert**

```tsx
// client/src/ui/shadcn/alert.tsx
import { View, Text, type ViewProps } from "react-native";
import { cn } from "../cn";
import { TriangleAlert } from "lucide-react-native";

interface AlertProps extends ViewProps {
    variant?: "default" | "destructive";
    title?: string;
    description?: string;
}

export function Alert({
    variant = "default",
    title,
    description,
    className,
    ...props
}: AlertProps) {
    return (
        <View
            className={cn(
                "rounded-lg border p-4",
                variant === "destructive"
                    ? "border-destructive/50 bg-destructive/10"
                    : "border-border bg-muted/50",
                className,
            )}
            {...props}
        >
            <View className="flex-row items-center gap-2">
                {variant === "destructive" && (
                    <TriangleAlert size={16} className="text-destructive" />
                )}
                {title ? (
                    <Text
                        className={cn(
                            "text-sm font-semibold",
                            variant === "destructive" ? "text-destructive" : "text-foreground",
                        )}
                    >
                        {title}
                    </Text>
                ) : null}
            </View>
            {description ? (
                <Text
                    className={cn(
                        "text-sm mt-1",
                        variant === "destructive" ? "text-destructive/80" : "text-muted-foreground",
                    )}
                >
                    {description}
                </Text>
            ) : null}
        </View>
    );
}
```

- [ ] **Step 12: Create shadcn Select (dropdown)**

```tsx
// client/src/ui/shadcn/select.tsx
import { useState } from "react";
import { View, Text, Pressable, Modal, ScrollView } from "react-native";
import { cn } from "../cn";
import { ChevronDown } from "lucide-react-native";

interface SelectOption {
    value: string;
    label: string;
}

interface SelectProps {
    value: string;
    options: SelectOption[];
    onChange: (value: string) => void;
    placeholder?: string;
    className?: string;
}

export function Select({ value, options, onChange, placeholder, className }: SelectProps) {
    const [open, setOpen] = useState(false);
    const selected = options.find((o) => o.value === value);

    return (
        <View className={className}>
            <Pressable
                onPress={() => setOpen(true)}
                className="flex-row items-center justify-between h-10 rounded-md border border-input bg-background px-3"
            >
                <Text
                    className={cn(
                        "text-sm",
                        selected ? "text-foreground" : "text-muted-foreground",
                    )}
                >
                    {selected?.label ?? placeholder ?? "Select..."}
                </Text>
                <ChevronDown size={16} className="text-muted-foreground" />
            </Pressable>
            <Modal
                visible={open}
                transparent
                animationType="fade"
                onRequestClose={() => setOpen(false)}
            >
                <Pressable className="flex-1 bg-black/50" onPress={() => setOpen(false)} />
                <View className="bg-card rounded-t-xl max-h-[60%]">
                    <ScrollView>
                        {options.map((opt) => (
                            <Pressable
                                key={opt.value}
                                onPress={() => {
                                    onChange(opt.value);
                                    setOpen(false);
                                }}
                                className={cn(
                                    "px-4 py-3 border-b border-border/50",
                                    opt.value === value ? "bg-primary/10" : "",
                                )}
                            >
                                <Text
                                    className={cn(
                                        "text-sm",
                                        opt.value === value
                                            ? "text-primary font-medium"
                                            : "text-foreground",
                                    )}
                                >
                                    {opt.label}
                                </Text>
                            </Pressable>
                        ))}
                    </ScrollView>
                </View>
            </Modal>
        </View>
    );
}
```

- [ ] **Step 13: Create barrel export**

```ts
// client/src/ui/shadcn/index.ts
export { Button } from "./button";
export { Card, CardHeader, CardTitle, CardDescription, CardContent, CardFooter } from "./card";
export { Badge } from "./badge";
export { Input } from "./input";
export { Label } from "./label";
export { Select } from "./select";
export { Separator } from "./separator";
export { Sheet } from "./sheet";
export { Tabs, TabsList, TabsTrigger, TabsContent } from "./tabs";
export { Alert } from "./alert";
export { Skeleton } from "./skeleton";
```

- [ ] **Step 7: Commit**

```bash
git add client/src/ui/shadcn/ client/src/ui/cn.ts
git commit -m "feat: add shadcn component primitives (Button, Card, Badge, Input, etc.)"
```

### Task A4: Write tests for all shadcn components

**Files:**

- Create: `tests/ui/button.test.tsx`, `card.test.tsx`, `badge.test.tsx`, `input.test.tsx`
- Create: `tests/ui/sheet.test.tsx`, `tabs.test.tsx`, `select.test.tsx`, `skeleton.test.tsx`, `alert.test.tsx`

- [ ] **Step 1: Write Button tests**

```tsx
// tests/ui/button.test.tsx
import { render, fireEvent } from "@testing-library/react-native";
import { Button } from "../../src/ui/shadcn/button";

describe("Button", () => {
    it("renders label", () => {
        const { getByText } = render(<Button label="Click me" />);
        expect(getByText("Click me")).toBeTruthy();
    });

    it("fires onPress", () => {
        const onPress = jest.fn();
        const { getByText } = render(<Button label="Press" onPress={onPress} />);
        fireEvent.press(getByText("Press"));
        expect(onPress).toHaveBeenCalledTimes(1);
    });

    it("does not fire onPress when disabled", () => {
        const onPress = jest.fn();
        const { getByText } = render(<Button label="Press" disabled onPress={onPress} />);
        fireEvent.press(getByText("Press"));
        expect(onPress).not.toHaveBeenCalled();
    });

    it("renders each variant without crash", () => {
        for (const v of [
            "default",
            "destructive",
            "outline",
            "secondary",
            "ghost",
            "link",
        ] as const) {
            const { toJSON } = render(<Button label="Test" variant={v} />);
            expect(toJSON()).toBeTruthy();
        }
    });

    it("applies size classes", () => {
        const { getByText } = render(<Button label="Small" size="sm" />);
        expect(getByText("Small")).toBeTruthy();
    });
});
```

- [ ] **Step 2: Write Badge tests**

```tsx
// tests/ui/badge.test.tsx
import { render } from "@testing-library/react-native";
import { Badge } from "../../src/ui/shadcn/badge";

describe("Badge", () => {
    it("renders label", () => {
        /* ... */
    });
    it("renders each variant", () => {
        /* variants: default, secondary, destructive, outline */
    });
    it("applies custom className", () => {
        /* ... */
    });
});
```

- [ ] **Step 3: Write Card tests**

```tsx
// tests/ui/card.test.tsx
import { render } from "@testing-library/react-native";
import {
    Card,
    CardHeader,
    CardTitle,
    CardDescription,
    CardContent,
    CardFooter,
} from "../../src/ui/shadcn/card";

describe("Card", () => {
    it("renders all slots: header + title + description + content + footer", () => {
        /* ... */
    });
    it("renders standalone Card without slots", () => {
        /* ... */
    });
});
```

- [ ] **Step 4: Write Input tests**

```tsx
// tests/ui/input.test.tsx
describe("Input", () => {
    it("renders with placeholder", () => {
        /* ... */
    });
    it("fires onChangeText on type", () => {
        /* ... */
    });
    it("applies value prop", () => {
        /* ... */
    });
});
```

- [ ] **Step 5: Write Sheet tests**

```tsx
// tests/ui/sheet.test.tsx
describe("Sheet", () => {
    it("shows children when open=true", () => {
        /* ... */
    });
    it("hides children when open=false", () => {
        /* ... */
    });
    it("fires onClose when Done pressed", () => {
        /* ... */
    });
    it("renders title when provided", () => {
        /* ... */
    });
});
```

- [ ] **Step 6: Write Tabs tests**

```tsx
// tests/ui/tabs.test.tsx
describe("Tabs", () => {
    it("shows content for active tab", () => {
        /* ... */
    });
    it("hides content for inactive tab", () => {
        /* ... */
    });
    it("fires onChange on trigger press", () => {
        /* ... */
    });
    it("renders multiple triggers in list", () => {
        /* ... */
    });
});
```

- [ ] **Step 7: Write remaining component tests**

`select.test.tsx`: open modal, select option, onChange fires.  
`skeleton.test.tsx`: renders, has animate-pulse class.  
`alert.test.tsx`: default variant shows text; destructive variant shows icon + red text.

- [ ] **Step 8: Run all UI tests**

```bash
cd client && npx jest tests/ui/ 2>&1
```

Expected: All 9 test suites pass.

- [ ] **Step 9: Commit**

```bash
git add tests/ui/
git commit -m "test: add test suites for all shadcn UI primitives"
```

---

## Phase B: Architecture — Split the monoliths

### Task B1: Split BoardConnectionContext into focused contexts

**Files:**

- Create: `client/src/state/TransportContext.tsx`
- Create: `client/src/state/BoardStateContext.tsx`
- Create: `client/src/state/CommandContext.tsx`
- Modify: `client/src/state/BoardConnectionContext.tsx`
- Modify: `client/src/AppView.tsx`

- [ ] **Step 1: Create TransportContext**

Extract transport selection, connection lifecycle, baseUrl, connect/disconnect actions from BoardConnectionContext.

```tsx
// client/src/state/TransportContext.tsx
import { createContext, useContext, useCallback, useState, type ReactNode } from "react";
import type { MonitorTransportType } from "../hardware/transportPresentation";

interface TransportState {
    selectedTransportType: MonitorTransportType;
    baseUrl: string;
    isSelectedTransportReady: boolean;
    connectionBusy: boolean;
    connected: boolean;
}

interface TransportActions {
    setTransportType: (type: MonitorTransportType) => void;
    setBaseUrl: (url: string) => void;
    setReady: (ready: boolean) => void;
    connect: () => Promise<void>;
    disconnect: () => void;
}

const TransportStateCtx = createContext<TransportState | null>(null);
const TransportActionsCtx = createContext<TransportActions | null>(null);

// Provider, hooks: useTransportState(), useTransportActions()
```

- [ ] **Step 2: Create BoardStateContext**

Extract board state, status fetching, frame ingestion, polling.

```tsx
// client/src/state/BoardStateContext.tsx
import { createContext, useContext, type ReactNode } from "react";
import type { BoardState } from "@teslacanmodder/protocol";

interface BoardStateValue {
    boardState: BoardState;
    statusPollInterval: number;
    setPollInterval: (ms: number) => void;
    fetchStatus: () => Promise<void>;
    boardMessages: BoardMessage[];
}

const BoardStateCtx = createContext<BoardStateValue | null>(null);
export function useBoardState() {
    /* ... */
}
```

- [ ] **Step 3: Create CommandContext**

Extract command lifecycle, history, execution.

```tsx
// client/src/state/CommandContext.tsx
interface CommandValue {
    history: HistoryEntry[];
    commandBus: CommandBusState;
    runCommand: (cmd: string) => Promise<CommandResult>;
    isGated: (cmd: string) => boolean;
}
```

- [ ] **Step 4: Update AppView.tsx to wrap with new providers**

```tsx
// AppView.tsx
<TransportProvider>
    <BoardStateProvider>
        <CommandProvider>
            <AppExperience />
        </CommandProvider>
    </BoardStateProvider>
</TransportProvider>
```

- [ ] **Step 5: Verify typecheck**

```bash
cd client && npx tsc --noEmit 2>&1
```

- [ ] **Step 6: Commit**

```bash
git add client/src/state/
git commit -m "refactor: split BoardConnectionContext into Transport, BoardState, Command contexts"
```

### Task B2: Extract tab connectors from AppExperience

**Files:**

- Create: `client/src/components/dashboard/DashboardTab.tsx`
- Create: `client/src/components/dashboard/ControlsTab.tsx`
- Create: `client/src/components/dashboard/ConsoleTab.tsx`
- Create: `client/src/components/dashboard/FlasherTab.tsx`
- Create: `client/src/components/dashboard/DocsTab.tsx`
- Modify: `client/src/AppExperience.tsx`

- [ ] **Step 1: Create DashboardTab connector**

Extract all DashboardScreen props and polling logic from AppExperience into a self-contained component:

```tsx
// client/src/components/dashboard/DashboardTab.tsx
import { useMemo } from "react";
import { DashboardScreen } from "../../screens/DashboardScreen";
import { useBoardState } from "../../state/BoardStateContext";
import { useCommandContext } from "../../state/CommandContext";

export function DashboardTab() {
    const { boardState } = useBoardState();
    const { runCommand } = useCommandContext();

    const dashboardProps = useMemo(
        () => ({
            boardState,
            onCommand: runCommand,
        }),
        [boardState, runCommand],
    );

    return <DashboardScreen {...dashboardProps} />;
}
```

- [ ] **Step 2: Create ControlsTab, ConsoleTab, FlasherTab, DocsTab**

Same pattern — each tab connector is a thin wrapper (30-50 lines) that:

1. Reads only the contexts it needs
2. Derives props for the screen component
3. Returns the screen

- [ ] **Step 3: Simplify AppExperience to tab routing only**

```tsx
// client/src/AppExperience.tsx (~80 lines after refactor)
import { lazy, Suspense } from "react";
import { SafeAreaView } from "react-native-safe-area-context";
import { useAppRouteState } from "./state/appRoute";
import { ConnectionHeader } from "./components/transport/ConnectionStatusBar";
import { MenuHeader } from "./components/MenuHeader";
import { TabErrorBoundary } from "./components/TabErrorBoundary";
import { DashboardTab } from "./components/dashboard/DashboardTab";
import { ControlsTab } from "./components/dashboard/ControlsTab";
import { ConsoleTab } from "./components/dashboard/ConsoleTab";

const FlasherTab = lazy(() =>
    import("./components/dashboard/FlasherTab").then((m) => ({ default: m.FlasherTab })),
);
const DocsTab = lazy(() =>
    import("./components/dashboard/DocsTab").then((m) => ({ default: m.DocsTab })),
);

export function AppExperience() {
    const { activeTab } = useAppRouteState();

    return (
        <SafeAreaView className="flex-1 bg-background">
            <ConnectionHeader />
            <TabErrorBoundary tab={activeTab}>
                <Suspense fallback={<TabSkeleton />}>
                    {activeTab === "dashboard" ? (
                        <DashboardTab />
                    ) : activeTab === "controls" ? (
                        <ControlsTab />
                    ) : activeTab === "console" ? (
                        <ConsoleTab />
                    ) : activeTab === "flasher" ? (
                        <FlasherTab />
                    ) : activeTab === "docs" ? (
                        <DocsTab />
                    ) : null}
                </Suspense>
            </TabErrorBoundary>
            <MenuHeader />
        </SafeAreaView>
    );
}
```

- [ ] **Step 4: Verify typecheck**

```bash
cd client && npx tsc --noEmit 2>&1
```

- [ ] **Step 5: Commit**

```bash
git add client/src/components/dashboard/ client/src/AppExperience.tsx
git commit -m "refactor: extract tab connectors from AppExperience (1130→80 lines)"
```

### Task B4: Write tests for contexts + error boundary + tab connectors

**Files:**

- Create: `tests/state/TransportContext.test.tsx`
- Create: `tests/state/BoardStateContext.test.tsx`
- Create: `tests/state/CommandContext.test.tsx`
- Create: `tests/components/TabErrorBoundary.test.tsx`
- Create: `tests/components/dashboard/DashboardTab.test.tsx`
- Create: `tests/components/dashboard/ControlsTab.test.tsx`

- [ ] **Step 1: Write TransportContext tests**

```tsx
// tests/state/TransportContext.test.tsx
import { renderHook, act } from "@testing-library/react-native";
import {
    TransportProvider,
    useTransportState,
    useTransportActions,
} from "../../src/state/TransportContext";

describe("TransportContext", () => {
    it("starts with HTTP transport by default", () => {
        const { result } = renderHook(() => useTransportState(), { wrapper: TransportProvider });
        expect(result.current.selectedTransportType).toBe("http");
    });

    it("setTransportType switches to BLE", () => {
        const { result } = renderHook(
            () => ({
                state: useTransportState(),
                actions: useTransportActions(),
            }),
            { wrapper: TransportProvider },
        );
        act(() => result.current.actions.setTransportType("ble"));
        expect(result.current.state.selectedTransportType).toBe("ble");
    });

    it("disconnect sets connected=false", () => {
        const { result } = renderHook(
            () => ({
                actions: useTransportActions(),
            }),
            { wrapper: TransportProvider },
        );
        act(() => result.current.actions.disconnect());
        const { result: state } = renderHook(() => useTransportState(), {
            wrapper: TransportProvider,
        });
        expect(state.current.connected).toBe(false);
    });
});
```

- [ ] **Step 2: Write BoardStateContext tests**

```tsx
// tests/state/BoardStateContext.test.tsx
describe("BoardStateContext", () => {
    it("starts with initial board state (all zeros)", () => {
        /* ... */
    });
    it("fetchStatus updates board state from transport", () => {
        /* ... */
    });
    it("setPollInterval changes polling rate", () => {
        /* ... */
    });
});
```

- [ ] **Step 3: Write CommandContext tests**

```tsx
// tests/state/CommandContext.test.tsx
describe("CommandContext", () => {
    it("runCommand adds to history on success", () => {
        /* ... */
    });
    it("runCommand adds to history on failure", () => {
        /* ... */
    });
    it("commandBus tracks pending/acked/failed lifecycle", () => {
        /* ... */
    });
    it("isGated returns false for ungated commands (ping)", () => {
        /* ... */
    });
    it("isGated returns true for gated commands when board lacks features", () => {
        /* ... */
    });
});
```

- [ ] **Step 4: Write TabErrorBoundary tests**

```tsx
// tests/components/TabErrorBoundary.test.tsx
import { render, fireEvent } from "@testing-library/react-native";
import { Text } from "react-native";
import { TabErrorBoundary } from "../../src/components/TabErrorBoundary";

function CrashingChild() {
    throw new Error("Kaboom!");
    return null;
}

describe("TabErrorBoundary", () => {
    it("renders children when no error", () => {
        const { getByText } = render(
            <TabErrorBoundary tab="Dashboard">
                <Text>Hello</Text>
            </TabErrorBoundary>,
        );
        expect(getByText("Hello")).toBeTruthy();
    });

    it("shows error UI and retry button on crash", () => {
        const { getByText } = render(
            <TabErrorBoundary tab="Dashboard">
                <CrashingChild />
            </TabErrorBoundary>,
        );
        expect(getByText("Dashboard tab crashed")).toBeTruthy();
        expect(getByText("Retry")).toBeTruthy();
    });

    it("retry button clears error state", () => {
        /* ... */
    });
});
```

- [ ] **Step 5: Write tab connector smoke tests**

Each tab connector test: renders without crash, passes minimal mock props.

```tsx
// tests/components/dashboard/DashboardTab.test.tsx
describe("DashboardTab", () => {
    it("renders without crash", () => {
        /* wrap in providers, render, check toJSON */
    });
});
// Same pattern for ControlsTab, ConsoleTab, FlasherTab, DocsTab
```

- [ ] **Step 6: Run context + component tests**

```bash
cd client && npx jest tests/state/ tests/components/TabErrorBoundary tests/components/dashboard/ 2>&1
```

Expected: All pass.

- [ ] **Step 7: Commit**

```bash
git add tests/state/ tests/components/TabErrorBoundary.test.tsx tests/components/dashboard/
git commit -m "test: add tests for Transport/BoardState/Command contexts, error boundary, tab connectors"
```

### Task B3: Add TabErrorBoundary

**Files:**

- Create: `client/src/components/TabErrorBoundary.tsx`

- [ ] **Step 1: Create error boundary**

```tsx
// client/src/components/TabErrorBoundary.tsx
import { Component, type ReactNode } from "react";
import { View, Text } from "react-native";
import { Button } from "../ui/shadcn/button";

interface Props {
    tab: string;
    children: ReactNode;
}
interface State {
    error: Error | null;
}

export class TabErrorBoundary extends Component<Props, State> {
    state: State = { error: null };

    static getDerivedStateFromError(error: Error) {
        return { error };
    }

    handleRetry = () => {
        this.setState({ error: null });
    };

    render() {
        if (this.state.error) {
            return (
                <View className="flex-1 items-center justify-center gap-4 p-8">
                    <Text className="text-lg font-semibold text-destructive">
                        {this.props.tab} tab crashed
                    </Text>
                    <Text className="text-sm text-muted-foreground text-center">
                        {this.state.error.message}
                    </Text>
                    <Button label="Retry" variant="outline" onPress={this.handleRetry} />
                </View>
            );
        }
        return this.props.children;
    }
}
```

- [ ] **Step 2: Commit**

```bash
git add client/src/components/TabErrorBoundary.tsx
git commit -m "feat: add per-tab error boundary"
```

---

## Phase C: Redesign Screens

### Task C1: Redesign DashboardScreen

**Files:**

- Modify: `client/src/screens/DashboardScreen.tsx`
- Modify: `client/src/components/TelemetryPanel.tsx`
- Modify: `client/src/components/IntegrationPanel.tsx`
- Modify: `client/src/components/UtilityPanel.tsx`

- [ ] **Step 1: Replace custom components with shadcn**

Replace all `ui/Card` with shadcn `Card/CardHeader/CardTitle/CardContent`, `ui/Badge` with shadcn `Badge`, `ui/Divider` with `Separator`.

- [ ] **Step 2: Use Tailwind classes instead of StyleSheet.create**

Replace all `StyleSheet.create({...})` objects with NativeWind className strings. Example:

```tsx
// Before
<View style={styles.container}>
// After
<View className="gap-4 p-4">
```

- [ ] **Step 3: Responsive grid layout**

Dashboard cards must adapt to screen width. Use `flex-wrap` + min-width constraints (no JS breakpoint switches):

```tsx
// Mobile (default): single column, full-width cards
// Tablet (md:): 2-column grid
// Desktop (lg:): 3-column grid
<View className="flex-row flex-wrap gap-3 px-3 pt-3">
    <View className="w-full md:w-[calc(50%-6px)] lg:w-[calc(33.333%-8px)]">
        <TelemetryPanel />
    </View>
    <View className="w-full md:w-[calc(50%-6px)] lg:w-[calc(33.333%-8px)]">
        <IntegrationPanel />
    </View>
    <View className="w-full md:w-[calc(50%-6px)] lg:w-[calc(33.333%-8px)]">
        <UtilityPanel />
    </View>
</View>
```

Modal drawer between "Overview" and "Drive" sections:

- **Mobile:** full-height bottom sheet (shadcn `Sheet`)
- **Desktop:** side-by-side tabs (shadcn `Tabs`), no modal

```tsx
import { useBreakpoint } from "../../state/useBreakpoint";

const bp = useBreakpoint();
const isDesktop = bp === "desktop" || bp === "cluster";

{
    isDesktop ? (
        <Tabs value={section} onChange={setSection}>
            <TabsList>
                <TabsTrigger value="overview" label="Overview" />
                <TabsTrigger value="drive" label="Drive" />
            </TabsList>
            <TabsContent value="overview">
                <OverviewGrid />
            </TabsContent>
            <TabsContent value="drive">
                <DriveScreen />
            </TabsContent>
        </Tabs>
    ) : (
        <>
            <OverviewGrid />
            <Sheet open={driveOpen} onClose={() => setDriveOpen(false)} title="Drive">
                <DriveScreen />
            </Sheet>
        </>
    );
}
```

- [ ] **Step 4: Run tests**

```bash
cd client && npx jest --testPathPattern="Dashboard" 2>&1
```

- [ ] **Step 5: Commit**

```bash
git add client/src/screens/DashboardScreen.tsx client/src/components/
git commit -m "refactor: redesign DashboardScreen with shadcn components"
```

### Task C2: Split and redesign ControlsScreen

**Files:**

- Create: `client/src/components/controls/FsdPanel.tsx`
- Create: `client/src/components/controls/BmsPanel.tsx`
- Create: `client/src/components/controls/DasPanel.tsx`
- Create: `client/src/components/controls/GamepadPanel.tsx`
- Create: `client/src/components/controls/SpeedTuningPanel.tsx`
- Create: `client/src/components/controls/VehicleCommandsPanel.tsx`
- Modify: `client/src/screens/ControlsScreen.tsx`

- [ ] **Step 1: Extract FsdPanel**

FSD toggle, FSD force, nag mode selector, profile, offset, ECE R79, ISA chime, ALC, region spoof — grouped in one Card:

```tsx
// client/src/components/controls/FsdPanel.tsx
import { Card, CardHeader, CardTitle, CardContent } from "../../ui/shadcn/card";
import { Button } from "../../ui/shadcn/button";
import { Badge } from "../../ui/shadcn/badge";

export function FsdPanel({ boardState, onCommand }: PanelProps) {
    return (
        <Card>
            <CardHeader>
                <CardTitle>FSD & Autopilot</CardTitle>
                <Badge
                    label={boardState.fsd ? "Active" : "Inactive"}
                    variant={boardState.fsd ? "default" : "outline"}
                />
            </CardHeader>
            <CardContent className="gap-2">
                <Button
                    label={`FSD: ${boardState.fsd ? "ON" : "OFF"}`}
                    variant={boardState.fsd ? "default" : "outline"}
                    onPress={() => onCommand(boardState.fsd ? "fsd:off" : "fsd:on")}
                />
                {/* ... more controls ... */}
            </CardContent>
        </Card>
    );
}
```

- [ ] **Step 2: Extract BmsPanel, DasPanel, GamepadPanel, SpeedTuningPanel, VehicleCommandsPanel**

Same pattern — each panel is a Card with its feature controls inside. Panels are independently testable extractables (~60-150 lines each).

- [ ] **Step 3: Simplify ControlsScreen with responsive grid**

```tsx
// client/src/screens/ControlsScreen.tsx (~60 lines after refactor)
export function ControlsScreen({ boardState, onCommand, ...props }: ControlsScreenProps) {
  return (
    <ScrollView className="flex-1 px-3 pt-3" contentContainerClassName="gap-3">
      {/* Mobile: stack all panels. Desktop: 2-column grid */}
      <View className="flex-row flex-wrap gap-3">
        <View className="w-full lg:w-[calc(50%-6px)]"><FsdPanel ... /></View>
        <View className="w-full lg:w-[calc(50%-6px)]"><BmsPanel ... /></View>
      </View>
      <View className="flex-row flex-wrap gap-3">
        <View className="w-full lg:w-[calc(50%-6px)]"><DasPanel ... /></View>
        <View className="w-full lg:w-[calc(50%-6px)]"><GamepadPanel ... /></View>
      </View>
      <View className="flex-row flex-wrap gap-3">
        <View className="w-full lg:w-[calc(50%-6px)]"><SpeedTuningPanel ... /></View>
        <View className="w-full lg:w-[calc(50%-6px)]"><VehicleCommandsPanel ... /></View>
      </View>
    </ScrollView>
  );
}
```

- [ ] **Step 4: Write tests for all panels**

**Files:**

- Create: `tests/components/controls/FsdPanel.test.tsx`
- Create: `tests/components/controls/BmsPanel.test.tsx`
- Create: `tests/components/controls/DasPanel.test.tsx`
- Create: `tests/components/controls/GamepadPanel.test.tsx`
- Create: `tests/components/controls/SpeedTuningPanel.test.tsx`

```tsx
// tests/components/controls/FsdPanel.test.tsx
import { render, fireEvent } from "@testing-library/react-native";
import { FsdPanel } from "../../../src/components/controls/FsdPanel";

const mockState = {
    fsd: false,
    fsdForce: false,
    nagMode: "off",
    speedProfile: 1,
    speedOffset: 0,
    eceR79: false,
    isaChime: false,
    regionCode: 0,
};

describe("FsdPanel", () => {
    it("renders all FSD controls", () => {
        const { getByText } = render(<FsdPanel boardState={mockState} onCommand={jest.fn()} />);
        expect(getByText("FSD & Autopilot")).toBeTruthy();
        expect(getByText("FSD: OFF")).toBeTruthy();
    });

    it("fsd toggle fires fsd:on command", () => {
        const onCommand = jest.fn();
        const { getByText } = render(<FsdPanel boardState={mockState} onCommand={onCommand} />);
        fireEvent.press(getByText("FSD: OFF"));
        expect(onCommand).toHaveBeenCalledWith("fsd:on");
    });

    it("fsd toggle fires fsd:off when already on", () => {
        const onCommand = jest.fn();
        render(<FsdPanel boardState={{ ...mockState, fsd: true }} onCommand={onCommand} />);
        // "FSD: ON" button should fire fsd:off
    });

    it("renders nag mode selector with current mode highlighted", () => {
        /* ... */
    });
    it("profile change fires profile:N command", () => {
        /* ... */
    });
    it("does not render offset controls for legacy variant", () => {
        /* ... */
    });
});
```

- [ ] **Step 5: Run screen tests**

```bash
cd client && npx jest tests/components/controls/ tests/screens/ 2>&1
```

Expected: All pass.

- [ ] **Step 6: Commit**

```bash
git add client/src/components/controls/ client/src/screens/ControlsScreen.tsx tests/components/controls/
git commit -m "refactor: split ControlsScreen (1157→60 lines) into focused panels + tests"
```

- [ ] **Step 4: Run tests**

```bash
cd client && npx jest --testPathPattern="Controls" 2>&1
```

Expected: All pass (update snapshots if snapshot-based).

- [ ] **Step 5: Commit**

```bash
git add client/src/components/controls/ client/src/screens/ControlsScreen.tsx
git commit -m "refactor: split ControlsScreen (1157→60 lines) into focused panels"
```

### Task C3: Split ConnectionHeader

**Files:**

- Create: `client/src/components/transport/TransportPicker.tsx`
- Create: `client/src/components/transport/ConnectSheet.tsx`
- Create: `client/src/components/transport/ConnectionStatusBar.tsx`
- Modify: `client/src/components/ConnectionHeader.tsx`

- [ ] **Step 1: Extract TransportPicker**

Transport type selector (HTTP/BLE/Serial) with icons and labels:

```tsx
// client/src/components/transport/TransportPicker.tsx
export function TransportPicker({ selected, onSelect }: TransportPickerProps) {
    return (
        <View className="flex-row gap-2">
            {TRANSPORT_OPTIONS.map((opt) => (
                <Pressable
                    key={opt.id}
                    onPress={() => onSelect(opt.id)}
                    className={cn(
                        "px-3 py-2 rounded-md border",
                        selected === opt.id ? "border-primary bg-primary/10" : "border-border",
                    )}
                >
                    <Text>{opt.label}</Text>
                </Pressable>
            ))}
        </View>
    );
}
```

- [ ] **Step 2: Extract ConnectSheet**

Connection config: URL input for HTTP, scan/pair for BLE, port picker for Serial, presets, connect button.

- [ ] **Step 3: Extract ConnectionStatusBar**

Top bar showing transport type label, connected/disconnected status with pulse dot, connection type hint.

- [ ] **Step 4: Simplify ConnectionHeader**

```tsx
export function ConnectionHeader() {
    return (
        <View className="flex-row items-center justify-between px-4 py-2 border-b border-border">
            <ConnectionStatusBar />
            <Pressable onPress={() => setSheetOpen(true)}>
                <TransportPicker selected={transportType} onSelect={setType} />
            </Pressable>
            <ConnectSheet open={sheetOpen} onClose={() => setSheetOpen(false)} />
        </View>
    );
}
```

- [ ] **Step 5: Write transport component tests**

**Files:**

- Create: `tests/components/transport/TransportPicker.test.tsx`
- Create: `tests/components/transport/ConnectSheet.test.tsx`
- Create: `tests/components/transport/ConnectionStatusBar.test.tsx`

```tsx
// tests/components/transport/TransportPicker.test.tsx
describe("TransportPicker", () => {
    it("renders HTTP, BLE, Serial options", () => {
        /* ... */
    });
    it("highlights selected option with primary style", () => {
        /* ... */
    });
    it("fires onSelect with correct type on press", () => {
        /* ... */
    });
});

// tests/components/transport/ConnectSheet.test.tsx
describe("ConnectSheet", () => {
    it("shows URL input for HTTP transport", () => {
        /* ... */
    });
    it("shows scan button for BLE transport", () => {
        /* ... */
    });
    it("connect button calls onConnect", () => {
        /* ... */
    });
    it("presets dropdown changes URL", () => {
        /* ... */
    });
});

// tests/components/transport/ConnectionStatusBar.test.tsx
describe("ConnectionStatusBar", () => {
    it("shows 'Connected' with green indicator when connected", () => {
        /* ... */
    });
    it("shows 'Disconnected' with gray indicator when not connected", () => {
        /* ... */
    });
    it("shows 'Connecting...' with orange indicator when busy", () => {
        /* ... */
    });
});
```

- [ ] **Step 6: Run transport tests**

```bash
cd client && npx jest tests/components/transport/ 2>&1
```

- [ ] **Step 7: Commit**

```bash
git add client/src/components/transport/ client/src/components/ConnectionHeader.tsx tests/components/transport/
git commit -m "refactor: split ConnectionHeader (440 lines) into transport/ sub-components + tests"
```

### Task C4: Polish ConsoleScreen, FlasherScreen, DocsScreen

**Files:**

- Modify: `client/src/screens/ConsoleScreen.tsx`
- Modify: `client/src/screens/FlasherScreen.tsx`
- Modify: `client/src/screens/DocsScreen.tsx`

- [ ] **Step 1: ConsoleScreen**

Replace custom container styling with shadcn `Card` and `Separator`. Use `Badge` for bus status indicators. Use `Input` for command text field. Use `Button` for send/export.

**Responsive:** Console sub-navigation adapts:

- **Mobile:** `MonitorBottomBar` (3-tab bar below content)
- **Desktop:** `MonitorSidebarNavigation` (vertical sidebar with `<Separator>` between sections, 200px wide, fixed)

```tsx
const bp = useBreakpoint();
const isWide = bp === "tablet" || bp === "desktop" || bp === "cluster";
{
    isWide ? <MonitorSidebarNavigation /> : <MonitorBottomBar />;
}
```

- [ ] **Step 2: FlasherScreen**

Wrap build options in `Card` components. Use `Select` for connectivity/CAN bus/clock dropdowns. Use `Button` for download/flash actions. Show progress with `Skeleton` during build. Mobile: single column cards. Desktop: 2-column (build options | CLI reference).

- [ ] **Step 3: DriveScreen — Portrait/Landscape responsive layouts**

```tsx
// Mobile portrait: TopBar → SpeedDial → side panels stacked → BottomStrip
// Landscape/tablet: TopBar → [LeftPanel | SpeedDial | RightPanel] → BottomStrip
const { width, height } = useWindowDimensions();
const isLandscape = width > height;

<View className="flex-1">
    <TopBar />
    <View className={cn("flex-1", isLandscape ? "flex-row" : "flex-col")}>
        {isLandscape && <LeftPanel className="w-1/4" />}
        <SpeedDial className={isLandscape ? "flex-1" : ""} />
        <RightPanel className={isLandscape ? "w-1/4" : "mt-2"} />
    </View>
    <BottomStrip />
</View>;
```

**Responsive test file:** `tests/screens/DriveScreen.responsive.test.tsx`

```tsx
describe("DriveScreen responsive", () => {
    it("renders column layout in portrait (height > width)", () => {
        /* mock dimensions 375x812 */
    });
    it("renders row layout in landscape (width > height)", () => {
        /* mock dimensions 812x375 */
    });
    it("shows left + right panels in landscape, hides side panels in portrait", () => {
        /* ... */
    });
});
```

- [ ] **Step 4: DocsScreen**

Use `Separator` between doc tree items. Active doc highlight with primary accent. Use `Card` for doc content area. Mobile: tree is a collapsible `Sheet`. Desktop: tree is a fixed 250px sidebar.

- [ ] **Step 5: Write screen-level responsive tests**

**Files:**

- Create: `tests/screens/DashboardScreen.responsive.test.tsx`
- Create: `tests/screens/ControlsScreen.responsive.test.tsx`
- Create: `tests/screens/DriveScreen.responsive.test.tsx`

```tsx
// tests/screens/DashboardScreen.responsive.test.tsx
import { render } from "@testing-library/react-native";

// Use jest.mock to control useWindowDimensions return values
jest.mock("react-native/Libraries/Utilities/useWindowDimensions", () => ({
    default: () => ({ width: 375, height: 812, scale: 2, fontScale: 1 }),
}));

describe("DashboardScreen responsive", () => {
    it("renders single column on phone (375w)", () => {
        // Verify cards are full-width, no side-by-side
    });

    it("renders 2-column on tablet (768w)", () => {
        jest.resetModules();
        // Remock with width: 768
        // Verify 2 cards per row
    });

    it("uses Sheet (modal) on mobile, Tabs on desktop for Overview/Drive switch", () => {
        /* ... */
    });
});
```

- [ ] **Step 6: Run all tests**

```bash
cd client && npx jest 2>&1
```

- [ ] **Step 7: Commit**

```bash
git add client/src/screens/ tests/screens/
git commit -m "refactor: polish Console/Flasher/Docs/Drive screens + responsive layouts + tests"
```

---

## Phase D: ESP32 Dashboard Redesign

### Task D1: Redesign dashboard.html

**Files:**

- Modify: `firmware/lib/client/dashboard/dashboard.html`
- Modify: `firmware/lib/client/dashboard/dashboard.h` (if serving logic changes)

- [ ] **Step 1: Rewrite with modern CSS Grid, dark mode, and responsive layout**

Replace the 1257-line monolithic HTML with a modern design:

- **CSS Grid layout:** header, status bar, telemetry grid (auto-fill, minmax 280px), feature toggle grid, log section
- **Dark/light mode:** `prefers-color-scheme` media query with CSS custom properties
- **Responsive:** single column on mobile (< 600px), multi-column on tablet/desktop
- **Live data:** auto-refresh via `fetch()` polling every 2s with error state display
- **Tesla red accent** (#d73c1f) for brand consistency with client app
- **No JS framework** — vanilla HTML/CSS, under 20KB gzipped

```html
<!-- Key layout structure: -->
<div class="dashboard">
    <header class="hero"><span class="pulse"></span> TeslaCANModder · HW4 · WiFi</header>

    <div class="status-bar"><!-- FSD, nag, profile, offset badges --></div>

    <div class="grid telemetry">
        <!-- auto-fill grid: 1 col mobile, 2 col tablet, 3 col desktop -->
        <div class="card">
            <h3>HV Battery</h3>
            <span id="voltage">375 V</span><span id="current">5 A</span>
        </div>
        <div class="card">
            <h3>State of Charge</h3>
            <div class="gauge-ring" id="soc-gauge">70%</div>
        </div>
        <div class="card">
            <h3>Temperatures</h3>
            <span id="temp-min">25°C</span><span id="temp-max">32°C</span>
        </div>
        <div class="card">
            <h3>TPMS</h3>
            <span>FL 2.5 · FR 2.5 · RL 2.5 · RR 2.5 bar</span>
        </div>
        <div class="card">
            <h3>Power Limits</h3>
            <span id="regen">85 kW</span><span id="discharge">200 kW</span>
        </div>
        <div class="card">
            <h3>CAN Health</h3>
            <span>Chassis ● Vehicle ● Body ○</span>
        </div>
    </div>

    <div class="grid features">
        <!-- Feature status cards in same auto-fill grid -->
    </div>

    <div class="log">
        <h3>CAN Log</h3>
        <div id="frame-log"><!-- JS-populated --></div>
    </div>
</div>
```

- [ ] **Step 2: Add responsive CSS Grid with mobile-first media queries**

```css
.telemetry,
.features {
    display: grid;
    gap: 12px;
    grid-template-columns: 1fr; /* mobile: single column */
}
@media (min-width: 640px) {
    .telemetry,
    .features {
        grid-template-columns: repeat(2, 1fr);
    }
}
@media (min-width: 1024px) {
    .telemetry,
    .features {
        grid-template-columns: repeat(3, 1fr);
    }
    .wrap {
        max-width: 1100px;
    }
}

.card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 16px;
    box-shadow: var(--shadow);
}
.card h3 {
    font-size: 13px;
    color: var(--muted);
    margin: 0 0 8px 0;
}
```

- [ ] **Step 3: Verify HTML size is under 400 lines**

```bash
(Get-Content "firmware/lib/client/dashboard/dashboard.html" | Measure-Object -Line).Lines
```

Expected: < 400 lines (down from 1257).

- [ ] **Step 4: Test in browser — mobile and desktop viewports**

Open file in Chrome DevTools device toolbar, verify at 375px, 768px, 1024px widths. Verify dark mode toggle (OS setting).

- [ ] **Step 5: Commit**

```bash
git add firmware/lib/client/dashboard/
git commit -m "refactor: redesign ESP32 dashboard with CSS Grid + dark mode + responsive (1257→~350 lines)"
```

- [ ] **Step 2: Add CSS custom properties for theming**

```css
:root {
    --bg: #f8fafc;
    --surface: #ffffff;
    --text: #0f172a;
    --muted: #64748b;
    --border: #e2e8f0;
    --accent: #d73c1f;
    --ok: #22c55e;
    --warn: #f59e0b;
    --radius: 12px;
    --shadow: 0 4px 24px rgba(0, 0, 0, 0.08);
}
@media (prefers-color-scheme: dark) {
    :root {
        --bg: #0f172a;
        --surface: #1e293b;
        --text: #f1f5f9;
        --muted: #94a3b8;
        --border: #334155;
        --shadow: 0 4px 24px rgba(0, 0, 0, 0.3);
    }
}
```

- [ ] **Step 3: Verify HTML size is reasonable**

```bash
wc -l firmware/lib/client/dashboard/dashboard.html
```

Expected: < 400 lines (down from 1257).

- [ ] **Step 4: Test in browser**

Open the file in a browser and verify layout at mobile/desktop widths. Verify dark mode toggle.

- [ ] **Step 5: Commit**

```bash
git add firmware/lib/client/dashboard/
git commit -m "refactor: redesign ESP32 dashboard with CSS Grid + dark mode (1257→~350 lines)"
```

---

## Phase E: Gamepad BLE UX

### Task E1: Improve gamepad pairing flow + responsive binding table

**Files:**

- Modify: `client/src/components/controls/GamepadPanel.tsx`
- Create: `tests/components/controls/GamepadPanel.test.tsx`

- [ ] **Step 1: Add step-by-step pairing wizard**

Replace the single "Scan" button with a 3-step flow (shown inline in `CardContent`):

1. **Scan** — Button "Scan for Gamepads", spinner, discovered devices list
2. **Pair** — Tap device, show "Pairing..." with progress
3. **Paired** — Device name, battery %, RSSI, "Bind Buttons" CTA

**Responsive binding table:** Mobile: vertical list (one button per row, tap to bind). Desktop: horizontal table (columns: Button # | Tap Cmd | Hold Cmd | Bind action) with `overflow-x-auto` for the binding table on narrow screens.

- [ ] **Step 2: Write GamepadPanel tests**

```tsx
// tests/components/controls/GamepadPanel.test.tsx
describe("GamepadPanel", () => {
    it("shows scan button when no gamepad paired", () => {
        /* ... */
    });
    it("shows paired device info when addr exists in boardState", () => {
        /* ... */
    });
    it("scan button fires gamepad:scan command", () => {
        /* ... */
    });
    it("unpair button fires gamepad:unpair command", () => {
        /* ... */
    });
    it("renders binding table with 16 rows", () => {
        /* ... */
    });
    it("binding table is scrollable horizontally on narrow screens", () => {
        /* ... */
    });
});
```

- [ ] **Step 1: Add step-by-step pairing wizard**

Replace the single "Scan" button with a 3-step flow:

1. **Scan** — Press Scan, show scanning spinner, list discovered devices with signal strength
2. **Select** — Tap a device to pair, show pairing progress
3. **Confirm** — Show paired device name, battery %, RSSI, bind buttons CTA

```tsx
export function GamepadPanel({ boardState, onCommand }: PanelProps) {
    const [step, setStep] = useState<"idle" | "scanning" | "pairing" | "paired">("idle");
    const [devices, setDevices] = useState<Device[]>([]);
    const pairedAddr = boardState.gamepad?.pairedAddr;

    if (pairedAddr) {
        return <PairedGamepadCard addr={pairedAddr} boardState={boardState} onBind={openBinding} />;
    }

    return (
        <Card>
            <CardHeader>
                <CardTitle>Gamepad</CardTitle>
            </CardHeader>
            <CardContent>
                {step === "idle" && <Button label="Scan for Gamepads" onPress={startScan} />}
                {step === "scanning" && <ScanningView devices={devices} onSelect={pairDevice} />}
                {step === "pairing" && <Text>Pairing...</Text>}
            </CardContent>
        </Card>
    );
}
```

- [ ] **Step 2: Add button binding interface**

When a gamepad is paired, show a table of 16 buttons with their bound commands:

```tsx
function BindingTable({ bindings, onChange }: BindingTableProps) {
    return (
        <View className="gap-1">
            {Array.from({ length: 16 }).map((_, i) => (
                <View
                    key={i}
                    className="flex-row items-center justify-between py-2 border-b border-border/50"
                >
                    <Text className="text-sm font-medium">Button {i}</Text>
                    <View className="flex-row gap-2">
                        <Badge label={bindings[i]?.tap ?? "—"} variant="outline" />
                        <Badge label={bindings[i]?.hold ?? "—"} variant="secondary" />
                    </View>
                    <Button
                        label="Bind"
                        size="sm"
                        variant="ghost"
                        onPress={() => openBindDialog(i)}
                    />
                </View>
            ))}
        </View>
    );
}
```

- [ ] **Step 3: Commit**

```bash
git add client/src/components/controls/GamepadPanel.tsx
git commit -m "feat: add step-by-step gamepad pairing wizard + binding table"
```

---

## Phase F: Cleanup & final polish

### Task F1: Remove old ui/ components, update imports

**Files:**

- Remove: `client/src/ui/Alert.tsx`, `Badge.tsx`, `Button.tsx`, `Card.tsx`, `Divider.tsx`, `Input.tsx`, `Label.tsx`, `Select.tsx`, `Sheet.tsx`, `Tabs.tsx`
- Modify: All files that imported from old ui/
- Modify: `client/src/ui/index.ts`

- [ ] **Step 1: Update ui/index.ts barrel to point to shadcn**

```ts
export {
    Button,
    Card,
    CardHeader,
    CardTitle,
    CardDescription,
    CardContent,
    CardFooter,
} from "./shadcn";
export {
    Badge,
    Input,
    Label,
    Select,
    Separator,
    Sheet,
    Tabs,
    TabsList,
    TabsTrigger,
    TabsContent,
} from "./shadcn";
export { Alert, Skeleton } from "./shadcn";
```

- [ ] **Step 2: Delete old ui component files**

- [ ] **Step 3: Update all imports across the codebase**

Replace `from "../ui/Card"` → `from "../ui/shadcn/card"` (or use barrel import `from "../ui"` for all)

- [ ] **Step 4: Typecheck + test**

```bash
cd client && npx tsc --noEmit && npx jest 2>&1
```

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "refactor: remove old ui/ components, switch to shadcn"
```

### Task F2: Final QA — all tests + responsive verification

- [ ] **Step 1: Run all client tests (120+ test suites)**

```bash
cd client && npx jest --coverage 2>&1
```

Expected: All pass, coverage > 80%.

- [ ] **Step 2: Typecheck**

```bash
npm run typecheck:client && npm run typecheck:protocol
```

- [ ] **Step 3: Lint + format**

```bash
npm run lint:all
```

- [ ] **Step 4: Responsive QA manual checklist**

Start dev server: `cd client && npm run web`

Verify at these widths (Chrome DevTools device toolbar):
| Width | Check |
|--------|-------------------------------------------------------|
| 375px | Single column, bottom tabs visible, cards full-width |
| 768px | 2-column grids, console shows sidebar |
| 1024px | 3-column grids, all sidebars visible |
| 375x812 | DriveScreen portrait: stacked layout |
| 812x375 | DriveScreen landscape: left/middle/right panels |

Open ESP32 dashboard at 375px, 768px, 1024px — verify grid columns + dark mode.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "chore: final QA — all 120+ tests pass, typecheck clean, responsive verified"
```

Expected: All pass.

- [ ] **Step 2: Run all client tests**

```bash
npm run test:client
```

Expected: All pass.

- [ ] **Step 3: Typecheck client + protocol**

```bash
npm run typecheck:client && npm run typecheck:protocol
```

Expected: No errors.

- [ ] **Step 4: Start dev server and visually verify**

Manual: `cd client && npm start` — open on web, iOS simulator, verify all 5 tabs render, transport picker works, toggles work, no crashes.

- [ ] **Step 5: Commit final**

```bash
git commit -m "chore: final QA pass — lint, tests, typecheck all green"
```
