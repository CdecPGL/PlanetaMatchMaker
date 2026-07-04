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

### Push

```bash
docker push cdec/planeta-match-maker-server:0.3.1
```
