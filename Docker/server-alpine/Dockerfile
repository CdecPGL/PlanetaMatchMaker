FROM alpine:3.11.0
LABEL maintainer="Cdec <cdecrement@gmail.com>"

# Install basic requirements
# Install ssh and git for CircleCI
RUN apk update && apk add --no-cache \
  make \
  cmake \
  git \
  openssh \
  clang \
  g++ \
  boost-dev

# Clone PlanetaMatchMakerServer repository
RUN git clone https://github.com/CdecPGL/PlanetaMatchMaker.git

# Build PlanetaMatchMakerServer
RUN \
  cd PlanetaMatchMaker && \
  mkdir build && \
  cd build && \
  cmake -DCMAKE_CXX_COMPILER=clang++ .. && \
  make && \
  make install

# Copy pmms setting
COPY pmms/setting.json /etc/pmms/

# Start server
CMD ["pmms"]
