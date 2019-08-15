FROM ubuntu:18.04
LABEL maintainer="Cdec <cdecrement@gmail.com>"

# Prefent to display "debconf: delaying package configuration, since apt-utils is not installed"
ENV DEBIAN_FRONTEND noninteractive

# Install basic requirements
RUN apt-get update && apt-get install -y \
  cmake \
  git \
  software-properties-common \
  wget \
&& apt-get clean \
&& rm -rf /var/lib/apt/lists/*

# Add apt repositories
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
  wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
  apt-add-repository -y "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-8 main" && \
  add-apt-repository ppa:mhier/libboost-latest

# Install Boost Library 1.70 or higher
# Install g++9 to install libc++ compatible with C++17
# Install Clang8
# Install Boost Library 1.70 or higher
RUN apt-get update && apt-get install -y \
  clang-8 \
  g++-9 \
  libboost1.70-dev \
  lld-8 \
  lldb-8 \
&& apt-get clean \
&& rm -rf /var/lib/apt/lists/*

# Clone PlanetaMatchMakerServer repository
RUN git clone https://github.com/CdecPGL/PlanetaMatchMaker.git

# Build PlanetaMatchMakerServer
RUN \
  cd PlanetaMatchMaker && \
  mkdir build && \
  cd build && \
  cmake -DCMAKE_CXX_COMPILER=clang++-8 .. && \
  make && \
  make install

# Copy setting file
COPY setting.json ~/.pmms

# Start server
CMD ["pmms"]
