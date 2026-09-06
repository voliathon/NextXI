# NextXI Addons & Dependencies

**⚠️ ARCHITECTURAL NOTE FOR FUTURE DEVELOPMENT ⚠️**

Currently, core dependencies and official Windower resources are downloaded and extracted automatically 
via a **Visual Studio Post-Build Event** using PowerShell. 

*   `addons/shared_libs/`: Populated with `resources_data` directly from the `Windower/Resources` GitHub repository.
*   `addons/windower/libs/`: Populated with legacy Lua libraries directly from the `Windower/Lua` GitHub repository.
*   `addons/nextxi/libs/`: Contains our hard-forked NextXI native dependencies (e.g., `mime`, `target`, `account_service`). 
These are safely committed directly to this repository.

### The Future Goal: Launcher Migration
The NextXI C++ Core Engine has been completely stripped of its internal downloader and no longer reaches out to Fenestra or Windower servers. 
It is 100% offline.

In the future, the downloading, updating, and verifying of `shared_libs` and legacy `windower/libs` should be removed from the Visual Studio 
build process and built directly into the **NextXI C# Launcher**. The Launcher must assume responsibility for updating these resource files 
before `pol.exe` is ever injected.



Visual Studio Post-Build Script
-------------------------------------------
:: 1. Create the base folders
mkdir "$(OutDir)addons\shared_libs" 2>nul
mkdir "$(OutDir)addons\windower\libs" 2>nul

:: 2. Download and extract Windower Resources (resources_data) into shared_libs
powershell -NonInteractive -NoProfile -Command "Write-Host 'Downloading resources_data...'; Invoke-WebRequest -Uri 'https://github.com/Windower/Resources/archive/refs/heads/master.zip' -OutFile '$(OutDir)addons\shared_libs\res.zip'; Expand-Archive -Path '$(OutDir)addons\shared_libs\res.zip' -DestinationPath '$(OutDir)addons\shared_libs\temp_res' -Force; if (Test-Path '$(OutDir)addons\shared_libs\resources_data') { Remove-Item '$(OutDir)addons\shared_libs\resources_data' -Recurse -Force }; Move-Item -Path '$(OutDir)addons\shared_libs\temp_res\Resources-master\resources_data' -Destination '$(OutDir)addons\shared_libs\' -Force; Remove-Item '$(OutDir)addons\shared_libs\res.zip' -Force; Remove-Item '$(OutDir)addons\shared_libs\temp_res' -Recurse -Force"

:: 3. Download and extract Windower Lua Libs into windower/libs
powershell -NonInteractive -NoProfile -Command "Write-Host 'Downloading windower libs...'; Invoke-WebRequest -Uri 'https://github.com/Windower/Lua/archive/refs/heads/dev.zip' -OutFile '$(OutDir)addons\windower\lua.zip'; Expand-Archive -Path '$(OutDir)addons\windower\lua.zip' -DestinationPath '$(OutDir)addons\windower\temp_lua' -Force; if (Test-Path '$(OutDir)addons\windower\libs') { Remove-Item '$(OutDir)addons\windower\libs' -Recurse -Force }; Move-Item -Path '$(OutDir)addons\windower\temp_lua\Lua-dev\addons\libs' -Destination '$(OutDir)addons\windower\' -Force; Remove-Item '$(OutDir)addons\windower\lua.zip' -Force; Remove-Item '$(OutDir)addons\windower\temp_lua' -Recurse -Force"

exit 0