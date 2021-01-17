[![Release](https://img.shields.io/github/v/release/CdecPGL/PlanetaMatchMaker?include_prereleases&sort=semver)](https://github.com/CdecPGL/PlanetaMatchMaker/releases)
[![License](https://img.shields.io/github/license/CdecPGL/PlanetaMatchMaker)](https://github.com/CdecPGL/PlanetaMatchMaker/blob/master/LICENSE)
[![CircleCI Buld and Test Status](https://circleci.com/gh/CdecPGL/PlanetaMatchMaker/tree/master.svg?style=shield)](https://circleci.com/gh/CdecPGL/PlanetaMatchMaker/tree/master)

# Planeta Match Maker

[README (English)](README.md)

P2Pオンラインゲーム向けのシンプルで軽量なマッチメイキングシステムです。
Linux, Windows向けのサーバーバイナリおよびUnityを含めたC#向けクライアントライブラリが提供されます。

## 機能

- ルームの作成とルームへの参加
- ルーム所有者名によるルームの検索
- UPnPを用いたポートマッピングの自動作成によるNAT越え
- (未実装) ランダムマッチ

## プラットフォーム

### サーバー

以下のプラットフォーム使用可能なバイナリが提供されます。

- Windows
- Linux

### クライアント

以下の言語およびプラットフォームで使用可能なライブラリが提供されます。

- C# (.Net Standard 2.1に準拠した.Net Framework又は.Net Core)
- Unity (.Net 4.0)

## 使用方法

簡単にサーバーとクライアントをインストールし、使用できます。

### サーバー (Docker)

Dockerを使用することで、数ステップでサーバーをインストールすることができます。

1. `cdec/plaenta-match-maker:server-alpine`をタグとして指定し、Dockerイメージをプルする
2. プルしたイメージでコンテナを実行する

以下のコマンドは、Dockerを使用してポート57000でサーバーを起動する例です。

```bash
docker pull cdec/plaenta-match-maker:server-alpine
docker run -p 57000:57000 cdec/plaenta-match-maker:server-alpine
```

サーバー設定ファイルで指定されているポートでTCP通信を受け取ることができるよう、ファイアーウォールの設定が必要になることがあります。

サーバーの設定は[サーバー設定ファイル](Documents/ServerSettings.md)を編集することで行えます。

### サーバー (手動)

LinuxおよびWindowsで、以下の手順によりサーバーを手動でインストールすることができます。

1. リリースページから実行可能ファイルをダウンロードする
1. 任意の場所に実行可能ファイルを配置する
1. サーバー設定ファイルを`/etc/pmms/setting.json`に作成する
1. 実行可能ファイルを実行する

サーバー設定ファイルで指定されているポートでTCP通信を受け取ることができるよう、ファイアーウォールの設定が必要になることがあります。

サーバーの設定は[サーバー設定ファイル](Documents/ServerSettings.md)を編集することで行えます。

### C#クライアント

1. リリースページからソースコードをダウロードするか、このリポジトリをクローンする
1. ダウンロードしたソースコードの`PlanetaMatchMakerClient/Source`ディレクトリを使用したいプロジェクトにコピーする

### Unityクライアント (Unityパッケージ)

1. リリースページからUnityパッケージをダウンロードする
1. Unityパッケージを使用したいプロジェクトにインポートする

### Unityクライアント (手動)

1. リリースページからソースコードをダウロードするか、このリポジトリをクローンする
1. ダウンロードしたソースコードの`PlanetaMatchMakerUnityClient/Assets`ディレクトリを使用したいプロジェクトの`Assets` ディレクトリ配下にコピーする

## ドキュメント

- [サーバー設定（英語）](Documents/ServerSettings.md)
- [ビルド手順（英語）](Documents/BuildManual.md)
- [サーバーメッセージAPIリファレンス（英語）](Documents/ServerMessageAPIReference.md)
- [NUPnPによるNAT越えの流れ（英語）](Documents/NatTraversal.md)

## ライセンス

他のリポジトリ由来のコードを除き、本リポジトリのコードには[MITライセンス](https://github.com/CdecPGL/PlanetaMatchMaker/blob/master/LICENSE)が適用されます。

本リポジトリには、他のリポジトリ由来の以下のライブラリが含まれます。
これらのライブラリのコードについては、各リポジトリのライセンスが適用されます。

- [namepf C++](https://github.com/Neargye/nameof) ([MITライセンス](https://github.com/Neargye/nameof/blob/master/LICENSE))
- [minimal-serializer](https://github.com/CdecPGL/minimal-serializer) ([MITライセンス](https://github.com/CdecPGL/minimal-serializer/blob/master/LICENSE))
- [Open.NAT](https://github.com/lontivero/Open.NAT) ([MITライセンス](https://github.com/lontivero/Open.NAT/blob/master/LICENSE))
