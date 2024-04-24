FROM debian:bookworm-20240408-slim
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
  && ./b2 install toolset=clang link=static,shared --prefix=/usr/local -j"$(nproc)" \
  && cd .. \
  && rm -rf boost

CMD ["/bin/bash"]
