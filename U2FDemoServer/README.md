# U2Fローカルテストサーバー

## 概要
PC環境で、U2Fサーバー機能をデモするためのツール群です。

macOS上のPython環境下で動作します。

## 構成物件
* htdocs : 静的Webコンテンツ
* python-u2flib-server : U2Fライブラリーサーバー
* u2f-chrome-extension.crx : Chrome U2Fエクステンション（パッケージ済み）
* u2f-chrome-extension.pem : Chrome U2Fエクステンションのパッケージ用秘密鍵
* u2f-chrome-extension : Chrome U2Fエクステンションのソースコード
* U2FMaintenanceTool : U2F管理ツール（エクステンションID修正版＝上記エクステンションから起動されるように改修したもの）

### 動作環境
macOS Sierra (Version 10.12.6)

## 手順書

- <b>[Chrome U2Fエクステンションのパッケージ手順](documents/CHROMEEXTPACK.md) </b><br>
`u2f-chrome-extension.crx`をパッケージする方法について掲載いたします。

- <b>[U2F管理ツールの修正手順](documents/U2FMTBUILD.md) </b><br>
U2F管理ツール（エクステンションID修正版）のインストール媒体修正手順を、以下に掲載いたします。
