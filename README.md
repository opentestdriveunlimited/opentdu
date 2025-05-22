# Open Test Drive Unlimited (OpenTDU)
Source port for Test Drive Unlimited (TDU). 

The goal of this project is to provide a modern client fixing various issues encountered on modern systems (AI, rendering, security, etc.) as well as providing cross-platform compatibility.
**This project is not a replacement for the game (you still need to own a legal copy of the game); and does not provide any game assets!**

## Project Status
Work in progress.

## Download
If you do not want to build from source, prebuilt executables are available here: [https://github.com/opentestdriveunlimited/opentdu/releases](https://github.com/opentestdriveunlimited/opentdu/releases).

## Build instructions
### Requirements
-CMake 3.15 (or newer)
-C++17 compatible compiler (e.g. Visual Studio 2022)

### Instructions
1. Use CMake to generate a project for your desired code generator
2. Build the client (opentdu) and the asset_extractor tool (opentdu_assetExtractor)
3. Run asset_extractor first (to extract configuration files and shader binaries from the original .exe). Make sure the assets are extracted next to the executable.
4. Run opentdu client
