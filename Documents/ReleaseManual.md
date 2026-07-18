# Release Manual

This is a manual to release the libraries.

## GitHub

### Procedure

1. Add tag which starts with "v"
1. Push created tag
1. Build server binary for windows manually because it is not build by CircleCI currently
1. Wait CircleCI finish building, creating release and aploading artifacts
1. Upload server binary for windows to created release in GitHub manually

### Pre-release

By adding tag which starts with "v0.", CircleCI marks release as pre-release automatically.

## Docker Hub

Before build image, push changes to master branch.

### Image Build

In repository root, run following command.

```bash
docker build -f Docker/server/Dockerfile -t cdec/planeta-match-maker-server:0.3.1 . --no-cache
```

Before publishing, build the opt-in HTTPS integration target. It inherits the
production image filesystem and TLS environment, then uses the PMMS
authentication HTTP client to call Steam's public `GetServerInfo` endpoint:

```shell
docker build -f Docker/server/Dockerfile --target production-https-test .
```

This check requires outbound HTTPS access and is intentionally not part of the
offline CTest suite.

### Push

```bash
docker push cdec/planeta-match-maker-server:0.3.1
```
