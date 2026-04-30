# UI Library (Fenestra Immediate-Mode GUI)

The `ui` library is a native, immediate-mode graphical user interface wrapper for the Windower 5 C++ engine. It allows developers to draw reactive, high-performance DirectX overlays, windows, and widgets without managing complex object hierarchies.

Because it is an immediate-mode UI, you must draw your interface every frame inside a render loop. 

---

## 1. The Rendering Loop

### `ui.display(draw_function)`
The master coroutine hook. You must wrap your entire UI logic inside this function. It schedules the render loop with the C++ engine and automatically yields/resumes every frame.

### `ui.screen(draw_function)`
Draws elements directly to the root DirectX screen overlay (HUD style), completely bypassing window containers.

---

## 2. Windows & State Objects

Because the UI is drawn fresh every frame, components that need to remember data (like where a window was dragged, or where a scrollbar is positioned) require a **State Object** declared *outside* the render loop.

### `ui.window_state()`
Creates a persistent state object for a window.
* **Properties:** `.title`, `.x`, `.y`, `.width`, `.height`, `.visible`, `.movable`, `.resizable`, `.closeable`, `.style` (`'standard'`, `'tooltip'`, `'chromeless'`).

### `ui.window(state, draw_function)`
Renders a draggable, floating window to the screen using the provided state.

### Other State Constructors
* `ui.scroll_panel_state(width, height)`: Tracks scrollbar offsets.
* `ui.edit_state()`: Tracks user keyboard input for text boxes.
* `ui.image_button_descriptor(image_data, width, height)`: Tracks image states (normal, hot, active, disabled).
* `ui.progress_entries(count)`: Creates a tracked array of progress bar data.

---

## 3. Layout & Positioning Commands

Widgets are drawn sequentially from top to bottom. You can manipulate the layout engine before drawing a widget to change where it appears.

Call these directly on the `layout` object passed into your draw functions:

* `layout:same_line()`: Forces the next widget to draw to the right of the previous one, instead of below it.
* `layout:space(pixels)`: Adds margin space before the next widget.
* `layout:indent(pixels)`: Indents the current column to the right.
* `layout:unindent(pixels)`: Pulls the column back to the left.
* `layout:padding(left, top, right, bottom)`: Sets internal padding for the layout boundaries.
* `layout:move(x, y)`: Hardcodes the exact X/Y pixel coordinate for the next widget.
* `layout:size(width, height)`: Forces the next widget to be an exact pixel size.
* `layout:width(width)` / `layout:height(height)`: Forces only one dimension of the next widget.
* `layout:fill()`: Forces the next widget to stretch and consume all remaining space in the layout.

---

## 4. The Widget Library

Widgets are executed as methods on the `layout` object. 
*Note: Most interactive widgets return two variables: `clicked` (a boolean indicating if it was clicked this frame) and `state` (a table containing deep interaction data).*

### `layout:label(text)`
Draws simple, non-interactive text.

### `layout:button(id, text, [checked])`
Draws a standard clickable button.

### `layout:check(id, text, current_state)`
Draws a checkbox.

### `layout:radio(id, text, current_state)`
Draws a radio button.

### `layout:slider(id, value, min, max, [fill_color], [direction])`
Draws a draggable slider and returns the new value.

### `layout:edit(edit_state)`
Draws a text-input box. Requires an `edit_state` object.

### `layout:link(id, text)`
Draws clickable, underlined text.

### `layout:scroll_panel(scroll_state, draw_function)`
Creates a nested, scrollable viewport.

### `layout:progress(value, max, [color], [direction])`
Draws a loading/progress bar.

### `layout:color_picker(id, current_color, [alpha_enabled])`
Draws an interactive color-picker canvas. Returns the newly selected hex color integer.

### `layout:image(texture_path, [patch], [color])`
Draws a static image or texture to the UI.

### `layout:image_button(id, image_descriptor)`
Draws a clickable button using a custom image atlas instead of standard UI rendering.

---

## 5. Layout Scopes & Modifiers

### `layout:enabled(boolean)`
Disables/Enables interactivity for all subsequent widgets in the current scope. Disabled widgets appear grayed out.

### `layout:scope(draw_function)`
Isolates layout modifiers (like indenting or disabling) so they don't leak out to the rest of the window.

---

## 6. Colors

The `ui.color` table provides CSS Level 4 compliant colors, system theme colors, and utility functions for manipulation.

**System Theme Colors:**
These automatically adapt if the user changes their Windower 5 theme.
* `ui.color.skin_accent`
* `ui.color.skin_window_title`
* `ui.color.skin_label`
* `ui.color.system_white` / `ui.color.system_error`

**Color Math Functions:**
* `ui.color.rgb(r, g, b, [a])`: Converts standard 0-255 RGB into an engine-safe integer.
* `ui.color.torgb(color_int)`: Unpacks an integer into `r`, `g`, `b`, `a` values.
* `ui.color.hsv(h, s, v, [a])`: Creates a color using Hue, Saturation, Value.
* `ui.color.tohsv(color_int)`: Unpacks an integer into `h`, `s`, `v` values.
* `ui.color.fade(color_int, alpha_0_to_255)`: Returns a new color with adjusted opacity.
* `ui.color.tohex(color_int)`: Converts an engine color integer into a `#RRGGBBAA` hex string.