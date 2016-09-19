# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog v0.3.0](http://keepachangelog.com/en/0.3.0/) and this project adheres to [Semantic Versioning](http://semver.org/).

## v0.1.1 - 2016-09-19

This version is an internal Beta version.

### Added
- IW5 material embedding.

### Changed
- Enhanced mod download with detailed progress display.

### Fixed
- Fixed and optimized network logging commands.
- Fixed console not displaying command inputs properly.
- Fixed crash when running multiple instances of the IW4x client from the same directory.
- Fixed crash when the `securityLevel` command is used on a game server.
- Fixed possible data corruption in code for decentralized networking.
- Fixed possible deadlock during client shutdown.

## v0.1.0 - 2016-09-03

This version is an internal Beta version.

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
