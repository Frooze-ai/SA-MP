# SA-MP
A repository for making the latest official SA-MP version open source by reverse engineering it's binaries.

San Andreas Multiplayer (SA-MP) is a free, unofficial online multiplayer modification for the original PC version of Grand Theft Auto: San Andreas, a game developed by Rockstar North and released in 2005.

SA-MP allows users to play each other over internet or LAN with up to 1.000 other people and do almost anything that you can do in single player. This mod also features the ability to create your own scripted game modes using PAWN scripting language. There are many game modes that feature a series of different and fun things to do. Many of which are not possible in single player.

**Be aware that this is not an official source code! This repository is not affiliated with or endorsed by SA-MP Team, nor Rockstar Games, Rockstar North or Take-Two Interactive**

## Status
**The project have halted for an undetermined period of time.**

Project is STILL in reversing state! Building any of the components may fail or may be unstable. In this current state it's also does not support the latest
official version.

## Layout
| Folder | Description |
| ------ | ----------- |
| announce | Server component to send required information to the master server and be listed in server browsers. |
| archive | Files to build SAA archive file. |
| arctool2 | Source code of the tool to create SAA2 archive file. |
| exgui | SA-MP server browser. |
| font | Contains font files to render text like weapons or object edit icons. |
| idb | IDA Pro databases for reversing the mod |
| launch3 | Basic GUI appliction to launch GTA: SA with the required command lines to run SA-MP. |
| nsis | Install wizard configuration file for Nullsoft Scriptable Install System |
| pawno | PAWN script editor and compiler with the latest include files. |
| raknet | Shared networking library between `saco`, `server` and `testbot` |
| rcon | Remote console client for the server owners. |
| remoteac | Not used. It's a leftover project from 0.2X and 0.1b anti cheat measurements. |
| saco | Client side DLL which will be injected in to the game. |
| scm | Custom SCM script file. |
| server | The dedotated server. |
| testbot | Not used. Leftover testing tool from 0.2X |

## Requirements
- Copy of GTA: San Andreas for PC
  - Must be EU/US V1.0, it does not support V1.01, V2.0, V3.0 nor Microsoft Store or Definitive Edition
- Visual Studio 2022 Community
  - Professional or Enterprise also works. 
  - Make sure `Desktop development with C++` workload is checked, and in the `Optional` list also make sure `C++ ATL for latest v143 build tools (x86 & x64)` and `C++ MFC for latest v143 build tools (x86 & x64)` also checked.
- Borland Delphi 7
  - Only required for building `exgui`, the SA-MP server browser 
- [optional] IDA Free v8.2
  - For viewing `*.idb` files in the `idb` folder.

These requirements are only for building Windows binaries. Building on Linux is currently not supported.

## Building <sub>for dummies</sub>

#### For building `saco`, `server`, `announce`, `launch3`, `rcon`, `arctool2`
1. Go to any of the folder.
2. Open `.sln` or `.vcxproj` to open the project's solution.
3. Make sure `Release` and `x86` configuration selected.
4. Go to `Build` and select `Build Solution` or press `F7` on your keyboard.
5. An output console will pop-up and wait until the sucessfully finishes.
6. When the compilation finshes go bact to your folder, and there will be a `Release` folder, and you'll find your binary.**

#### For building `exgui`
1. Go to the `exgui` folder
2. Open `samp.dpr` to open Delphi with the project configuration**
3. Go to the `Project` menu item on top and select `Build samp`.
4. When compilation succeeded, then go back to your `exgui` folder and you'll find your `samp.exe` there.

** When launching Borland Delphi 7, it may complain about your Debugger. Simply click to 'No' to continue.

## Firing up

### Client

To run the mod, you'll need builds from `saco`, `font`, `archive`, `launch3`, `exgui`

Place your...

1. `samp.exe` from `./exgui`
    - Optional if you use basic launcher from `launch3`.
2. `launch3.exe` from `./launch3/Release/launch3.exe`
    - No need to rename it to `samp_debug.exe`.
3. `saco.dll` from `./saco/Release/saco.dll`
    - You have rename `saco.dll` to `samp.dll`.
4. `gtaweap3.ttf` from `./font`
5. `samp.saa` from `./archive/build`

... and place the files to your GTA: San Andreas installation directory. Now you should be able to launch the mod using the server browser or basic launcher.

**Warning: Do not use any files from the official SA-MP version, like `samp.saa`. It will crash the game.**

### Server

To run the server (`server.exe`) from `./server/Release/` is much simpler, since it can run in any directory. Create a text file next to your server, and rename it to `server.cfg`. Open it with your favourite text editor, add `rcon_password (your password)` and `gamemode0 (gamemode_name_here) 1` lines then save it. Afterwards, create a `gamemodes` folder next to your server, and place your game mode in the folder. Just make sure you have an `.amx` file in your `gamemodes` folder and the game mode name is set to your `gamemode0 (gamemode_name_here) 1` without `.amx` extension.

**Note: Server may throw "File or funtion not found." run time error, if the gamemode contain functions not implemented functions.**
**Note 2: Server may crash if you are using plugins that modifies server's memory (like YSF).**

## Reporting an issue
If you find any issues, please open an issue here, or report the problem in the #bug-report text channel at the repository's Discord server.
Reporting crash logs and bugs from the official SA-MP version also a welcome.
