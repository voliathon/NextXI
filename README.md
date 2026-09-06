<p align="center">
  <img src="/launcher/res/NextXI_Logo.png" alt="NextXI">
</p>

# NextXI

NextXI is a C++20 engine wrapper and C# .NET launcher for Final Fantasy XI that replaces legacy DirectX 8 hooks to enable DirectX 8/11/12 rendering, window management, and Lua 5.1 addon execution.

## Prerequisites & Installation

**Prerequisites:**
* Windows 10 or later
* .NET 10 Desktop Runtime (x86)
* Visual Studio 2022 (C++ Desktop and .NET Desktop workloads)
* vcpkg (for C++ package management)
* PlayOnline Viewer and Final Fantasy XI client

> **Linux & Steam Deck Note:** If you are running NextXI via WINE or Proton, you must install the .NET 10 Desktop Runtime into your prefix before running the launcher. You can do this via `winetricks dotnetdesktop10` or by executing the Windows installer directly inside your prefix.

**Installation:**
```cmd
git clone [https://github.com/voliathon/NextXI.git](https://github.com/voliathon/NextXI.git)
cd NextXI
msbuild nextxi.sln /p:Configuration=Release /p:Platform=x86
```

## Usage

Launch the C# desktop application to configure game profiles and inject the C++ core into the PlayOnline process.

```cmd
.\launcher\bin\Release\NextXI.Launcher.exe
```

Once injected and in-game, interact with the Lua scripting engine via the chat interface:

```text
//load fps
//unload fps
```

## Configuration

Settings are managed via the launcher and stored in an XML profile on disk. Addon configurations reside in the `addons` directory.

* **Profile Location:** `settings/profiles.xml`
* **Addon Location:** `addons/<addon_name>/`

**Primary Profile Variables:**
* `graphics-engine`: Sets the rendering backend (e.g., `Direct3D12` via dgVoodoo2).
* `fps-divisor`: Sets the memory patch value for the frame rate limit (`1` = 60 FPS, `2` = 30 FPS, `0` = uncapped).
* `vram-allocation`: Sets the simulated video memory size in megabytes.