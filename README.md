# PlanetaMatchMaker

A match making server and client for network game.

## Server

### Dependencies

- Boost Library 1.70
- CMake 3.8
- minimal-serializer v0.1.5 (included in this repogitory)

### Functions

- Create rooms and join rooms
- Random match making (not implemented)

### Supported Platforms

- Windowds 7 or higher
- Linux(Ubuntu)
- Docker

### Build

#### Windows

1. Install a compiler (VC++15.7, g++7 or clang5) which is compatible with C++17
1. Install vcpkg
1. Install Boost Library 1.70 or higher by vcpkg
1. Execute (1) or (2) process
1. (1)Open PlanetaMatchMaker.sln and build PlanetaMatchMakerServer project
1. (2)Install cmake
1. (2)mkdir build
1. (2)cmake ..
1. (2)make install

#### Linux(Ubuntu)

Install by refering to Docker/pmms/Dockerfile .

Ubuntu18.0.4 is supported.

#### Docker

1. Install Git
1. Install Docker and Docker-Compose
1. Clone https://github.com/CdecPGL/PlanetaMatchMaker.git
1. Move to Docker directory in the cloned repository
1. Execute docker-compose up

### Run

#### Windows

1. Change `~/.pmms/setting.json` if need
1. Open CommandPrompt or PowerShell
1. Execute `pmms` command

#### Linux(Ubuntu)

1. Change `~/.pmms/setting.json` if need
1. Open bash
1. Execute `pmms` command

#### Docker

1. Move to `[PlanetaMatchMaker Root]/Docker` directory
1. Execute `docker-compose up` command

### Setting File

You can locate a setting file `setting.json` to `.pmms` directory in home directory.

## Client

### Dependencies

- minimal-serializer v0.1.5 (included in this repogitory)
- [Open.NAT (compile error fixed version of commit 643f04e8227fe873731b884244bcee4cc84c8d49)](https://github.com/lontivero/Open.NAT) (included in this repogitory)

### Windows

1. Install Visual Studio 2019 or higher
1. Open the solution
1. Build the project

### Linux

1. Unknown

## TestClient

### Dependencies

- Command Line Parser v2.6.0 (from NuGet)

## UnityClient

## Coding Rule

### C++ Project

### Standard C# Project

- Naming Rule: https://qiita.com/Ted-HM/items/67eddbe36b88bf2d441d

### Unity C# Project
