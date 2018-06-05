# U2Fローカルテストサーバー

## 概要
PC環境で、U2Fサーバー機能をデモするためのツール群です。

macOS上のPython環境下で動作します。

## 構成物件
* htdocs : 静的Webコンテンツ
* python-u2flib-server : U2Fライブラリーサーバー
* u2f-chrome-extension : パッケージされていないChrome U2Fエクステンション

### 動作環境
macOS Sierra（Version 10.12.6）<br>
Google Chrome（Version 67.0.3396.62）

## 手順書

- <b>[U2Fテスト用ローカルサーバー構築手順](../Research/U2F_LOCAL_TSTSVR.md) </b><br>
macOS上のApacheで稼働するローカルWebサーバーに、U2Fライブラリーサーバーを組み込む手順を掲載いたします。

- <b>[Chromeブラウザーを使用したデモ手順](documents/CHROMEDEMO.md) </b><br>
ChromeおよびU2Fエクステンションを使用し、U2F管理ツール、One Cardを使用して、U2F Register／Authenticateを実行するまでの手順を掲載しております。

## ご参考

- <b>[Chrome U2Fエクステンションのパッケージ手順](documents/CHROMEEXTPACK.md) </b><br>
`u2f-chrome-extension.crx`をパッケージする方法について掲載いたします。<br>
ただし、この手順で作成したパッケージ済みエクステンションは、Chrome 67以降ではインストールができませんのでご注意ください。
