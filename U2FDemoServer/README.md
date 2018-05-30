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

- <b>[U2Fテスト用ローカルサーバー構築手順](../Research/U2F_LOCAL_TSTSVR.md) </b><br>
macOS上のApacheで稼働するローカルWebサーバーに、U2Fライブラリーサーバーを組み込む手順を掲載いたします。

- <b>[Chrome U2Fエクステンションのパッケージ手順](documents/CHROMEEXTPACK.md) </b><br>
`u2f-chrome-extension.crx`をパッケージする方法について掲載いたします。

- <b>[U2F管理ツールの修正手順](documents/U2FMTBUILD.md) </b><br>
U2F管理ツール（エクステンションID修正版）のインストール媒体修正手順について掲載いたします。

- <b>[Chromeブラウザーを使用したデモ手順](documents/CHROMEDEMO.md) </b><br>
パッケージ化されたChrome U2Fエクステンションを使用し、U2F管理ツール（エクステンションID修正版）、One Cardを使用して、U2F Register／Authenticateを実行するまでの手順を掲載しております。
