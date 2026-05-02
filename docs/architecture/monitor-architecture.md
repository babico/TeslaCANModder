---
title: Monitor Architecture
description: How to integrate the responsive MonitorScreen component into AppExperience
category: architecture
folder: architecture
tags: [monitor, integration, screens]
order: 20
---

## Overview

The new `MonitorScreen.tsx` component is a complete replacement for the bloated inline Monitor code that was previously in `AppExperience.tsx`. It provides a clean, responsive, tabbed interface for diagnostics, frame monitoring, decoding, and command execution.

## Status

✅ **Component Created**: `client/src/components/MonitorScreen.tsx` (608 lines)
✅ **Fully Typed**: All props are TypeScript interfaces
✅ **Responsive**: Mobile→Tablet→Desktop adaptive layouts
✅ **Exported**: Both named export and default export available

## Integration Steps

### Step 1: Import the Component

In `client/src/AppExperience.tsx`, add to imports:

```typescript
import { MonitorScreen } from "./components/MonitorScreen";
```

### Step 2: Replace the Old Monitor Rendering

Find the old Monitor tab rendering in the `return` statement (currently around line 1900+):

```typescript
// OLD CODE - Replace this entire block:
{activeTab === "monitor" ? (
  <ScrollView contentContainerStyle={styles.page}>
    {/* ... massive inline Monitor code ... */}
  </ScrollView>
) : null}
```

With:

```typescript
// NEW CODE:
{activeTab === "monitor" ? (
  <MonitorScreen
    boardState={boardState}
    selectedTransportType={selectedTransportType}
    selectedTransportOption={selectedTransportOption}
    transportStatus={transportStatus}
    isSelectedTransportReady={isSelectedTransportReady}
    frameCount={boardState.frameCount}
    visibleFrames={sampledVisibleFrames}
    selectedFrame={selectedFrame}
    frameFilter={frameFilter}
    busFilter={busFilter}
    frameFeedPaused={frameFeedPaused}
    frameWindowSize={frameWindowSize}
    frameSampleStep={frameSampleStep}
    selectedDecoderDataset={selectedDecoderDataset}
    decodedEntries={decodedEntries}
    diagnosticsQuery={diagnosticsQuery}
    diagnosticsCategory={diagnosticsCategory}
    statusText={statusText}
    lastResult={lastResult}
    history={history}
    onFrameFilterChange={setFrameFilter}
    onBusFilterChange={setBusFilter}
    onFrameFeedPausedChange={setFrameFeedPaused}
    onFrameSelect={setSelectedFrameKey}
    onDiagnosticsQueryChange={setDiagnosticsQuery}
    onDiagnosticsCategoryChange={setDiagnosticsCategory}
    onDatasetChange={setDecoderDatasetId}
    onRunCommand={runCommand}
    onFetchStatus={readStatus}
    onExportJson={exportVisibleFramesJson}
    onExportCsv={exportVisibleFramesCsv}
    onClearFeed={clearMonitorFeed}
  />
) : null}
```

### Step 3: Remove Old Styles

Delete all Monitor-related styles from the bottom of `AppExperience.tsx`. These are no longer needed:

- `monitorHero`
- `eyebrow`, `subtitle`, `subtitleHero`
- `heroFeatureRow`, `heroGrid`
- `transportChip*`
- `transportHelper`
- `gateWarning`
- `presetRow`, `presetChip`
- `commandList`, `commandListContent`
- `frameRow*`
- `decodedCard*`
- `historyList`, `historyRow*`
- `okBadge`, `errBadge`
- And any other Monitor-specific styles

### Step 4: Validate

```bash
# Typecheck
npm run -w @teslacanmodder/client typecheck

# Run tests
npm run -w @teslacanmodder/client test
```

## Benefits

| Before                       | After                           |
| ---------------------------- | ------------------------------- |
| 2000+ lines in AppExperience | <300 lines (just state & logic) |
| Inline styles scattered      | Encapsulated in component       |
| Single-form-factor layout    | Responsive browser + mobile     |
| Single giant view            | Organized tabs                  |
| Hard to test UI              | Pure state logic                |
| Monolithic code              | Modular, reusable component     |

## Component Sections

The MonitorScreen provides four organized tabs:

1. **Monitor** - Live CAN frame feed with filtering, windowing, snapshots
2. **Decoder** - Signal decoding by dataset, frame details
3. **Connection** - Transport selection, status output, exports
4. **Diagnostics** - Command history, event filtering, analytics

## Props Reference

See `MonitorScreenProps` interface in `MonitorScreen.tsx` for complete prop documentation.

## Migration Checklist

- [ ] Add `import { MonitorScreen } from "./components/MonitorScreen"`
- [ ] Replace old Monitor tab rendering code
- [ ] Delete old Monitor-specific styles
- [ ] Run typecheck (should pass)
- [ ] Run tests (244 should pass)
- [ ] Test Monitor tab in the browser target
- [ ] Remove old commented code

## Rollback

If needed, the old code is still in git history. Run:

```bash
git checkout HEAD~1 client/src/AppExperience.tsx
```

## Notes

- MonitorScreen uses the same state management as before (hooks in AppExperience)
- All view logic is extracted; state orchestration remains in AppExperience
- Component is fully typed - no `any` types
- Responsive design uses `useWindowDimensions()` for breakpoints
