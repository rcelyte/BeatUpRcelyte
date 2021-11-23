# **Disclaimer: this project is still in early development and far from functional.**

![icon](https://repository-images.githubusercontent.com/430196616/1d564a38-bfb7-4e4c-b73d-fc47b13fb310)

BeatUpRcelyte
=============
A multiplayer private server for *you!* Planed support for crossplay between PC and Quest.

*Not to be confused with the in-game "Create Server" button, which creates a **room** on a server.*

Joining
-------
There are a few ways to access private servers in game:

### Beat Saber Server Browser
ServerBrowser, available for [PC](https://github.com/roydejong/BeatSaberServerBrowser#beat-saber-server-browser-pc) and [Quest](https://github.com/EnderdracheLP/BeatSaberServerBrowserQuest#beat-saber-server-browser-quest), lets you join existing public rooms created on any server.

### BeatTogether (recommended)
Install BeatTogether for [PC](https://github.com/pythonology/BeatTogether#installation) or [Quest](https://github.com/pythonology/BeatTogether.Quest#installation), then open and close the game at least once.

Add your server's hostname and status uri to `UserData/BeatTogether.json` in your Beat Saber game directory. The file should look something like this:
```json
{
    "SelectedServer": "BeatTogether",
    "Servers": [
        {
            "ServerName": "BeatTogether",
            "HostName": "master.beattogether.systems",
            "Port": 2328,
            "StatusUri": "http://master.beattogether.systems/status"
        },
        {
            "ServerName": "My Private Server",
            "HostName": "my.server.com",
            "Port": 2328,
            "StatusUri": "http://my.server.com"
        }
    ]
}
```
You should now be able to select your server in the multiplayer menu.

### Vanilla
*This method overrides the official servers. Use [BeatTogether](#beattogether-recommended) if you want the ability to switch between servers.*

[*Server side configuration is required for this to work, see notes below*](#hosting)



That's right, you can use custom servers *without* mods!

Open `settings.cfg`, located under `%AppData%\..\LocalLow\Hyperbolic Magnetism\Beat Saber` on Windows, or `~/.steam/steam/steamapps/compatdata/620980/pfx/drive_c/users/steamuser/AppData/LocalLow/Hyperbolic Magnetism/Beat Saber` on Linux.

Search for "useCustomServerEnvironment".

Set `useCustomServerEnvironment` to `true`, and `customServerHostName` to your server's hostname. The file should look something like this:
```
...ₒₜh":4.0,"useCustomServerEnvironment":false,"customServerHostName":"my.server.com","voₗᵤ...
```

Hosting
-------
Looking to host your own private server?

Download and run the latest build for Windows (*no releases yet*) or Linux (*no releases yet*).

*Note: Vanilla Beat Saber requires the status uri to be the `status` subdomain of the hostname (i.e. `status.my.server.com`), hosted over HTTPS with a **valid SSL certificate***
