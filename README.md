# Planeta Match Maker

A very simple and light match making system for P2P online game.
Server binary for linux and windows, and client library for C# including Unity are provided.

## Features

- Creating room and joining room
- Searching room by owners name
- Port mapping auto creation by UPnP which allows NAT traversal
- (Not implemented now) Random Matching

## Platforms

### Server

A binary which is executable in below platforms.

- Windows
- Linux

### Client

A library by below languages and platforms.

- C# (.Net Framework or .Net Core which is campatible with .Net Standard 2.1)
- Unity (.Net 4.0)

## Usage

You can easily install and use server and client.

### Server (Use Docker)

You can very easily install server by using docker by following steps.

1. Pull docker image with tag `cdec/plaenta-match-maker:server-alpine`
2. Run a container with the image

Following commands are example to run a server with port 57000 by using docker.

```bash
docker pull cdec/plaenta-match-maker:server-alpine
docker run -p 57000:57000 cdec/plaenta-match-maker:server-alpine
```

You may need to set firewall to acceppt recieve connection of TCP port which is defined in the setting file.

You can change settings by editing [the setting file](Documents/ServerSettings.md) if you need.

### Server (Mannually Install)

In linux and windows, you can install server by manually by following steps.

1. Download a binary from release page
1. Put the binary to any place you like
1. Put the setting file to `/etc/pmms/setting.json`
1. Execute the binary

You may need to set firewall to acceppt recieve connection of TCP port which is defined in the setting file.

You can change settings by editing [the setting file](Documents/ServerSettings.md) if you need.

### C# Client

1. Download source codes from release page or clone this repogitory
1. Put source codes in `PlanetaMatchMakerClient/Source` directory to your project

### Unity Client

1. Download unity package from release page
1. Import the unity package to your project

## Documents

- [Server Settings](Documents/ServerSettings.md)
- [Build Manual](Documents/BuildManual.md)
- [Server Message API Reference](Documents/ServerMessageAPIReference.md)

## License

The codes in this repository are lisenced unfer the [MIT License](https://github.com/CdecPGL/PlanetaMatchMaker/blob/master/LICENSE).

This repogitory includes following libraries from other repogitories.
The licenses of these codes follows each repogitories.

- [namepf C++](https://github.com/Neargye/nameof) ([MIT License](https://github.com/Neargye/nameof/blob/master/LICENSE))
- [minimal-serializer](https://github.com/CdecPGL/minimal-serializer) ([MIT License](https://github.com/CdecPGL/minimal-serializer/blob/master/LICENSE))
- [Open.NAT](https://github.com/lontivero/Open.NAT) ([MIT License](https://github.com/lontivero/Open.NAT/blob/master/LICENSE))
