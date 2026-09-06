# NextXI Architecture

NextXI is a modernized, high-performance injection engine and launcher for Final Fantasy XI. It is designed to strictly 
separate the C++ core engine, the C# WPF launcher, and the Lua-based addon ecosystems.

## High-Level Project Structure

*   **`launcher/`**
    *   A C# Windows Presentation Foundation (WPF) application.
    *   Responsible for managing PlayOnline profiles, updating game files, and injecting the `core.dll` into `pol.exe`.
*   **`core/`**
    *   The native C++20 injection engine.
    *   Intercepts DirectX/Direct3D calls to draw an ImGui overlay.
    *   Hosts the Lua Virtual Machine and provides native C++ hooks to the game's memory, network packets, and UI.
*   **`addons/`**
    *   The Lua scripting ecosystem, strictly partitioned into three distinct vaults to prevent legacy code from polluting the modern engine.
    *   **`nextxi/`**: Modern, native NextXI addons (e.g., `config`, `caveman_test`) and hard-forked core libraries (e.g., `mime`, `target`, `account_service`).
    *   **`shared_libs/`**: Live-updated game data (e.g., `resources_data`). Downloaded automatically from the official Windower GitHub via Post-Build scripts.
    *   **`windower/`**: Legacy Windower 4 addons and their respective Lua libraries. Downloaded automatically via Post-Build scripts.

## Core Engine Subsystems (`core/src/`)

*   **`addon/`**: Manages the Lua Virtual Machine (`lua.cpp`), parses `manifest.xml` files (`package.cpp`), resolves topological dependency load orders 
				  (`package_manager_graph.cpp`), and binds C++ functions to Lua modules (`modules/`).
*   **`ui/`**: The ImGui-based visual layer. Includes the `addon_browser`, the `engine_console`, and custom rendering widgets.
*   **`hooks/`**: The low-level API interceptors. Hooks into `d3d8`, `ddraw`, `dinput8`, and `ws2_32` to capture rendering, 
				  inputs, and network traffic before the game processes them.
*   **`utilities/`**: Signature scanning (`sigscan.cpp`), XML parsing (`xml.cpp`), and internal engine helpers.

## Build Pipeline

NextXI uses MSBuild/Visual Studio. The C++ engine is entirely offline and self-sufficient. External dependencies 
(like live FFXI resource data and legacy Lua libs) are deliberately stripped from the C++ bootloader and are 
instead fetched via **PowerShell Post-Build Events** during compilation, ensuring the runtime environment remains lightweight and secure.