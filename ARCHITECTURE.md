# NextXI Architecture & AI Map

NextXI is a modern injection engine and addon framework for Final Fantasy XI[cite: 15]. It bridges legacy C++ game code with a modern C# launcher and a LuaJIT addon environment[cite: 15].

## 1. The Pipeline

The engine operates in three distinct phases[cite: 15]:

* **The Launcher (`launcher/`)**: A C# WPF application[cite: 15]. It handles profiles, settings, Steam integration, updates, and launching the PlayOnline Viewer[cite: 15].
* **The Core Engine (`core/`)**: A C++20 DLL injected directly into the FFXI process[cite: 15]. It hooks native Windows and DirectX APIs to control the game loop and render overlays[cite: 15].
* **The Lua Environment (`addons/`)**: An isolated Lua sandbox[cite: 15]. It loads libraries, background services, and user-facing scripts[cite: 15].

## 2. Core Module Boundaries

C++ modifications must strictly adhere to these directory rules[cite: 15]:

* **`core/src/hooks/`**: Exclusively for intercepting native Windows and game functions[cite: 15]. This includes DirectX 8 (`d3d8.cpp`), input (`dinput8.cpp`), networking (`ws2_32.cpp`), and the main FFXI loop (`ffximain.cpp`)[cite: 15].
* **`core/src/wrappers/`**: The translation layer[cite: 15]. Wraps legacy game calls and routes them safely (`direct_3d_device.cpp`, `direct_input.cpp`)[cite: 15].
* **`core/src/ui/`**: The presentation layer[cite: 15]. Handles custom widgets, window management, hardware-accelerated primitives, and text rasterization over the game window[cite: 15].
* **`core/src/addon/`**: The FFI bridge[cite: 15]. Defines how Lua asks C++ for memory, commands, and packet manipulation[cite: 15].
* **`core/src/utilities/`**: Engine utilities[cite: 15]. Handles asynchronous logging, memory signature scanning, and Shift-JIS to UTF-8 translation[cite: 15].

## 3. Addon Ecosystem

The Lua sandbox is split into three layers[cite: 15]:

* **Services (`addons/libs/*_service/`)**: Background daemons[cite: 15]. They quietly parse incoming network packets into structured data (e.g., `items_service`, `action_service`)[cite: 15].
* **Libraries (`addons/libs/`)**: Developer APIs[cite: 15]. Provides native functions like `socket`, `mime`, `struct`, and `memory` for addon creators[cite: 15].
* **User Addons (`addons/`)**: Frontend tools[cite: 15]. Examples include `AddonManager` for UI toggles and `config` for FPS/draw distance tweaks[cite: 15].

## 4. AI Coding Conventions

When generating or refactoring code for NextXI, AI agents MUST obey these rules[cite: 15]:

* **File Size**: No single C# or C++ file should exceed 350 lines[cite: 15]. Split bloated files into single-responsibility modules[cite: 15].
* **Exceptions**: Never silently swallow exceptions in background tasks[cite: 15]. Catch blocks must unwrap and log the full error via the core logger[cite: 15].
* **Strings**: Use explicit UTF-8 strings (`std::u8string`, `u8""`) for all C++ logging and UI text[cite: 15]. FFXI uses Shift-JIS; standard strings will corrupt rendering[cite: 15].
* **Casts**: No legacy C-style casts in C++[cite: 15]. Use `static_cast`, `reinterpret_cast`, or `gsl::narrow_cast`[cite: 15].