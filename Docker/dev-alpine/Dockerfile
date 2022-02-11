FROM alpine:3.15.0
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

CMD ["/bin/ash"]
