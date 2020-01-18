# Build Manual

## Server

### Environment

To build server project, you need a compiler which is compatible with C++17.

Following compilers and platforms are tested.

- MSVC 14.6.2 (Windows)
- clang 8.0.0 (Ubuntu 18.04 and Alpine)

g++ is not supported now.

### Dependencies

- Boost Library 1.70 or higher
- CMake 3.8 or higher
- minimal-serializer v0.1.5 (included in this repogitory)
- nameof C++ 0.97 (included in this repogitory)

### Build by CMake with Docker

1. Install docker
1. Build a docker image
1. Run a docker container
1. Create build directory and move created directory
1. Generate makefile with cmake command
1. Build with make command

```bash
docker build -t planeta-match-maker:dev-alpine PlanetaMatchMaker/Docker/dev-alpine
docker run -it planeta-match-maker:dev-alpine /bin/ash
# In the docker container
mkdir build
cd build
cmake .. CMAKE_CXX_COMPILER=clang++
make build -j4
```

Currently, the server project is compatible with gcc so specify clang++ to cmake.

### Bulild by CMake Manually

1. Install cmake 3.8 or higher
1. Install clang++ 3.8 or higher
1. Create build directory and move created directory
1. Generate makefile with cmake command
1. Build with make command

Following commands are example.

```bash
mkdir build
cd build
cmake .. CMAKE_CXX_COMPILER=clang++
make build -j4
```

Currently, the server project is compatible with gcc so specify clang++ to cmake.

### Build by Visual Studio (Windows)

1. Install a compiler (VC++15.7, g++7 or clang5) which is compatible with C++17
1. Install vcpkg
1. Install Boost Library 1.70 or higher by vcpkg
1. Open `PlanetaMatchMaker.sln`
1. Build `PlanetaMatchMakerServer` project

## Client

### Environment

To build client project, you need a C# compiler which is compatible with .Net Standard 2.1.

### Dependencies

- minimal-serializer v0.1.5 (included in this repogitory)
- [Open.NAT (compile error fixed version of commit 643f04e8227fe873731b884244bcee4cc84c8d49)](https://github.com/lontivero/Open.NAT) (included in this repogitory)

### Build by Visual Studio (Windows)

1. Install Visual Studio 2019 or higher
1. Open the solution
1. Build `PlanetaMatchMakerClient` project

### Linux

Not checked.

## TestClient

### Environment

To build client project, you need a C# compiler which is compatible with .Net Standard 2.1.

### Dependencies

- minimal-serializer v0.1.5 (included in this repogitory)
- [Open.NAT (compile error fixed version of commit 643f04e8227fe873731b884244bcee4cc84c8d49)](https://github.com/lontivero/Open.NAT) (included in this repogitory)
- Command Line Parser v2.6.0 (from NuGet)

### Build by Visual Studio (Windows)

1. Install Visual Studio 2019 or higher
1. Open the solution
1. Build `PlanetaMatchMakerTestClient` project

### Linux

Not checked.

## UnityClient

### Environment

You can use client in Unity with .Net 4.0.

Following versions are tested.

- Unity 2019.2.12f1 (Windows)

### Dependencies

Same as C# client.

### Build by Unity

1. Import unity package to your project
1. Build
