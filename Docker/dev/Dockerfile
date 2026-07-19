FROM debian:bookworm-20260623-slim
LABEL maintainer="Cdec <cdecrement@gmail.com>"

# Install basic requirements
RUN apt update && apt install -y --no-install-recommends \
  make \
  cmake \
  git \
  clang \
  libclang-rt-14-dev \
  lcov \
  llvm \
  tar \
  pkg-config \
  perl \
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

ENV OPENSSL_ROOT_DIR=/usr/local/openssl
ENV CMAKE_PREFIX_PATH=/usr/local/openssl
ENV PMMS_ENABLE_COVERAGE=ON

# Install boost library
ARG BOOST_VERSION=1.89.0
ARG BOOST_ARCHIVE_VERSION=1_89_0
ARG BOOST_SHA256=9de758db755e8330a01d995b0a24d09798048400ac25c03fc5ea9be364b13c93
ADD https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_ARCHIVE_VERSION}.tar.gz /tmp/boost.tar.gz
RUN echo "${BOOST_SHA256}  /tmp/boost.tar.gz" | sha256sum -c - \
  && mkdir /tmp/boost-src \
  && tar -xzf /tmp/boost.tar.gz -C /tmp/boost-src --strip-components=1 \
  && cd /tmp/boost-src \
  && ./bootstrap.sh \
  && ./b2 install toolset=clang link=static,shared --prefix=/usr/local -j"$(nproc)" \
    --with-coroutine \
    --with-date_time \
    --with-filesystem \
    --with-json \
    --with-log \
    --with-test \
    --with-thread \
  && rm -rf /tmp/boost.tar.gz /tmp/boost-src

CMD ["/bin/bash"]
