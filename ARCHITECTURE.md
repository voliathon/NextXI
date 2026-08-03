# NextXI Architecture & AI Map

NextXI is a modern, DX11-based injection engine and addon framework for Final Fantasy XI, designed to replace the legacy D3D8 Windower 4 architecture while maintaining Lua addon compatibility.

## 1. The Pipeline

The engine operates in three distinct phases, crossing multiple language boundaries:

1. **The Launcher (C# / WPF):** 
   - Lives in `launcher/`.
   - Responsible for UI, configuration, finding the PlayOnline/FFXI installation, and bootstrapping the game.
   - Core injection logic is handled by `launcher/src/Core/Injector.cs`.
2. **The Core Engine (C++20):** 
   - Lives in `core/`.
   - Injected into the FFXI process. It immediately hooks the Windows API and DirectX via `core/src/hooks/` (e.g., `ffximain.cpp`, `d3d8.cpp`, `user32.cpp`).
   - Uses `dgVoodoo2` to translate legacy D3D8 calls into D3D11.
   - Intercepts the `IDXGISwapChain::Present` pipeline to render modern, hardware-accelerated UI overlays over the game via `core/src/ui/`.
3. **The Lua Environment (C++ to LuaJIT):** 
   - Managed by `core/src/addon/` (specifically `lua.cpp` and `script_environment.cpp`).
   - Boots up the isolated Lua states.
   - Loads the user-facing scripts located in `addons/` and `scripts/`.

## 2. Module Boundaries

If you are modifying NextXI, strictly adhere to these directory boundaries:

* **`core/src/hooks/`**: ONLY for intercepting native Windows/Game functions (DirectX, DirectInput, WinSock, memory offsets).
* **`core/src/wrappers/`**: The translation layer. This wraps legacy game calls and safely routes them to modern APIs (e.g., `direct_3d_device.cpp`).
* **`core/src/ui/`**: The modern DX11 rendering engine. Handles ImGui, text rasterization, and drawing primitives on top of the FFXI window.
* **`core/src/addon/`**: The bridge between C++ and Lua. Defines how Lua addons ask C++ for memory, packets, and UI draws.
* **`launcher/`**: Strictly C# desktop application logic. Do not put game-loop logic here.

## 3. State Management

* **Global State:** Minimized as much as possible. Core engine state is managed by `core/src/core.cpp` and subsystem managers (e.g., `CommandManager`, `WindowManager`).
* **Asynchronous Tasks:** Addons initialize asynchronously. Exceptions in background tasks MUST be caught and explicitly unwrapped via the core logger; they must never be silently swallowed.
* **Memory Hooks:** Managed explicitly via `core/src/hooklib/` (trampolines) and `core/src/utilities/sigscan.cpp`.

## 4. AI Coding Conventions

When generating or refactoring code for this project, AI agents MUST obey these rules:

1. **Strict 350-Line Limit:** No single C# or C++ file should exceed 350 lines. If a file grows beyond this, split it into single-responsibility modules (Data Models, Business Logic, Utility Headers).
2. **Never Swallow Exceptions:** `catch(...)` or `catch(std::exception&)` blocks must always log the full unwrapped error string and stack trace. 
3. **String Formatting:** Use explicit UTF-8 strings (`std::u8string`, `u8""`) for all C++ console logging and UI text to maintain compatibility with FFXI's Japanese Shift-JIS / modern UTF-8 bridging.
4. **No Legacy C-Style Casts:** Use `static_cast`, `reinterpret_cast`, or `gsl::narrow_cast` exclusively in the C++ core.