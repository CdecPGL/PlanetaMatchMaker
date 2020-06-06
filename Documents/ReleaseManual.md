# Release Manual

This is a manual to release the libraries to GitHub.

## Procedure

1. Add tag which starts with "v"
1. Push created tag
1. Build server binary for windows manually because it is not build by CircleCI currently
1. Wait CircleCI finish building, creating release and aploading artifacts
1. Upload server binary for windows to created release in GitHub manually

## Pre-release

By adding tag which starts with "v0.", CircleCI marks release as pre-release automatically.
