# NextXI Core Packages

**⚠️ DO NOT DELETE THESE FOLDERS ⚠️**

This directory contains the foundational data packages required for the NextXI Core Engine to inject and boot successfully. If these folders are missing, the Lua engine will throw a `Missing required dependency` fatal error and crash the client during injection.

### Package Glossary:

* **`resources_data`**
  The master database. It contains all the translated game data (Item IDs, Spell Names, Zone IDs, Buffs, etc.). Without this, the engine and addons cannot translate raw network IDs (like `1042`) into human-readable strings (like `"Cure IV"`).

* **`target`**
  The memory signature and offset maps. Final Fantasy XI patches frequently, which shifts the memory addresses of the client. This package tells our C++ hooks exactly where to inject into `pol.exe` so the game doesn't instantly crash to desktop.

* **`mime`**
  A robust data serialization and encoding library used by the core for handling, formatting, and packing raw network packets between the client and the server.

---
*Note: During the Visual Studio build process, these folders are automatically copied into the `build/bin/<config>/addons/` directory to ensure fresh installs boot flawlessly.*