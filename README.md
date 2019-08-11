# PlanetaMatchMaker

A match making server and client for network game.

## Server

### Dependencies

- Boost Library 1.70
- CMake 3.8

### Functions

- Create rooms and join rooms
- Random match making (not implemented)

### Environment Requirement

- Windowds 7 or higher
- Linux(Ubuntu)

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

In Ubuntu18.0.4, PlanetaMatchMakerServer is installable by below commands.

```bash
# Install Git
sudo apt install git
# Install g++9 to install libc++ compatible with C++17
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-9
# Install Clang8
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get install software-properties-common
sudo apt-add-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-8 main"
sudo apt update
sudo apt install clang-8 lldb-8 lld-8
# Install CMake
sudo apt install cmake
# Install Boost Library 1.70 or higher
sudo add-apt-repository ppa:mhier/libboost-latest # find ppa which has latest boost library
sudo apt install libboost1.70-dev
# Clone PlanetaMatchMakerServer repository
git clone https://github.com/CdecPGL/PlanetaMatchMaker.git
# Build PlanetaMatchMakerServer
cd PlanetaMatchMaker
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=clang++-8 ..
make
sudo make install
```

### Setting File

You can locate a setting file `setting.json` to `.pmms` directory in home directory.

## Client

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
