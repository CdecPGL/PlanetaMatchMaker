FROM debian:bookworm-20251117-slim AS base
LABEL maintainer="Cdec <cdecrement@gmail.com>"

# Install basic requirements
RUN apt update && apt install -y \
  make \
  cmake \
  git \
  clang \
  lcov \
  llvm \
  libssl-dev \
  openssl \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/*

# Install boost library
RUN git clone https://github.com/boostorg/boost.git \
  && cd boost \
  && git checkout boost-1.89.0 \
  && git submodule update --init \
  && ./bootstrap.sh \
  && ./b2 install toolset=clang link=static --prefix=/usr/local -j"$(nproc)" \
  && cd .. \
  && rm -rf boost

# Copy PlanetaMatchMakerServer repository
COPY . /PlanetaMatchMaker

# Build PlanetaMatchMakerServer with static Boost dependencies
RUN \
  cd PlanetaMatchMaker && \
  cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++ -DSTATIC_LINK_DEPENDENCIES=ON && \
  cmake --build build --target PlanetaMatchMakerServer --config Release -j"$(nproc)"

FROM gcr.io/distroless/cc-debian12
LABEL maintainer="Cdec <cdecrement@gmail.com>"

# Copy OpenSSL runtime libraries
COPY --from=base /usr/lib/x86_64-linux-gnu/libssl.so.3 /usr/lib/x86_64-linux-gnu/
COPY --from=base /usr/lib/x86_64-linux-gnu/libcrypto.so.3 /usr/lib/x86_64-linux-gnu/

# Copy PlanetaMatchMakerServer binary (same location of make install result)
COPY --from=base --chmod=755 /PlanetaMatchMaker/build/PlanetaMatchMakerServer/PlanetaMatchMakerServer /usr/local/bin/pmms

# Copy pmms setting
COPY Docker/server/pmms/setting.json /etc/pmms/

# Start server
CMD ["pmms"]
