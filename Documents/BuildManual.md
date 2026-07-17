# Build Manual

This is a manual to build server and client from source code.
In example code of this manual, default current directory is the directory this repogitory is placed.

## Server

### Environment

To build server project, you need MSVC or clang whose version is compatible with C++20.
gcc is not supported.

Following compilers and platforms are tested.

- MSVC 14.50 (Windows)
- clang 12.0.1 (Debian)

### Dependencies

- Boost Library 1.89.0
- OpenSSL 3.0 or higher (3.6.2 is pinned in `vcpkg.json` and used by the Docker images)
- CMake 3.21.3 or higher
- minimal-serializer v0.2.4 (included in this repogitory)
- nameof C++ 0.10.1 (included in this repogitory)

OpenSSL must provide a CMake package configuration file so that `find_package(OpenSSL 3.0 REQUIRED)` can resolve it. The repository `vcpkg.json` manifest installs the dependency when CMake is configured with the vcpkg toolchain. When installing dependencies manually, add its installation prefix to `CMAKE_PREFIX_PATH` as needed.

### Build by CMake with Docker

1. Install docker
1. Build a docker image
1. Run a docker container
1. Create build directory and move created directory
1. Generate makefile with cmake command
1. Build with make command
1. Run test with ctest command if need

```bash
docker build -t planeta-match-maker-dev Docker/dev

# For Linux
docker run -v ./:/planeta-match-maker -it planeta-match-maker-dev /bin/bash

# For Windows PowerShell
docker run -v "${PWD}:/planeta-match-maker" -it planeta-match-maker-dev /bin/bash

# In the docker container
cd planeta-match-maker
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=clang++
make -j"$(nproc)"
# Run test if need
ctest
# Calculate coverage if need
../tools/coverage.sh
```

The server project is not compatible with gcc so specify clang++ to cmake.
The development Docker image sets `PMMS_ENABLE_COVERAGE=ON`, so coverage instrumentation is enabled by default in this image.
Run `../tools/coverage.sh` from the build directory after `ctest`; the HTML report is generated in `build/lcovHtml/index.html`.

The default server setting uses TLS. Place a certificate chain and private key at the configured paths, or explicitly set `tls.mode` to `"plain"` for local plain TCP testing. See [TLS Certificate Setup](TLSCertificate.md) for certificate examples.

### Build by CMake Manually (Linux)

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
make -j"$(nproc)"
# Run test if need
ctest
```

The server project is not compatible with gcc so specify clang++ to cmake.
Coverage instrumentation is disabled by default outside the development Docker image.
To calculate coverage manually, install `lcov`, `llvm-cov`, and the Clang profile runtime package such as `libclang-rt-14-dev`, configure with `-DENABLE_COVERAGE=ON`, run tests, and then run `../tools/coverage.sh` from the build directory.

```bash
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=clang++ -DENABLE_COVERAGE=ON
make -j"$(nproc)"
ctest
../tools/coverage.sh
```

The coverage HTML report is generated in `build/lcovHtml/index.html`.

The default server setting uses TLS. Place a certificate chain and private key at the configured paths, or explicitly set `tls.mode` to `"plain"` for local plain TCP testing. See [TLS Certificate Setup](TLSCertificate.md) for certificate examples.

### Build by Visual Studio (Windows)

1. Install a compiler (VC++ or clang) which is compatible with C++20
1. Install vcpkg
1. Open `PlanetaMatchMaker.sln`
1. Build `PlanetaMatchMakerServer` project\
    Required packaged will be installed by vcpkg automatically
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

The test client defaults to the credential-free `None` method for development servers. Select `Steam` with
the `authentication_method` option when external authentication is enabled. Authentication credentials are then read
from the `PMMS_TEST_CLIENT_AUTHENTICATION_CREDENTIAL` environment variable by default. The command-line option
`--authentication_credential_environment_variable` selects a different variable.
Do not place a Steam ticket directly in command-line arguments because process listings and shell
history may expose it.

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

- Unity 2019.3.17f1 (Windows)
- Unity 2022.3.62f3 (Windows)

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

#### MSVC: `fatal  error C1189: #error:  WinSock.h has already been included`

- Target: ServerTest
- Library: Boost Library 1.78.0
- Platform: MSVC
- Reference: https://groups.google.com/g/boost-list/c/TKNG4U5UDU0

Add `WIN32_LEAN_AND_MEAN` to predefined macro.

#### MSVC: `error LNK2005: "public: virtual __cdecl boost::unit_test::lazy_ostream::~lazy_ostream(void)"`

- Target: ServerTest
- Library: Boost Library 1.78.0
- Platform: MSVC

Include `boost/test/unit_test.hpp` intead of `boost/test/included/unit_test.hpp`.
