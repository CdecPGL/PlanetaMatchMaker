FROM debian:bookworm-20260623-slim AS base
LABEL maintainer="Cdec <cdecrement@gmail.com>"

# Install basic requirements
RUN apt update && apt install -y --no-install-recommends \
  make \
  cmake \
  clang \
  tar \
  pkg-config \
  perl \
  ca-certificates \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/*

# Install OpenSSL as static libraries with the same version pinned in vcpkg.json.
ARG OPENSSL_VERSION=3.6.2
ARG OPENSSL_SHA256=aaf51a1fe064384f811daeaeb4ec4dce7340ec8bd893027eee676af31e83a04f
ADD https://github.com/openssl/openssl/releases/download/openssl-${OPENSSL_VERSION}/openssl-${OPENSSL_VERSION}.tar.gz /tmp/openssl.tar.gz
RUN echo "${OPENSSL_SHA256}  /tmp/openssl.tar.gz" | sha256sum -c - \
  && mkdir /tmp/openssl-src \
  && tar -xzf /tmp/openssl.tar.gz -C /tmp/openssl-src --strip-components=1 \
  && cd /tmp/openssl-src \
  && CC=clang ./Configure linux-x86_64 no-shared no-module no-tests --prefix=/usr/local/openssl --openssldir=/usr/local/openssl/ssl \
  && make -j"$(nproc)" \
  && make install_sw \
  && rm -rf /tmp/openssl.tar.gz /tmp/openssl-src

# Install boost library as static libraries
ARG BOOST_VERSION=1.89.0
ARG BOOST_ARCHIVE_VERSION=1_89_0
ARG BOOST_SHA256=9de758db755e8330a01d995b0a24d09798048400ac25c03fc5ea9be364b13c93
ADD https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_ARCHIVE_VERSION}.tar.gz /tmp/boost.tar.gz
RUN echo "${BOOST_SHA256}  /tmp/boost.tar.gz" | sha256sum -c - \
  && mkdir /tmp/boost-src \
  && tar -xzf /tmp/boost.tar.gz -C /tmp/boost-src --strip-components=1 \
  && cd /tmp/boost-src \
  && ./bootstrap.sh \
  && ./b2 install toolset=clang link=static --prefix=/usr/local -j"$(nproc)" \
    --with-coroutine \
    --with-date_time \
    --with-filesystem \
    --with-json \
    --with-log \
    --with-test \
    --with-thread \
  && rm -rf /tmp/boost.tar.gz /tmp/boost-src

# Copy PlanetaMatchMakerServer repository
COPY . /PlanetaMatchMaker

# Build PlanetaMatchMakerServer with static dependency libraries
RUN \
  cd PlanetaMatchMaker && \
  cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++ -DSTATIC_LINK_DEPENDENCIES=ON \
    -DENABLE_COVERAGE=OFF \
    -DOPENSSL_ROOT_DIR=/usr/local/openssl && \
  cmake --build build --target PlanetaMatchMakerServer \
    PlanetaMatchMakerAuthenticationHttpsIntegrationTest --config Release -j"$(nproc)"

FROM gcr.io/distroless/static-debian12 AS production
LABEL maintainer="Cdec <cdecrement@gmail.com>"

# OpenSSL was built with a private default directory. Point it at the Debian CA
# bundle copied into the production image so set_default_verify_paths() works.
COPY --from=base /etc/ssl/certs/ca-certificates.crt /etc/ssl/certs/ca-certificates.crt
ENV SSL_CERT_FILE=/etc/ssl/certs/ca-certificates.crt

# Copy C++ runtime libraries required by the PMMS binary.
COPY --from=base /lib64/ld-linux-x86-64.so.2 /lib64/
COPY --from=base /lib/x86_64-linux-gnu/libc.so.6 /lib/x86_64-linux-gnu/
COPY --from=base /lib/x86_64-linux-gnu/libm.so.6 /lib/x86_64-linux-gnu/
COPY --from=base /lib/x86_64-linux-gnu/libstdc++.so.6* /lib/x86_64-linux-gnu/
COPY --from=base /lib/x86_64-linux-gnu/libgcc_s.so.1 /lib/x86_64-linux-gnu/

# Copy PlanetaMatchMakerServer binary (same location of make install result)
COPY --from=base --chmod=755 /PlanetaMatchMaker/build/PlanetaMatchMakerServer/PlanetaMatchMakerServer /usr/local/bin/pmms

# Copy the fail-closed Steam authentication configuration. Deployment-specific
# identifiers and the publisher key must be supplied through environment
# overrides or a replacement setting file before the server starts.
COPY Docker/server/pmms/setting.json /etc/pmms/setting.json
COPY Docker/server/pmms/README.md /etc/pmms/README.md

# Start server
CMD ["pmms"]

# Opt-in integration target. It executes the PMMS authentication HTTP client
# against Steam from the production filesystem and TLS configuration.
FROM production AS production-https-test
COPY --from=base --chmod=755 /PlanetaMatchMaker/build/PlanetaMatchMakerServerTest/PlanetaMatchMakerAuthenticationHttpsIntegrationTest /usr/local/bin/pmms-authentication-https-test
RUN ["/usr/local/bin/pmms-authentication-https-test"]

# Keep the default docker build output identical to the production stage.
FROM production AS final
