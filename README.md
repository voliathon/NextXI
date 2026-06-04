# NextXI

NextXI is an advanced addon framework and launcher tailored for the game environment.

## Running in Visual Studio Insiders

To run and debug the NextXI application locally:

1. Open the **NextXI** project folder in **Visual Studio Insiders**.
2. Set the build configuration to `Debug` or `Release` according to your needs.
3. If building for the first time, ensure all project dependencies are restored.
4. Press `F5` to build the solution and launch the debugger, or `Ctrl+F5` to run without debugging.
5. The `build/bin/debug` (or release) directory will be populated with the executable and required core scripts.

### Note on Directory Structure
- All core binaries are built into the `build/` folder.
- Addon source code is kept in the `/addons/` directory. 
- Shared libraries are located in `/addons/libs/` so all addons can utilize them universally.
