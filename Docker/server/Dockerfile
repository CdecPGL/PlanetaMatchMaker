FROM debian:bookworm-20240408-slim as base
LABEL maintainer="Cdec <cdecrement@gmail.com>"

# Install basic requirements
RUN apt update && apt install -y \
  make \
  cmake \
  git \
  clang \
  lcov \
  llvm \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/*

# Install boost library
RUN git clone https://github.com/boostorg/boost.git \
  && cd boost \
  && git checkout boost-1.84.0 \
  && git submodule update --init \
  && ./bootstrap.sh \
  && ./b2 install toolset=clang link=static --prefix=/usr/local -j"$(nproc)" \
  && cd .. \
  && rm -rf boost

# Clone PlanetaMatchMakerServer repository
RUN git clone https://github.com/CdecPGL/PlanetaMatchMaker.git

# Build PlanetaMatchMakerServer with static link
RUN \
  cd PlanetaMatchMaker && \
  mkdir build && \
  cd build && \
  cmake -DCMAKE_CXX_COMPILER=clang++ -DSTATIC_LINK_DEPENDENCIES=ON .. && \
  make -j"$(nproc)"

FROM gcr.io/distroless/static-debian12
LABEL maintainer="Cdec <cdecrement@gmail.com>"

# Copy PlanetaMatchMakerServer binary (same location of make install result)
COPY --from=base --chmod=755 ./PlanetaMatchMaker/build/PlanetaMatchMakerServer/PlanetaMatchMakerServer /usr/local/bin/pmms

# Copy pmms setting
COPY pmms/setting.json /etc/pmms/

# Start server
CMD ["pmms"]
