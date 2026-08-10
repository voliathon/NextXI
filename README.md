# NextXI

**NextXI** is a modern, modular, and high-performance engine wrapper and launcher for *Final Fantasy XI*. 

Built to modernize a classic game, NextXI intercepts and elevates the aging FFXI client to run flawlessly on modern operating systems. It replaces brittle, legacy hooks with a clean, refactored architecture, providing a robust foundation for modern rendering, custom UI overlays, and advanced Lua addon scripting.

---

##  Architecture: Core vs. Launcher

The NextXI project is split into two distinct halves to ensure maximum stability and ease of development:

### 1. The Launcher (C#)
Located in the `launcher/` directory, this is a modern .NET desktop application. It acts as the user's front door. The launcher handles configuration management, account profiles, and safely bootstraps the injection process, ensuring that the C++ engine hooks are securely loaded into the PlayOnline Viewer (`pol.exe`) before the game boots.

### 2. The Core (C++)
Located in the `core/` directory, this is the beating heart of NextXI. It is a native C++ DLL injected directly into the game process. The Core is strictly modularized into isolated domains (Window Management, Input, DirectX Rendering, Memory) and is responsible for:
* Intercepting the legacy DirectX 8 graphics pipeline.
* Pumping window messages (`user32`) for safe alt-tabbing and borderless dragging.
* Running the Lua addon environment.
* Rendering the hardware-accelerated Dear ImGui overlay.

---

##  DirectX 12 Modernization

Final Fantasy XI natively runs on a deeply outdated DirectX 8 renderer. NextXI is specifically engineered to intercept these legacy D3D8 calls and play flawlessly with modern translation layers like **dgVoodoo2**. 

By gracefully managing the legacy graphics pipeline and window states, NextXI allows FFXI to run on a modern **DirectX 12** flip-model swapchain. This unlocks massive performance improvements, eliminates screen-tearing, enables native hardware-accelerated UI overlays, and allows the game to utilize modern GPU features without panicking or deadlocking the legacy D3D device.

---

##  Technologies Used

* **C++20** - The native language of the NextXI Core, utilizing modern C++ standards, smart pointers, and strict static analysis.
* **C# / .NET** - Used for the lightweight, fast, and reliable NextXI Launcher.
* **Dear ImGui** - Powers the hardware-accelerated, bloat-free in-game UI overlay.
* **Lua 5.1** - The embedded scripting engine that drives the NextXI addon ecosystem.
* **DirectX 8 / DirectX 12** - Hooking legacy D3D8 to bridge the gap to modern DX12 rendering backends.

---

##  Documentation & Wiki

For everything from compiling the source code to writing your first Lua addon, please consult the official Wiki. It contains detailed setup guides, architecture overviews, and troubleshooting steps for common third-party software conflicts (like DisplayFusion).

**📖 Visit the NextXI Wiki here:** [https://github.com/voliathon/NextXI/wiki](https://github.com/voliathon/NextXI/wiki)