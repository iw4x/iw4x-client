# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog v0.3.0](http://keepachangelog.com/en/0.3.0/) and this project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]

### Known issues

- IW4x on Linux currently requires gnutls to be installed to access the Tor service via HTTPS.

## [0.2.2] - 2016-12-25

This is the first public Beta version, it mostly consists of bug fixes.

### Added

- Add peacekeeper

### Changed

- Optimize fastfiles, they are now a lot smaller.
- Improve security by modifying guids to allow 64 bit certificate fingerprints.

### Fixed

- Fix issues when spawning more than one bot.
- Fix no ammo bug.
- Fix splash screen hang.
- Fix concurrent image loading bug.
- Fix server crash on startup.

### Known issues

- IW4x on Linux currently requires gnutls to be installed to access the Tor service via HTTPS.

## [0.2.1] - 2016-12-14

This is the second public Alpha version.

### Added

- Add support for CoD:Online maps.

    - Firing Range (mp_firingrange)
    - Rust (mp_rust_long)
    - Shipment (mp_shipment/mp_shipment_long)

- Add sv_motd dvar for server owners to set custom motd (will be visible in the loadscreen).
- Add Zonebuilder support for sounds and fx.
- Add command setviewpos.
- Add high-definition loadscreens.

### Changed

- Rename Arctic Wet Work map to it's official name (Freighter).
- Complete redesign of the main menus.
- Allow cl_maxpackets to be set up to 125.

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

- IW5 material embedding.

### Changed

- Enhanced mod download with detailed progress display.

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

- Added `banclient` command which will permanently ban a client from a server. The ban will persist across restarts.
- Added capabilities to save played games and replay them ("Theater").
- Added code for generating and sending minidumps for debugging purposes. This feature is meant to be used only during the Open Beta and will be removed once the code goes to stable release.
- Added commands that allow forwarding console and games log via UDP to other computers ("network logging").
- Added D3D9Ex.
- Added filters for server list.
- Added handling for `iw4x://` URLs ("connect protocol"). For example, if IW4x is properly registered in Windows as URL handler for `iw4x://` URLs you can type `iw4x://ip:port`. If possible, this will connect to the server in an already running IW4x client.
- Added lean support through new key bindings.
- Added native cursor as replacement for the sometimes laggy in-game cursor. This change can be reverted in the settings menu.
- Added news ticker.
- Added remote console ("RCon").
- Added support for BigBrotherBot.
- Added support for hosting game mods in order to allow players to just join modded servers out of the box ("mod download").
- Added Warfare2 text coloring.
- Added zone builder. For more information see the respective documentation.
- Implemented a completely decentralized peering network.
- Implemented playlists which can be used for flexible map and gametype rotation.
- Introduced security levels. This ensures that you need to "pay" with CPU power to verify your identity once before being able to join a server which reduces the interval at which people who get banned can circumvent server bans through using new identities. The default security level is 23.
- IW4x resource files are in their own folder to prevent clogging up the main game directories.
- Reintroduced parties, now also available for dedicated servers ("lobby servers").

### Changed

- Logs moved to `userraw` folder.
- Replaced main menu background music.
