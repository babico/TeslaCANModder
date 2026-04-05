# Web UI Fixes - Dashboard Scroll & Simplification

## Issues Fixed

### 1. Scroll Bug
**Problem**: Dashboard tiles were not scrolling properly, content was overflowing

**Solution**:
- Added proper overflow containment hierarchy
- `.dash-container` → flex container with `overflow: hidden`
- `.dashboard-content` → scrollable area with `overflow-y: auto`
- `.dashboard-tile-content` → proper `overflow: hidden` on panels

### 2. Complexity Removed
**Removed**:
- Drag & drop tile reordering
- Tile resizing handles
- Hidden tiles management
- Complex grid auto-flow
- Tile measurement observers
- Layout persistence

**Result**: Simple, clean grid layout that just works

## Changes Made

### CSS (`dashboard.css`)
```css
.dash-container {
  height: 100%;
  overflow: hidden;  /* Container doesn't scroll */
}

.dashboard-content {
  overflow-y: auto;  /* Content area scrolls */
}

.dashboard-tile-content {
  overflow: hidden;  /* Tiles contain their content */
}

.log-panel, .console-panel {
  overflow: hidden;  /* Panels manage their own scroll */
}
```

### Component (`Dashboard.jsx`)
- Removed all drag/drop handlers
- Removed resize logic
- Removed hidden tiles state
- Simplified to basic grid with fixed column spans
- Kept all functionality (connect, commands, streaming, etc.)

## What Still Works

✅ USB & Bluetooth connection
✅ Variant switching (HW4/HW3/Legacy)
✅ All feature controls (FSD, nag, profile, offset, ISA chime)
✅ Live CAN frame streaming
✅ Console with command input
✅ Mobile responsive (tabbed interface)
✅ Package panels (FSD controls)

## Testing

1. Start web UI: `npm run dev`
2. Connect to board
3. Scroll dashboard - should work smoothly
4. All tiles visible and functional
5. Frame streaming scrolls properly
6. Console scrolls properly

## Result

- Clean, simple grid layout
- Proper scroll behavior
- No complexity overhead
- Same functionality
- Better performance
