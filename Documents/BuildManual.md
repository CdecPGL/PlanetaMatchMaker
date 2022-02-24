# Build Manual

This is a manual to build server and client from source code.
In example code of this manual, default current directory is the directory this repogitory is placed.

## Server

### Environment

To build server project, you need a compiler which is compatible with C++17.

Following compilers and platforms are tested.

- MSVC 14.30 (VS 17.0.2) (Windows)
- clang 12.0.1 (Alpine)

g++ is not supported now.

### Dependencies

- Boost Library 1.77 or higher
- CMake 3.21.3 or higher
- minimal-serializer v0.2.4 (included in this repogitory)
- nameof C++ 0.10.1 (included in this repogitory)

### Build by CMake with Docker (Linux)

1. Install docker
1. Build a docker image
1. Run a docker container
1. Create build directory and move created directory
1. Generate makefile with cmake command
1. Build with make command
1. Run test with ctest command if need

```bash
docker build -t planeta-match-maker-dev PlanetaMatchMaker/Docker/dev
docker run -it planeta-match-maker-dev /bin/ash
# In the docker container
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=clang++
make -j4
# Run test if need
ctest
```

Currently, the server project is compatible with gcc so specify clang++ to cmake.

### Bulild by CMake Manually (Linux)

1. Install cmake 3.21.3 or higher
1. Install clang++ 3.8 or higher
1. Create build directory and move created directory
1. Generate makefile with cmake command
1. Build with make command
1. Run test with ctest command if need

Following commands are example.

```bash
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=clang++
make -j4
# Run test if need
ctest
```

Currently, the server project is compatible with gcc so specify clang++ to cmake.

### Build by Visual Studio (Windows)

1. Install a compiler (VC++ or clang) which is compatible with C++20
1. Install vcpkg
1. Install Boost Library 1.77 or higher by vcpkg
1. Open `PlanetaMatchMaker.sln`
1. Build `PlanetaMatchMakerServer` project
1. Build and run `PlanetaMatchMakerServerTest` project if need

## Client

### Environment

- C# Compiler using .Net Core 3.1 or higher as a backend (the backend which is compatible with .Net Standard 2.1 is also OK but not tested)

### Dependencies

- minimal-serializer v0.2.4 (included in this repogitory)
- [Open.NAT (compile error fixed version of commit 643f04e8227fe873731b884244bcee4cc84c8d49)](https://github.com/lontivero/Open.NAT) (included in this repogitory)

### Build by Visual Studio (Windows)

1. Install Visual Studio 2019 or higher
1. Open the solution
1. Build `PlanetaMatchMakerClient` project
1. Build and run `PlanetaMatchMakerClientTest` project

### Build by .Net Core CLI (Windows or Linux)

1. Install .Net Core SDK 3.1 or higher
1. Build `PlanetaMatchMakerClient` project
1. Build and run `PlanetaMatchMakerClientTest` project if need

```bash
dotnet build PlanetaMatchMakerClient -c Release
# Build and run test if need
dotnet test PlanetaMatchMakerClientTest -c Release
```

## TestClient

### Environment

- C# Compiler using .Net Core 3.1 or higher as a backend (the backend which is compatible with .Net Standard 2.1 is also OK but not tested)

### Dependencies

- minimal-serializer v0.2.4 (included in this repogitory)
- [Open.NAT (compile error fixed version of commit 643f04e8227fe873731b884244bcee4cc84c8d49)](https://github.com/lontivero/Open.NAT) (included in this repogitory)
- Command Line Parser v2.6.0 (from NuGet)

### Build by Visual Studio (Windows)

1. Install Visual Studio 2022 or higher
1. Open the solution
1. Build `PlanetaMatchMakerTestClient` project

### Build by .Net Core CLI (Windows or Linux)

1. Install .Net Core SDK 3.1 or higher
1. Build `PlanetaMatchMakerTestClient` project

```bash
dotnet build PlanetaMatchMakerTestClient -c Release
```

if you want to build self-contained and single file binary, run below command.

```bash
dotnet publish PlanetaMatchMakerTestClient -c Release --self-contained true -p:PublishSingleFile=true -p:PublishTrimmed=true -r ${RIDs}
```

RIDs is a identifier to indicate build target platform. Typical RIDs is as below.

- Windows: win-x64
- Linux: linux-x64
- Max OS X: osx-x64

If you want to know about RIDs, see [Microsoft's document](https://docs.microsoft.com/ja-jp/dotnet/core/rid-catalog).

## UnityClient

### Environment

You can use client in Unity with .Net 4.0. as a backend.

Following versions are tested.

- Unity 2019.2.12f1 (Windows)
- Unity 2019.3.13f1 (Windows)
- Unity 2021.2.8f1 (Windows)

### Dependencies

Same as C# client.

### Export unity package by Unity GUI (Windows, Linux or Mac OS X)

1. Open `PlanetaMatchMakerUnityClient` directory with Unity
1. Export `PlanetaMatchMakerUnityClient` directory as an unity package

### Export unity package by Unity CLI (Windows, Linux or Mac OS X)

1. Find a path your Unity is installed
1. Run unity from commandline with options to export PlanetaMatchMakerUnityClient.unitypackage

Below code is an example to export unity package to `PlanetaMatchMakerUnityClient/PlanetaMatchMakerUnityClient.unitypackage` in Linux.

```bash
UNITY_EXECUTABLE='A path of Unity executable'
. UNITY_EXECUTABLE -exportPackage Assets/PlanetaGameLabo PlanetaMatchMakerUnityClient.unitypackage -ProjectPath PlanetaMatchMakerUnityClient -batchmode -nographics -logfile unity_build.log -quit
```

## Supplement

### Compile Error

#### MSVC: `error C2039: 'value': is not a member of 'boost::proto'`

- Target: Server
- Library: Boost Library 1.70.0-1.71.0
- Platform: MSVC
- Reference: https://github.com/boostorg/proto/issues/20

Replace `#if BOOST_WORKAROUND(BOOST_MSVC, BOOST_TESTED_AT(1700))` to `#if BOOST_WORKAROUND(BOOST_MSVC, < 1800)` around line 230 of `boost/proto/generate.hpp`.
