# FPS Monitor

A native ImGui framerate monitor for NextXI. It draws a lightweight, borderless text overlay to track client performance in real-time.

## Installation
1. Place the `fps` folder inside your `NextXI/addons/` directory.
2. Ensure the folder contains `fps.lua`, `manifest.xml`, and this `README.md`.

## Usage
* **Load:** `//load fps`
* **Unload:** `//unload fps`

### Toggles
You can show or hide the overlay using the following methods:
* **Keyboard:** Press `F12`
* **Command:** `//fps` or `//fps toggle`

## Developer Note: The F12 Crash
Windows hardcodes `F12` as a kernel-level debugger hotkey. **If you are running NextXI attached to the Visual Studio Debugger, pressing F12 will trigger a `DebugBreak()` exception and crash the engine.** 

To test the `F12` toggle safely:
* Launch the engine from Visual Studio using **Start Without Debugging (Ctrl+F5)**.
* Alternatively, change the hotkey to `F11` (DIK code 87) inside `fps.lua` while debugging.