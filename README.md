![license](https://img.shields.io/github/license/IW4x/iw4x-client.svg)
![forks](https://img.shields.io/github/forks/IW4x/iw4x-client.svg)
![stars](https://img.shields.io/github/stars/IW4x/iw4x-client.svg)
![issues](https://img.shields.io/github/issues/IW4x/iw4x-client.svg)
[![build status](https://ci.appveyor.com/api/projects/status/rvljq0ooxen0oexm/branch/develop?svg=true)](https://ci.appveyor.com/project/iw4x/iw4x-client/branch/develop)
[![discord](https://img.shields.io/endpoint?url=https://momo5502.com/iw4x/members-badge.php)](https://discord.gg/sKeVmR3)
[![patreon](https://img.shields.io/badge/patreon-support-blue.svg?logo=patreon)](https://www.patreon.com/xlabsproject)

# IW4x: Client

## Commit message style

```
[Module] Imperative summary

- points or text

[ci skip]
```

`[ci skip]` is optional.

## How to compile

- Run `premake5 vs2019` or use the delivered `generate.bat`.
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

## Command line arguments

| Argument                    | Description                                    |
|:----------------------------|:-----------------------------------------------|
| `-tests`                    | Perform unit tests.                            |
| `-entries`                  | Prints fast file info to the console.          |
| `-stdout`                   | Redirect stdout to the external console.       |
| `-console`                  | Enables external console.                      |
| `-dedicated`                | Dedicated server.                              |
| `-scriptablehttp`           | Adds HTTP console commands.                    |
| `-bigdumps`                 | Enables dumps.                                 |
| `-reallybigdumps`           | Unused.                                        |
| `-bigminidumps`             | Mini dumps.                                    |
| `-reallybigminidumps`       | Big mini dumps.                                |
| `-dump`                     | Prints asset info to a .ents file.             |
| `-monitor`                  | Enables monitor.                               |
| `-nointro`                  | Skips game's intro.                            |
| `-version`                  | Prints IW4X version.                           |
| `-zonebuilder`              | Enables zone builder.                          |
| `-nosteam`                  | Disables Steam features.                       |


## Disclaimer

This software has been created purely for the purposes of
academic research. It is not intended to be used to attack
other systems. Project maintainers are not responsible or
liable for misuse of the software. Use responsibly.
