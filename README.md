<img style="height:1.152em" src="https://user-images.githubusercontent.com/25163630/227527749-e3b5934e-b3ed-423f-b88f-4a785699f6af.png" alt="icon"> BeatUpServer
============
*"beat saber gettin violent these days" -some Discord user*

A lightweight server to enable modded multiplayer in Beat Saber 1.19.0 and newer (Cross-version lobbies supported for 1.20.0<->1.28.0).

Ways to Join
------------

### Beat Saber Server Browser \[[PC](https://github.com/roydejong/BeatSaberServerBrowser#installation)\]
ServerBrowser lets hosts publicly list their lobbies for other ServerBrowser users to join, regardless of server.

### BeatUpClient | BETA \[[PC](https://github.com/rcelyte/BeatUpRcelyte/releases/tag/0.3.1)\]
BeatUpClient enables custom servers to be added in-game from the multiplayer menu using the `+` button, or directly in the config located at `UserData\BeatUpClient.json`. Using BeatUpClient will additionally enable the following features in BeatUpServer lobbies:
* Downloading of levels not available on BeatSaver
* Per-player difficulty
* Per-player modifiers
* Skipping the countdown
* Skipping the end-of-level podium


### BeatTogether Mod \[[PC](https://github.com/pythonology/BeatTogether#installation)\]
To access a custom server through the BeatTogether mod on PC, add its hostname, port, and status URL to your BeatTogether config (located at `UserData\BeatTogether.json` in the game folder). As an example, here's what the file would look like with rcelyte's `master.battletrains.org` instance added:
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

-------------------------------------------
## Self Hosting

For hosting your own instance, you can either directly start the compiled file or host it inside of a docker container.

`Hosting directly` is recommended for most people, only use docker if you know what you are doing and your environment is setup for this.

Requirements:
- Domain Name with SSL Cert
- Port 5000 free (Not configurable yet)

### Hosting directly (Recommended)
1. Compile the server application with make
2. Start the application on the target system, a default `beatupserver.json` will be created
3. Edit this config 
   1. Change the address to your public ipv4 or domain name with an A Record (AAAA Records should not be present for this domain, as it will most likely make problems for some players)
   2. In `status` add the Keys `cert` and `key`, and change the value to the path of your ssl cert
   3. in `master` this can be done as well, but currently is not required, because Beat Saber protocol is currently bypassing the check on client-side by MultiplayerCore and BeatUpClient. 
4. Start the server or create a service for autostart. The parameter `--deamon` is required, when running as a service and no input is available.
5. Follow the steps from [Ways to Join](#ways-to-join)

### Hosting with docker (Reverse Proxy expected)
For hosting the server inside of docker, you have to follow the following steps:

1. Compile the server application with make on linux (WSL or Linux Host)
2. Copy the result into the extras/Docker folder
3. Open the bash / powershell inside the extras/Docker folder
4. Run `docker build -f "Dockerfile" -t beatupserver:latest .`
5. If you ran this on the system, where you want to host this, skip to 5.
   1. Run `docker save -o beatupserver.tar beatupserver:latest`
   2. Copy beatupserver.tar onto target system
   3. Run `docker load -i beatupserver.tar`
   4. Copy the docker-compose.yml to the target system
6. Create a `beatupserver.json` in the same location where the `docker-compose.yml` is and change the values to fit your needs
    ```json
    {
      "instance": {
        "address": ["your external ips or domain, ipv4 recommended"],
        "count": 1
      },
      "master": {},
      "status": {
        "url": "http://0.0.0.0"
      }
    }
    ```
7. Run `docker-compose up -d` (Or `docker compose up -d` on newer versions)
8. Create config in your reverse proxy to pass the external https request into docker with port 80. Sample for nginx with certbot cert:
   ```nginx
    server {
        listen 443 ssl;
        listen [::]:443 ssl;
        server_name status.yourdomain.com www.status.yourdomain.com;

        set $upstreamName beatup:80;

        # SSL Config goes here, but the bare minimum is this:
        ssl_certificate /etc/letsencrypt/live/status.yourdomain.com/fullchain.pem;
        ssl_certificate_key /etc/letsencrypt/live/status.yourdomain.com/privkey.pem;

        location / {
            proxy_pass_request_headers   on;
            proxy_pass http://$upstreamName;
        }
    }
   ```
   ***!!! IMPORTANT !!!***

   Do not add any additional headers into the pass logic, because the amount of headers is currently used to determine, if it is a browser accessing the server or the game itself. So settings like this are not supported:
   ````nginx
    proxy_set_header Host $host;
    proxy_set_header X-Real-IP $remote_addr;
    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    proxy_set_header X-Forwarded-Proto https;
   ```
9. Follow the steps from [Ways to Join](#ways-to-join)

#### Troubleshooting
1. The Server is not reachable from Beat Saber
   1. Try to call the domain directly in a browser, you should be able to see the BeatUpServer website. If that isn't the case, check the port forwarding, firewall configs and ip address.
2. Server is reachable, but the icons look regular and I can't set the custom settings like per user difficulty
   1. Most likely happends when hosting inside docker, try to check if your reverse proxy adds headers and if so, try to get rid of the additional headers.
3. I can see the server, but after trying to connect I get a connection error
   1. Check if the `address` inside the `instance` config is reachable from the client
   2. Try to only have an A Record an no AAAA Record
   3. Try to add the ipv4 into `instance` `address` config instead of the domain name
   4. Check if Port 5000, 2328 (If you didn't change this) and 443 is reachable from the client