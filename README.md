[![Release](https://img.shields.io/github/v/release/CdecPGL/PlanetaMatchMaker?include_prereleases&sort=semver)](https://github.com/CdecPGL/PlanetaMatchMaker/releases)
[![License](https://img.shields.io/github/license/CdecPGL/PlanetaMatchMaker)](https://github.com/CdecPGL/PlanetaMatchMaker/blob/master/LICENSE)
[![CircleCI Buld and Test Status](https://circleci.com/gh/CdecPGL/PlanetaMatchMaker/tree/master.svg?style=shield)](https://circleci.com/gh/CdecPGL/PlanetaMatchMaker/tree/master)

# Planeta Match Maker

[README (日本語)](README_jp.md)

A very simple and light match making system for P2P online game.
Server binary for linux and windows, and client library for C# including Unity are provided.

## Features

- Creating room and joining room
- Searching room by owners name
- NAT traversal
  - Builtin Mode: Attemt to establish P2P connection by port mapping auto creation with UPnP
  - Steam Relay Mode: Help to exchange SteamID64 to establish P2P connection for Steam relay service
- (Not implemented now) Random Matching

## Platforms

### Server

A binary which is executable in below platforms.

- Windows
- Linux

For linux, extremely small docker image is also provided in [DockerHub](https://hub.docker.com/repository/docker/cdec/planeta-match-maker-server/general).

### Client

A library by below languages and platforms.

- C# (.Net Framework or .Net Core which is campatible with .Net Standard 2.1)
- Unity (.Net 4.0)

## Usage

You can easily install and use server and client.

### Server

#### Docker

You can very easily install server by using docker by following steps.

1. Pull docker image with tag `cdec/plaenta-match-maker:server-alpine`
2. Run a container with the image

Following commands are example to run a server with port 57000 by using docker.

```bash
docker pull cdec/plaenta-match-maker-server:latest
docker run -p 57000:57000 cdec/planeta-match-maker-server:latest
```

You may need to set firewall to acceppt recieve connection of TCP port which is defined in the setting file.

You can change settings by editing [the setting file](Documents/ServerSettings.md) if you need.

#### Mannual

In linux and windows, you can install server by manually by following steps.

1. Download a binary from release page
1. Put the binary to any place you like
1. Put the setting file to `/etc/pmms/setting.json`
1. Execute the binary

You may need to set firewall to acceppt recieve connection of TCP port which is defined in the setting file.

You can change settings by editing [the setting file](Documents/ServerSettings.md) if you need.

### Client

#### C#

1. Download source codes from release page or clone this repogitory
1. Put source codes in `PlanetaMatchMakerClient/Source` directory to your project

#### Unity (Unity Package)

1. Download unity package from release page
1. Import the unity package to your project

#### Unity (Mannual)

1. Download source codes from release page or clone this repogitory
1. Copy all files in `PlanetaMatchMakerUnityClient/Assets` to `Assets` directory of your unity project

### Steam Integration in Client

1. Install steamworks library for C# in below table
1. Set pre-defined macro of compiler in below table
1. Add `using PlanetaGameLabo.MatchMaker.Extentions;` in your code
1. Use `MatchMakerClient.CreateRoomWithSteamAsync` and `MatchMakerClient.JoinRoomWithSteamAsync`

Note that enabling `Facepunch.Steamworks` and `Steamworks.NET` at same time is not supported.

|Name|Macro|Repository|
|:---|:---|:---|
|Facepunch.Steamworks|PMM_FacepunchSteamworks|[URL](https://github.com/Facepunch/Facepunch.Steamworks)|
|Steamworks.NET|PMM_SteamworksNET|[URL](https://github.com/rlabrecque/Steamworks.NET)|

## Documents

- [Server Settings](Documents/ServerSettings.md)
- [Build Manual](Documents/BuildManual.md)
- [Server Message API Reference](Documents/ServerMessageAPIReference.md)
- [NAT Traversal with UPnP](Documents/NatTraversal.md)

## License

The codes in this repository except codes from other repositories are lisenced unfer the [MIT License](https://github.com/CdecPGL/PlanetaMatchMaker/blob/master/LICENSE).

This repogitory includes following libraries from other repogitories.
The licenses of these codes follows each repogitories.

- [namepf C++](https://github.com/Neargye/nameof) ([MIT License](https://github.com/Neargye/nameof/blob/master/LICENSE))
- [minimal-serializer](https://github.com/CdecPGL/minimal-serializer) ([MIT License](https://github.com/CdecPGL/minimal-serializer/blob/master/LICENSE))
- [Open.NAT](https://github.com/lontivero/Open.NAT) ([MIT License](https://github.com/lontivero/Open.NAT/blob/master/LICENSE))
