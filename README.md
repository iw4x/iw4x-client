![license](https://img.shields.io/github/license/IW4x/iw4x-client.svg)
![forks](https://img.shields.io/github/forks/IW4x/iw4x-client.svg)
![stars](https://img.shields.io/github/stars/IW4x/iw4x-client.svg)
![issues](https://img.shields.io/github/issues/IW4x/iw4x-client.svg)
[![build](https://github.com/XLabsProject/iw4x-client/workflows/Build/badge.svg)](https://github.com/XLabsProject/iw4x-client/actions)
[![build status](https://ci.appveyor.com/api/projects/status/rvljq0ooxen0oexm/branch/develop?svg=true)](https://ci.appveyor.com/project/iw4x/iw4x-client/branch/develop)
[![discord](https://img.shields.io/endpoint?url=https://momo5502.com/iw4x/members-badge.php)](https://discord.gg/sKeVmR3)
[![patreon](https://img.shields.io/badge/patreon-support-blue.svg?logo=patreon)](https://www.patreon.com/xlabsproject)

# IW4x: Client

## How to compile

- Run `premake5 vs2022` or use the delivered `generate.bat`.
- Build via solution file in `build\iw4x.sln`. (You can use the `build.bat` script to do it quick and easy.)

## Premake arguments

| Argument                    | Description                                    |
|:----------------------------|:-----------------------------------------------|
| `--copy-to=PATH`            | Optional, copy the DLL to a custom folder after build, define the path here if wanted. |
| `--copy-pdb`                | Copy debug information for binaries as well to the path given via --copy-to. |
| `--ac-disable`              | Disable anticheat.                             |
| `--ac-debug-detections`     | Log anticheat detections.                      |
| `--ac-debug-load-library`   | Log libraries that get loaded.                 |
| `--force-unit-tests`        | Always compile unit tests.                     |
| `--force-exception-handler` | Install custom unhandled exception handler even for Debug builds. |
| `--force-minidump-upload`   | Upload minidumps even for Debug builds.        |
| `--disable-bitmessage`      | Disable use of BitMessage completely.          |
| `--disable-base128`         | Disable base128 encoding for minidumps.        |
| `--no-new-structure`        | Do not use new virtual path structure (separating headers and source files). |
| `--iw4x-zones`              | Zonebuilder generates iw4x zones that cannot be loaded without IW4x specific patches. |

## Command line arguments

| Argument                    | Description                                    |
|:----------------------------|:-----------------------------------------------|
| `-tests`                    | Perform unit tests.                            |
| `-entries`                  | Print ff info after being loaded.              |
| `-stdout`                   | Redirect stdout to the console.                |
| `-console`                  | Create game's external console.                |
| `-dedicated`                | Enabled dedicated server.                      |
| `-scriptablehttp`           | Enable HTTP gsc functions.                     |
| `-bigdumps`                 | Include all code sections from loaded modules. |
| `-bigminidumps`             | Include all code sections from loaded modules. |
| `-reallybigminidumps`       | Include data sections from all loaded modules. |
| `-dump`                     | Write info of loaded assets to the raw folder. |
| `-monitor`                  | Indicated whenever the game console is active. |
| `-nointro`                  | Skip game's cinematic intro.                   |
| `-version`                  | Print IW4x build info                          |
| `-zonebuilder`              | Enable zone builder.                           |
| `-nosteam`                  | Same effect as enabling cl_anonymous dvar.     |


## Disclaimer

This software has been created purely for the purposes of
academic research. It is not intended to be used to attack
other systems. Project maintainers are not responsible or
liable for misuse of the software. Use responsibly.
