# PlanetaMatchMaker

A match making server and client for network game.

## Server

### Dependencies

- Boost Library 1.70 (from vcpgk)
- CMake 3.15

### Functions

- Create rooms and join rooms
- Random match making (not implemented)

### Environment Requirement

- Windowds 7 or higher
- Linux

### Build

#### Windows

1. Install Boost Library 1.70 or higher
1. Install a compiler (at least, VC++15.7, g++7 or clang5) which is compatible with C++17
1. Unknown

#### Linux

1. Install CMake
1. mkdir build
1. cmake ..
1. make

##### Ex. Cent OS 7

```bash
# install neccesarry packages
sudo yum -y install git
sudo yum -y install wget
sudo yum -y install gcc-c++ # use for build latest clang
sudo yum -y install bzip2 # use for installing latest g++

sudo yum -y install centos-release-scl

# install boost library 1.70+
install boost?
# install latest cmake manually because cmake installed through yum is old.
wget https://github.com/Kitware/CMake/releases/download/v3.15.1/cmake-3.15.1.tar.gz
tar zxvf cmake-3.15.1.tar.gz
cd cmake-3.15.1
./bootstrap --prefix=/opt/cmake --no-system-libs
make
sudo make install
vi ~/.bashrc # add PATH=$PATH:/opt/cmake/bin
source ~/.bashrc
cmake -version # check if cmake is installed successfully.
cd ~
# install latest clang manually because clang installed through yum is old. http://clang.llvm.org/get_started.html
cd ~
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=g++ -DLLVM_ENABLE_PROJECTS=clang -G "Unix Makefiles" ../llvm
# build match maker server
git clone https://github.com/CdecPGL/PlanetaMatchMaker.git
cd PlanetaMatchMaker
mkdir build
cd build
cmake  -DCMAKE_CXX_COMPILER=clang++ ..
make
sudo make install
# change setting if need
vi ~/.pmms/setting.json
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
