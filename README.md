![icon](https://repository-images.githubusercontent.com/430196616/1d564a38-bfb7-4e4c-b73d-fc47b13fb310)

BeatUpServer
============
A lightweight server to enable modded multiplayer in Beat Saber 1.19.0 and newer.

*The official BeatUpServer instance is hosted at `master.battletrains.org`*

Ways to Join (from least difficult to most)
-------------------------------------------

### Beat Saber Server Browser \[[PC](https://github.com/roydejong/BeatSaberServerBrowser#installation)\]
ServerBrowser lets you join existing public lobbies created on any server.

### BeatUpClient | BETA \[[PC](https://github.com/rcelyte/BeatUpRcelyte/releases/tag/0.3.1)\]
The easiest way to host lobbies on any server. Enables the following additional features in BeatUpServer lobbies:
* Downloading of levels not available on BeatSaver
* Per-player difficulty
* Per-player modifiers
* Skipping the countdown
* Skipping the end-of-level podium

### BeatTogether Mod \[[PC](https://github.com/pythonology/BeatTogether#installation)\]
BeatTogether has historically been the most popular mod for enabling modded multiplayer. To access BeatUpServer, add the hostname to your BeatTogether config located at `UserData\BeatTogether.json` in your game directory:
```json
{
  "SelectedServer": "BeatUpServer",
  "Servers": [
    {
      "ServerName": "BeatTogether",
      "HostName": "master.beattogether.systems",
      "Port": 2328,
      "StatusUri": "http://master.beattogether.systems/status"
    },
    {
      "ServerName": "BeatUpServer",
      "HostName": "master.battletrains.org",
      "Port": 2328,
      "StatusUri": "https://status.master.battletrains.org"
    }
  ]
}
```
