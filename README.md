<img style="height:1.152em" src="https://user-images.githubusercontent.com/25163630/227527749-e3b5934e-b3ed-423f-b88f-4a785699f6af.png" alt="icon"> BeatUpServer
============
*"beat saber gettin violent these days" -some Discord user*

A lightweight server to enable modded multiplayer in Beat Saber 1.19.0 and newer (Cross-version lobbies supported for 1.20.0<->1.29.3).

Ways to Join
------------

### Beat Saber Server Browser \[[PC](https://github.com/roydejong/BeatSaberServerBrowser#installation)\]
ServerBrowser lets hosts publicly list their lobbies for other ServerBrowser users to join, regardless of server.

### BeatUpClient | BETA \[[PC](https://github.com/rcelyte/BeatUpRcelyte/releases/tag/0.5.1)\]
BeatUpClient enables custom servers to be added in-game from the multiplayer menu using the `+` button, or directly in the config located at `UserData\BeatUpClient.json`. Using BeatUpClient will additionally enable the following features in BeatUpServer lobbies:
* Downloading of levels not available on BeatSaver
* Per-player difficulty
* Per-player modifiers
* Skipping the countdown
* Skipping the end-of-level podium


### BeatTogether Mod \[[PC](https://github.com/BeatTogether/BeatTogether#installation)\]
To access a custom server through the BeatTogether mod on PC, add its status URL to your BeatTogether config (located at `UserData\BeatTogether.json` in the game folder). As an example, here's what the file would look like with rcelyte's `master.battletrains.org` instance added:
```json
{
  "SelectedServer": "BeatUpServer",
  "Servers": [
    {
      "ServerName": "BeatTogether",
      "HostName": "master.beattogether.systems",
      "ApiUrl": "http://master.beattogether.systems:8989",
      "StatusUri": "http://master.beattogether.systems/status",
      "MaxPartySize": 100
    },
    {
      "ServerName": "BeatUpServer",
      "ApiUrl": "https://status.master.battletrains.org",
      "StatusUri": "https://status.master.battletrains.org",
      "MaxPartySize": 126
    }
  ]
}
```

Building BeatUpServer
---------------------
Compiling BeatUpServer requires a Linux/WSL2 host with `git`, `make`, and an updated version of GCC or Clang (at least GCC 12 or Clang 11).
To build, simply run `make` in the repository folder.
> Note: Some Linux distros like Debian ship C compilers **too old** for BeatUpServer. To specify a different compiler for building, set `CC` environment variable:
> ```
> sudo apt install clang-11
> CC=clang-11 make
> ```

Additionally, BeatUpServer may be cross-compiled for Windows using the mingw-w64 toolchain:
```
CC=x86_64-w64-mingw32-gcc make beatupserver.exe
```