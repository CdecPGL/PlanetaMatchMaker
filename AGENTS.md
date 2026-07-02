# Repository Guidelines

## Project Structure & Module Organization

This repository contains a lightweight match-making server, C# client libraries, tests, and a Unity package. C++ server code lives in `PlanetaMatchMakerServer/source`, with vendored headers in `PlanetaMatchMakerServer/library`. C++ unit tests are in `PlanetaMatchMakerServerTest`. The .NET client source is in `PlanetaMatchMakerClient/Source`, with MSTest coverage in `PlanetaMatchMakerClientTest/Source`. `PlanetaMatchMakerTestClient` is a CLI test client. Unity-facing assets are under `PlanetaMatchMakerUnityClient/Assets/PlanetaGameLabo/MatchMaker`; keep Unity `.meta` files with their assets. Documentation is in `Documents`.

## Build, Test, and Development Commands

- `cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++`: configure the C++ server build on Linux. GCC is not supported.
- `cmake --build build --config Release`: build the server and C++ test target.
- `ctest --test-dir build`: run the C++ Boost.Test suite.
- `dotnet build PlanetaMatchMakerClient -c Release`: build the .NET client library.
- `dotnet test PlanetaMatchMakerClientTest -c Release`: run MSTest client tests.
- `dotnet publish PlanetaMatchMakerTestClient -c Release --self-contained true -p:PublishSingleFile=true -p:PublishTrimmed=true -r win-x64`: publish a standalone test client.
- `docker build -t planeta-match-maker-dev Docker/dev`: build the documented Linux development image.

## Coding Style & Naming Conventions

Follow `.editorconfig`. Files use UTF-8 and LF endings. Default indentation is tabs at width 4; C# uses spaces. C++ targets C++20. C# naming is PascalCase for types and methods, camelCase for locals and parameters, `I`-prefixed interfaces, and `T`-prefixed type parameters. Prefer existing namespace and folder patterns when adding code.

## Testing Guidelines

Add C++ tests beside related coverage in `PlanetaMatchMakerServerTest`, naming files like `server_setting_test.cpp`. Add C# tests in `PlanetaMatchMakerClientTest/Source`, using MSTest classes and methods with clear behavior names. Run the relevant test command before submitting changes; use `../tools/coverage.sh` from a Linux build directory when coverage data is needed.

## Commit & Pull Request Guidelines

Recent commits use bracketed categories such as `[add]`, `[fix]`, and `[update]`, often followed by an issue number, for example `[fix] #25 fix server test errors`. Keep subjects imperative and scoped. Pull requests should describe the change, list validation commands, link related issues, and include screenshots or Unity package notes when Unity-visible behavior changes.

## Security & Configuration Tips

Do not commit local credentials, Unity license files, generated build output, or test artifacts. Server runtime configuration is documented in `Documents/ServerSettings.md`; prefer documenting new settings there with defaults.
