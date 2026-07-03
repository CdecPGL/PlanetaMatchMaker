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

Add C++ tests beside related coverage in `PlanetaMatchMakerServerTest`, naming files like `server_setting_test.cpp`. Keep pure C++ unit tests under `PlanetaMatchMakerServerTest/unit_tests`, and keep protocol compliance tests under `PlanetaMatchMakerServerTest/protocol_tests`. For `protocol_tests`, treat specification coverage as the quality gate: tests should cover 100% of the documented protocol specification, including success cases, error codes, disconnect behavior, and no-reply notice behavior. Add C# tests in `PlanetaMatchMakerClientTest/Source`, using MSTest classes and methods with clear behavior names. Run the relevant test command before submitting changes, and treat any build or test warnings as issues to fix before considering validation complete. Resolve all build warnings; do not leave warning output in accepted changes. Use `../tools/coverage.sh` from a Linux build directory when coverage data is needed.

## Commit & Pull Request Guidelines

Follow the repository commit message convention below for all future non-merge commits:

- Format: `[category] [#issue] imperative subject`
- Use the issue number immediately after the category when one exists, for example `[fix] #25 fix server test errors`. Omit the issue segment when there is no related issue.
- Valid categories are `[add]` for new features, files, or tests; `[fix]` for bug, build, or test fixes; `[update]` for behavior, dependency, configuration, documentation, or version changes; `[remove]` for deleting code, settings, or documentation; and `[refactor]` for internal restructuring without intended behavior changes.
- Write the subject in English, keep it concise and scoped, and prefer the imperative style used in the history: `add`, `fix`, `update`, `change`, `remove`, or `refactor`.
- Do not add a trailing period to a one-line subject. Put longer rationale or migration notes in the commit body instead of making the subject long.
- Merge commits may keep the default Git-generated merge message.

Pull requests should describe the change, list validation commands, link related issues, and include screenshots or Unity package notes when Unity-visible behavior changes.

## Security & Configuration Tips

Do not commit local credentials, Unity license files, generated build output, or test artifacts. Server runtime configuration is documented in `Documents/ServerSettings.md`; prefer documenting new settings there with defaults.
