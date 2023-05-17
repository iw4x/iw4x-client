# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog v0.3.0](http://keepachangelog.com/en/0.3.0/).

## r4303 - 2023-05-17

### Added

- Add `dumpZone` ZoneBuilder command (#1033)
- Add `FreezeControlsAllowLook` GSC method (#1026)

### Fixed

- The sound fix was removed as the bug no longer happens (#1007)
- Fix crash when the opening the "server info" menu on an empty server list (#1041)
- Fix patch that was causing the game thread to deadlock (#1038)

## r4251 - 2023-05-03

### Added

- Add GSC client field `ping` as a read-only field (#1002)
- Add GSC client field `address` as a read-only field (#1003)
- Add to the iw4x-rawfiles `common_scripts\utility` GSC script `getIP` function.

### Fixed

- `getPing` function in `common_scripts\utility` now works.

### Known issues

- Sound issue fix is experimental as the bug is not fully understood.

## r4246 - 2023-05-03

### Added

- Add to the iw4x-rawfiles `common_scripts\utility` GSC script `setPing` & `getPing` functions for backward compatibility.

### Fixed

- Fix bug with Steam Proxy (#991)
- Fix bug with the `say` GSC notify in regards to hidden chat messages (#989)

### Removed

- SetPing GSC method (#998)

### Known issues

- Sound issue fix is experimental as the bug is not fully understood.

## r4226 - 2023-04-26

### Changed

- Chat system will go back to using `SV_CMD_CAN_IGNORE` commands (#972)

### Security

- Check the address of the sender for the `print` OOB packet (#969)
- Check the address of the sender for the `voice` OOB packet (#973)

### Fixed

- Fix bug with how `sv_mapRotationCurrent` is parsed (#977)

### Known issues

- Sound issue fix is experimental as the bug is not fully understood.

## r4208 - 2023-04-22

### Changed

- `Noclip` GSC method does not require `sv_cheats` to be set to "1" for it to work (#962)
- `Ufo` GSC method does not require `sv_cheats` to be set to "1" for it to work (#962)

### Fixed

- Fix `InfoString` output (#961)
- Fix parsing of the server info (client-side) (#953)
- Fix bug in the /info TCP endpoint (#955)

### Known issues

- Sound issue fix is experimental as the bug is not fully understood.

## r4193 - 2023-04-19

### Added

- Add `LogString` GSC function (#895)
- Add `LogString` GSC method (#895)
- Add `sv_replaceTestClients` Dvar (#930)

### Changed

- `sv_mapRotationCurrent` supports `exec` directive for executing cfg scripts from the `game_settings` folder (#916)
- `SetPing` GSC method is now deprecated (#903)

### Security

- Fix DOS exploit in the network protocol of the game (#942)

### Fixed

- `sv_privatePassword` will work as intended (#908)
- Fix crash when loading bots.txt file (#927)

### Removed

- Remove `GetIp` GSC method (#903)
- Remove `GetPing` GSC method (#903)

### Known issues

- Sound issue fix is experimental as the bug is not fully understood.

## [0.7.9] - 2023-03-31

### Added

- Game scripts can be loaded from the `scripts/mp`, `scripts/mp/<map name>` and `scripts/mp/<game type>` folders (#859)
- Add `ReadStream` GSC function (#862)
- Add `IString` GSC function (#877)

### Changed

- Test Clients will no longer receive names from the Xlabs Patreon website. The old behaviour was restored (#852)
- Enabled `OpenFile` GSC function (#862)
- Enabled `CloseFile` GSC function (#862)
- Chat system will now use `SV_CMD_RELIABLE` commands to mitigate message duplication (#873)
- The built-in GSC compiler no longer throws fatal errors when overriding a built-in function or method (IW3 behaviour) (#880)
- `CastFloat` GSC function was renamed to `Float` (#880)

### Fixed

- Fix bug where knife lounges would not work with a gamepad (#848)
- Fix rare RCon crash (#861)
- Fix bug where `sv_RandomBotNames` stopped working (#876)
- Fix crash caused by "Mongoose" (dependency) (#874)

### Known issues

- Sound issue fix is experimental as the bug is not fully understood.

## [0.7.8] - 2023-03-17

### Added

- Clients can unprotect "saved" Dvars using the command line argument `-unprotect-dvars` (#694)
- Clients can protect all Dvars from being modified by the server using the command line argument `-protect-dvars` (#823)
- Add helpful information to script-related errors. Must set `developer` Dvar to "2" and `developer_script` Dvar to "1" (#721)
- Add command line `-disable-mongoose` argument to disable IW4x's built-in webserver (#728)
- Add command line `-disable-rate-limit-check` argument to disable the rate check on RCon requests (#769)
- GUID of muted clients is saved to the `muted-users.json` file in the `userraw` folder (#732)
- Add `sv_nextMap` Dvar (#736)
- Add `elifdef` and `elifndef` directives to the menu preprocessor (#747)
- Add `r_drawRunners` Dvar (#758)
- Add `r_drawLights` Dvar (#814)
- Add Discord Rich Presence (#761)
- Add `rcon_timeout` Dvar (#769)
- Add `sv_clanName` Dvar (#771)
- Add `InitialWeaponRaise` GSC method (#804)
- Add `RconWhitelistAdd` SV Command (#804)
- Add more options to `bg_bouncesAllAngles` Dvar (#806)
- Add `GetChar` GSC function (#813)
- Add `bg_bunnyHopAuto` Dvar (#818)
- Add `bg_disableLandingSlowdown` Dvar (#818)
- Add new map porting utility tool that makes the map porting process between CoD8 to CoD6 easy

### Changed

- Servers can no longer modify client Dvars that are saved in the client's config (#694)
- `banClient` and `muteClient` server commands do not apply to bots anymore (#730)
- The max value of `perk_extendedMeleeRange` Dvar was increased (#782)
- Test Clients will receive names from the Xlabs Patreon website in addition to the names from the bots.txt file (#771)

### Fixed

- Fix bug where`reloadmenus` command would not free resources used by custom menus (#740)
- Fix bug where demo playback would stop when opening a laptop based killstreak (#699)
- Fix bug where mod download speed was inexplicably slow (#707)
- Fix bug where `g_hardcore` could not be set by servers (#708)
- Fix "user cmd angles" for test clients (#707)
- Fix bug where `FileRead` GSC function could return buffers that are too big (#767)
- Fix bug where the `OnPlayerSay` GSC function would cause issues (#776)

### Removed

- Remove `zb_prefer_disk_assets` Dvar (#772)

### Known issues

- Sound issue fix is experimental as the bug is not fully understood.

## [0.7.7] - 2022-12-31

### Added

- Add `r_forceTechnique` Dvar to debug techniques on materials.
- Add `IsSprinting` GSC method (#587)
- Add `StorageLoad` GSC function (#595)
- Add `bg_climbAnything` Dvar (#663)
- Add "ClanTag" support for bots (#645)
- Add `sv_randomBotNames` Dvar (#665)
- Add support for parsing localized strings files (.str & .json) (#621)
- Add `callvote` menus (#613)
- IW3x-port converts Technique Sets between CoD4 and CoD6 properly.
- Add new map porting utility tool that makes the map porting process between CoD4 to CoD6 easy.

### Changed

- `r_drawModelNames` now uses ground lighting when available.
- Speculars are now enabled on custom maps.
- Techset(s) get loaded from disk first rather than memory.
- Use CSO format for Vertex Shaders & Pixel Shaders on ZoneBuilder to allow replacement/swapping.
- New map porting utility tool builds teams directly into the map.

### Fixed

- Fix bug where ZoneBuilder would not build loaded sounds correctly.
- Fix bug where ZoneBuilder no longer differentiates assets depending on their name.
- Fix building FX with ZoneBuilder.
- Fix branding in ZoneBuilder generated zones.
- Fix Script String crash when building zones.
- Fix the changelog menu (#583)
- Fix bug when adding commands (#609)
- Fix bug where some ported maps would either crash or lag (mp_zavod, mp_kowloon, ...)
- Fix GSC conversion from CoD4 to CoD6 (Specular Scale, Create Exp Fog, Create FX, ...)
- The map porting process from CoD4 to IW4x has improved.
- Ported map zones are about 40% lighter than before.
- Static models are now lit correctly depending on their position and ground lighting.
- New map porting utility ports sounds and effects properly.

### Removed

- Remove `HttpGet`& `HttpCancel` (#603)

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.

## [0.7.6] - 2022-11-22

### Added

- Add `GetStat` GSC method (#485)
- Add `SetStat` GSC method (#485)
- Add `statGet` command (#485)
- Add `FileRemove` GSC function (#509)
- Add `cg_drawDisconnect` Dvar (#551)
- Add loading info strings from raw file (#557)
- Add localized file string parsing to Zone Builder (.str) (#569)
- Add single localized file string parsing to Zone Builder (#569)

### Changed

- Change font and colour of the external console (#477)

### Fixed

- Fix crash for debug build (#476)
- Fix auth bug with the connect protocol `iw4x://` (#478)
- Fix party server not receiving packets (#500)
- Fix crash caused by server thread issues (#561)
- Fix hours spent playing IW4x not counting towards the hours spent playing MW2 on Steam (#573)
- Remove non-existent menus from code (#483)

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.

## [0.7.5] - 2022-09-03

### Added

- Add `bg_rocketJumpScale` Dvar (#413)
- Add `CastFloat` GSC function (#414)
- Add `Strtol` GSC function (#414)
- Add `bg_lean` Dvar (#421
- Add voice chat (#425)
- Add `vote` & `callvote` client commands (#447)
- Add `kill` client command (#451)
- Add `voteKick`, `voteTempBan`, `voteTypeMap`, `voteMap` and `voteGame` UI script tokens (#456)
- Add `Int64IsInt`, `Int64ToInt` and `Int64OP` GSC functions (#419)

### Changed

- Steam status is no longer set to busy (#417)
- `g_allowVote` is a replicated Dvar (#457)

### Security

- Disable `HttpGet`& `HttpCancel` (#449)

### Fixed

- Fixed `startSingleplayer` command (#404)
- General stability update

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.

## [0.7.4] - 2022-07-28

### Added

- Add `DisableWeaponPickup` GSC method (#329)
- Add `EnableWeaponPickup` GSC method (#329)
- Add `protect-saved-dvars` command line argument (#335)
- Add `clanName` dvar. Can be edited in the "barracks" menu (#361)
- Add DLC9 containing classic maps from CoD4: Backlot, Chinatown, Winter Crash, Pipeline and Downpour.
- Add to the iw4x-rawfiles `common_scripts\iw4x_utility` GSC script, it contains the scripts-based solution for the removed GSC built-in methods.

### Changed

- `sv_mapRotationCurrent` functionality has been restored for backward compatibility (#328)
- GSC IO Functions are restricted to the `scriptdata` folder (#322)
- `scriptablehttp` command line argument is no longer needed (#322)

### Fixed

- Node system works again for clients (#331)
- Fixed output of `g_log_list` & `log_list` (#336)
- Fixed toast notifications (#337)
- Fixed `banClient` command (#311)
- Fixed singleplayer maps crashing in renderer module (#340)
- Fixed muzzle flash on COD:OL maps (#352)
- Server commands are no longer being registered twice (#339)
- Fixed issue where grenades and projectiles had no bounce sounds (#368)
- Fixed issue that could cause the game to crash when loading CoD4 maps (#372)

### Removed

- Remove `noclip` built-in GSC method. Replaced with script-based solution (#387)
- Remove `ufo` built-in GSC method. Replaced with script-based solution (#387)
- Remove `god` built-in GSC method. Replaced with script-based  solution (#388)
- Remove `demiGod` built-in GSC method. Replaced with script-based  solution (#388)
- Remove `noTarget` built-in GSC method. Replaced with script-based  solution (#388)

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.

## [0.7.3] - 2022-06-23

### Added

- Add `SetName` GSC method (#288)
- Add `ResetName` GSC method (#288)
- Add `OnPlayerSay` GSC function (#265)
- Add `Give` client command (works with weapons only) (#292)
- Add `sv_disableChat` Dvar (#290)
- Add `addMap` command (#302)
- Add `addGametype` command (#302)

### Changed

- `sv_mapRotationCurrent` is not being used anymore (#302)
- `sv_mapRotation` is parsed once on startup (#283)

### Security

- Remove `openLink` command (#286)

### Fixed

- Fix font generation (#277)
- Fix crash on clearing key binds (#278)
- Fix maps dropping out of the map rotation when using `sv_randomMapRotation` (#283)

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.
- Toast notifications have been disabled because they cause a crash that needs to be investigated

## [0.7.2] - 2022-05-10

### Added

- Add `IsArray` GSC function (#248)
- Keybind fields in menus work with controller keys (#255)

### Changed

- GSC function `GetSystemTime` now returns the Unix time (#258)

### Fixed

- Knife charge is fixed for controller players (#259)
- Fixed internet, local and favorites filters (#260)
- `sv_lanOnly` Dvar now prevents the server from sending heartbeats to master if set to true (#246)

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.

## [0.7.1] - 2022-05-03

### Added

- Add `ToUpper` GSC Function (#216)
- Add `StrICmp` GSC Function (#216)
- Add `IsEndStr` GSC Function (#216)
- Add `DropAllBots` GSC Function (#174)
- Add GSC entity field `entityflags` (#228)
- Add GSC client field `clientflags` (#228)
- Add `bg_surfacePenetration` Dvar (#241)
- Add `bg_bulletRange Dvar` (#241)

### Changed

- Test clients' native functionality has been restored by default (#162)
- Renamed GSC method `isBot` to `IsTestClient` (#162)
- Custom GSC functions can be called correctly from a game script (#216)
- GSC functions `HttpCancel` and `HttpCancel` require the game to be launched with the command line argument `scriptablehttp` (#162)
- Master server list will be used instead of the node system (load server list faster) (#234)

### Fixed

- Fixed issue with mouse acceleration when polling rate is greater than 125Hz (#230)
- Fixed issue with player speed caused by sprinting from the prone position (#232)
- Fixed client crash when `cg_chatHeight` was set to 0 (#237)
- Fixed GSC function `Scr_TableLookupIStringByRow` (#162)

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.

## [0.7.0] - 2022-05-01

### Added

- Add controller support (#75)
- Add aim assist for controllers (#75)
- Unlock `camera_thirdPersonCrosshairOffset` Dvar (#68)
- Add support for building custom Fonts with Zonebuilder (#88)
- Add colorblind friendly team colors (#101)
- Add emojis based on title cards and emblems to use in the chat and server names Example: `:nuke:` (#130)
- Upon leaving a server 'archive' dvars (saved in the config file) will be reset to the value they had prior to joining the server (#134)
- Add `muteClient` command for the game chat (#159)
- Add `unmute` command for the game chat (#159)
- Add `sv_allowAimAssist` Dvar (#75)
- Add `sv_allowColoredNames` (#130)
- Add `sv_randomMapRotation` Dvar (#146)
- Add `rcon_log_requests` Dvar (#195)
- Add `player_duckedSpeedScale` Dvar (#141)
- Add `player_proneSpeedScale` Dvar (#141)
- Add `cg_ufo_scaler` Dvar (#158)
- Add `cg_noclip_scaler` Dvar (#158)
- Add `bg_bouncesAllAngles` Dvar (#158)
- Add `bg_rocketJump` Dvar (#158)
- Add `bg_elevators` Dvar (#156)
- Add `noclip` client command (#152)
- Add `ufo` client command (#152)
- Add `God` client command (#152)
- Add `demigod` client command (#152)
- Add `notarget` client command (#152)
- Add `noclip` GSC Function (#152)
- Add `ufo` GSC Function (#152)
- Add `God` GSC Function (#152)
- Add `demigod` GSC Function (#152)
- Add `notarget` GSC Function (#152)
- Add `replaceFunc` GSC Function (#144)

### Changed

- Renamed `sv_enableBounces` to bg_bounces (#158)
- Renamed `g_playerCollision` to bg_playerEjection (#158)
- Renamed `g_playerEjection` to bg_playerCollision (#158)
- `setviewpos` client command works outside private matches (#163)
- `ufo` client command works outside of private matches (#152)
- `noclip` client command works outside of private matches (#152)
- If a player name is less than 3 characters server will change it to `Unknown Soldier` (#130)
- `scr_player_forceautoassign` Dvar is false by default

### Fixed

- Fixed issue where CoD:O DLC Maps caused DirectX crash following `vid_restart` (#37)
- Fixes and improvements to Zonebuilder
- Fixed issue where the game froze following base game script throwing an error (#74)
- Fixed RCon on party servers (#91 - #95)
- Fixed slow motion during final kill cams (#111 - #107)
- Fixed sound issue that causes the game to freeze (#106)
- Fixed issue where materials strings found in hostnames, player names, chat etc. caused the game to crash (#113)
- Fixed issue with servers displaying an invalid player count (#113)

### Known issues

- HTTPS is not supported for fast downloads at the moment.
- Sound issue fix is experimental as the bug is not fully understood.
- `reloadmenus` command does not free resources used by custom menus.

## [0.6.1] - 2020-12-23

### Added

- Add host information to `/info` endpoint (request)
- Add `fileWrite` GSC Function (#36)
- Add `fileRead` GSC Function (#36)
- Add `fileExists` GSC Function (#36)
- Add `fileRemove` GSC Function (#36)
- Add `botMovement` GSC Function (#46)
- Add `botAction` GSC Function (#46)
- Add `botWeapon` GSC Function (#46)
- Add `botStop` GSC Function (#46)
- Add `isBot` GSC Function (#46)
- Add `setPing` GSC Function (#46)
- Add `GetSystemTime` and `GetSystemTimeMilliseconds` GSC Functions (#46)
- Add `PrintConsole` GSC Function (#46)
- Add `Exec` GSC Function (#46)
- Add `getIP` GSC Method (#36)
- Add `getPing` GSC Method (#36)
- Add `scr_intermissionTime` Dvar (#25)
- Add `g_playerCollision` Dvar (#36)
- Add `g_playerEjection` Dvar (#36)
- Add `r_specularCustomMaps` Dvar (#36)
- Unlock `safeArea_horizontal` and `safeArea_vertical` Dvars (#42)
- Unlock `cg_fovscale` Dvar (#47)
- Add `g_antilag` Dvar (#61)

### Changed

- Stats are now separate for each mod (#6). Player stats are copied to `fs_game` folder if no stats exist for this mod yet. Keep in mind this also means that level, XP and classes will not be synchronized with the main stats file after this point.
- Reduced duration of toasts (#48)
- Use old bot names if bots.txt is not found (#46)

### Fixed

- Fixed a node system related crash (#45)
- Fixed an issue that made dedicated servers crash when info was requested during map rotation (#43)
- Fixed an issue where the game was trying to decrypt GSC files which caused it to crash when loading mods (#35)
- Fixed an issue causing the game to crash when Steam was running in the background (#56)
- Fixed slow download speed when using fast download

### Removed

- Removed old updater functionality (#54)

### Known issues

- HTTPS is not supported for fast downloads at the moment.

## [0.6.0] - 2018-12-30

### Added

- Add `unbanClient` command.
- Add `/serverlist` api endpoint on dedicated servers
- Add dvar to control the server query rate (net_serverQueryLimit & net_serverFrames)

### Changed

- Update dependencies
 
### Fixed

- Fix mods not working in private matches.
- Fix multiple game structures (map tools)
- Fix multiple vulnerability's
- Fix lag spikes on lower end PCs
- Fix invalid name check
- Fix `openLink` command crash issue
- Fix lobby server map downloading
- Fix steam integration

### Known issues

- HTTPS is not supported for fast downloads at the moment.

## [0.5.4] - 2017-07-09

### Added

- Integrate IW4MVM by luckyy.

### Changed

- Displayed stats for Dragunov have been changed, has no effect on actual game play.

### Fixed

- Fix fast download failing when the target host is missing a trailing slash.
- Fix servers not being listed in the server browser.
- Fix some FPS drop issues caused by compression code.

### Known issues

- HTTPS is not supported for fast downloads at the moment.

## [0.5.3] - 2017-07-02

### Added

- Increase webserver security.

### Fixed

- Reduce lags introduced by nodes.
- Fix modlist download.

## [0.5.2] - 2017-07-02

### Fixed

- Fix dedicated server crash.

## [0.5.1] - 2017-07-02

### Added

- Add fast download option for custom mods/maps based on Call of Duty 4.
- Display a toast when an update is available.
- Use the hourglass cursor while loading assets (with the native cursor feature).
- Show bots in parenthesis after the number of players in the serverlist (request).
- Add GSC event `level waittill("say", string, player);` (request).
- Restrict unauthorized mod download for password protected servers.
- Add OMA support for 15 classes.

### Changed

- Show friend avatars when they play IW4x (request).
- Rewrite and optimize the entire node system.

### Fixed

- Fix lags and frame drops caused by server sorting.
- Fix demos on custom maps.
- Can no longer join a lobby or server with an incorrect password.
- Fix crashes caused by a bug in file/data compression.
- Improve overall stability.

### Removed

- Remove syncnode command for node system.
- Remove steam start.

## [0.5.0] - 2017-06-04

### Added

- Add GSC functionality to download files via HTTP(S) (request).
- Implement preliminary custom map support.

### Changed

- Add new nicknames for bots.
- Bumped Fastfile version. If you built fastfiles with the zone builder (e.g. mod.ff) you have to rebuild them!
- Default `sv_network_fps` to `1000` on dedicated game servers.
- Increase maximum FOV to 120.

### Fixed

- Fix `iw4x_onelog` dvar.
- Fix server list only showing local servers by default instead of Internet servers.
- Fix some deadlock situations on game shutdown.

## [0.4.2] - 2017-03-16

### Changed

- Disable unnecessary dvar update in server browser.
- Update bot names.

### Fixed

- Fix process permissions.
- Fix classic AK-47 color bleedthrough.
- Re-aligned the MG4's Raffica iron sights.

## [0.4.1] - 2017-03-10

### Fixed

- Fix command line argument parsing.

## [0.4.0] - 2017-03-10

### Added

- Set played with status.
- Add support for 15 classes.
- Add `iw4x_onelog` dvar.
- Add show server/playercount in server browser.

### Changed

- Do not show friend status notifications with stream friendly UI.
- Reduce loaded modules for dedis.
- Use joystick emblem for friend status.
- Disable XP bar when max level.
- Change fs_game display postition.
- Replace Painkiller with Juiced from IW5.

### Fixed

- Fix AK weaponfiles.
- Fix brightness slider.
- Fix text length for column mod in server browser.
- Changed the L86 LSW to use the correct HUD icon.
- Re-aligned the M93 Raffica's iron sights.
- Re-aligned the Desert Eagle's iron sights.

## [0.3.3] - 2017-02-14

### Added

- Add mapname to friend status (request).
- Add option to toggle notify friend state.
- Add support for mod.ff.

### Changed

- Disabled big minidump message box.
- Limit dedicated servers to 15 instances per IP.
- Move build number location.

### Fixed

- Fix audio bug in options menu.
- Fix crash caused by faulty file download requests to game hosts.
- Fix friend sorting.
- Fix game not starting issue under certain circumstances.
- Fix menu crash.
- Fix typo in security increase popmenu.
- Fix vid_restart crash with connect protocol.
- Fix weapon crash issue.
- Potentially fix no-ammo bug.

### Removed

- Remove news ticker and friends button from theater.

## [0.3.2] - 2017-02-12

This is the third public Beta version.

### Added

- Add working friend system.
- Add colored pings in the server list.
- Add credits.
- Add first launch menu.
- Add AK-47 (Classic) attachments.
- Add HUD icon for night vision goggles.

### Changed

- Join when pressing enter in server list (request).
- Redesign and refactor all fullscreen menus.
- Increase weapon and configstring limit.
- Allow creating full crash dumps if wanted.
- Set default name from steam.

### Fixed

- Fix missing models on village.
- Fix custom server motd (request).
- Fix various memory leaks.
- Fix mouse pitch (request).
- Fix compatibility with B3 (request).
- Fix RCon bug (request).
- Fix dedicated server crash on linux.
- Fix crash in mod download.
- Fix peacekeeper reload sound.
- Fix cl_maxpackets 125 in settings (request).
- Fix deserteaglegold_mp icon.

### Known issues

- IW4x on Linux currently requires gnutls to be installed to access the Tor service via HTTPS.

## [0.3.1] - 2017-01-21

This is the second public Beta version.

### Added

- Add classic AK-47 to CAC.
- Add servers to favorites when ingame.
- Add delete favorites button in the serverlist.

### Changed

- Change maplist to a dynamic list.

### Fixed

- Fix list focus.
- Fix mod restart loop.
- Fix mod download status.
- Fix modelsurf crash.
- Fix floating AK-74u.

### Known issues

- IW4x on Linux currently requires gnutls to be installed to access the Tor service via HTTPS.

## [0.3.0] - 2017-01-14

This is the first public Beta version.

### Added

- Add com_logFilter dvar.
- Add peacekeeper.
- Add support for maps from DLC 8 (Recycled Pack)

  - Chemical Plant (mp_storm_spring)
  - Crash: Tropical (mp_crash_tropical)
  - Estate: Tropical (mp_estate_tropical)
  - Favela: Tropical (mp_fav_tropical)
  - Forgotten City (mp_bloc_sh)

### Changed

- Improve node synchronization handling.
- Improve security by modifying GUIDs to allow 64-bit certificate fingerprints.
- Optimize fastfiles, they are now a lot smaller.
- Replace intro.

### Fixed

- Fix concurrent image loading bug.
- Fix issues when spawning more than one bot.
- Fix no ammo bug.
- Fix server crash on startup.
- Fix splash screen hang.

### Known issues

- IW4x on Linux currently requires gnutls to be installed to access the Tor service via HTTPS.

## [0.2.1] - 2016-12-14

This is the second public Alpha version.

### Added

- Add support for CoD:Online maps.

    - Firing Range (mp_firingrange)
    - Rust (mp_rust_long)
    - Shipment (mp_shipment/mp_shipment_long)

- Add `sv_motd` Dvar for server owners to set custom motd (will be visible in the loadscreen).
- Add Zonebuilder support for sounds and fx.
- Add command setviewpos.
- Add high-definition loadscreens.

### Changed

- Rename Arctic Wet Work map to it's official name (Freighter).
- Complete redesign of the main menus.
- Allow `cl_maxpackets` to be set up to 125.

### Fixed

- Fix crash when using the Harrier killstreak.
- Disable code that downloads news/changelog when in zonebuilder mode.
- Fix freeze on game shutdown.
- Disable unlockstats while ingame to prevent a crash.

### Known issues

- IW4x on Linux currently requires gnutls to be installed to access the Tor service via HTTPS.

## [0.2.0] - 2016-10-09

This is the first public Alpha version.

### Added

- Support for CoD:Online maps.

    - Arctic Wet Work (mp_cargoship_sh)
    - Bloc (mp_bloc)
    - Bog (mp_bog_sh)
    - Crossfire (mp_cross_fire)
    - Killhouse (mp_killhouse)
    - Nuketown (mp_nuked)
    - Wet Work (mp_cargoship)

### Fixed

- Fix techniques in Zonebuilder.
- Fix possible memory leak.
- Fix timeout bug when connecting to server via iw4x link.
- Partially fix deadlock in decentralized networking code.

### Known issues

- Running IW4x on Linux currently requires gnutls to be installed additional to Wine as it needs to access the Tor service via HTTPS.

## [0.1.1] - 2016-09-19

This version is an internal Pre-Alpha version.

### Added

- Add IW5 material embedding system.

### Changed

- Enhance mod download with detailed progress display.

### Fixed

- Fix and optimize network logging commands.
- Fix console not displaying command inputs properly.
- Fix crash when running multiple instances of the IW4x client from the same directory.
- Fix crash when the `securityLevel` command is used on a game server.
- Fix possible data corruption in code for decentralized networking.
- Fix possible deadlock during client shutdown.

## [0.1.0] - 2016-09-03

This version is an internal Pre-Alpha version.

### Added

- Add `banclient` command which will permanently ban a client from a server. The ban will persist across restarts.
- Add capabilities to save played games and replay them ("Theater").
- Add code for generating and sending minidumps for debugging purposes. This feature is meant to be used only during the Open Beta and will be removed once the code goes to stable release.
- Add commands that allow forwarding console and games log via UDP to other computers ("network logging").
- Add D3D9Ex.
- Add filters for server list.
- Add handling for `iw4x://` URLs ("connect protocol"). For example, if IW4x is properly registered in Windows as URL handler for `iw4x://` URLs you can type `iw4x://ip:port`. If possible, this will connect to the server in an already running IW4x client.
- Add lean support through new key bindings.
- Add native cursor as replacement for the sometimes laggy in-game cursor. This change can be reverted in the settings menu.
- Add news ticker.
- Add remote console ("RCon").
- Add support for BigBrotherBot.
- Add support for hosting game mods in order to allow players to just join modded servers out of the box ("mod download").
- Add Warfare2 text coloring.
- Add zone builder. For more information see the respective documentation.
- Implement a completely decentralized peering network.
- Implement playlists which can be used for flexible map and gametype rotation.
- Introduce security levels. This ensures that you need to "pay" with CPU power to verify your identity once before being able to join a server which reduces the interval at which people who get banned can circumvent server bans through using new identities. The default security level is 23.
- Move IW4x resource files into their own folder to prevent clogging up the main game directories.
- Reintroduce parties, now also available for dedicated servers ("lobby servers").

### Changed

- Move logs to `userraw` folder.
- Replace main menu background music.
