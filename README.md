# PlanetaMatchMaker

A match making server and client for network game.

## Server

### Dependencies

- Boost Library 1.53 (from vcpgk)

### Functions

- Create rooms and join rooms
- Random match making (not implemented)

### Environment Requirement

- Windowds 7 or higher
- Linux

### Build

#### Windows

1. Install Boost Library 1.53 or higher
1. Install a compiler (VC++ or clang) which is compatible with C++17
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
sudo yum -y install gcc-c++ # use for build latest g++
sudo yum -y install bzip2 # use for installing latest g++
sudo yum -y install boost boost-devel
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
# install latest g++ manually because g++ installed through yum is old.
cd ~
wget http://ftp.tsukuba.wide.ad.jp/software/gcc/releases/gcc-9.1.0/gcc-9.1.0.tar.gz
tar zxvf gcc-9.1.0.tar.gz
cd gcc-9.1.0
./contrib/download_prerequisites
mkdir build
cd build
../configure --enable-languages=c++ --prefix=/usr/local --disable-bootstrap --disable-multilib
make # make will fail with too small memory
sudo make install # g++ will be installed in /usr/local/lib64
g++ --version # check if g++ is installed successfully.
# build match maker server
git clone https://github.com/CdecPGL/PlanetaMatchMaker.git
cd PlanetaMatchMaker
mkdir build
cd build
cmake ..
make
# sudo make install
# PlanetaMatchMakerServer
# vi ~/.planeta_match_maker_server/setting.json
```

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
