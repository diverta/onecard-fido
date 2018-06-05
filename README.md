# One Card FIDO対応

One CardにFIDO U2F認証機能を実装するプロジェクトです。

## What's new

#### 2018/06/05

Chrome 67以降、パッケージ済みエクステンションが、Chromeウェブストアで取得したもの以外インストールできなくなってしまったようです。<br>
そのため、（パッケージ済みエクステンション導入を前提とした）U2Fローカルテストサーバーの手順等を更新しております。

具体的には従来通り、パッケージ化されていないエクステンションを使用するように、実装および手順を修正しております。<br>
何卒ご容赦ください。

#### 2018/05/30

U2Fローカルテストサーバーを約半年ぶりにアップデートしました。<br>

- [U2FDemoServer](U2FDemoServer)
- [手順書などのドキュメント](U2FDemoServer/README.md)

下記の通り、大幅にアップデートしています。

- 従来の[Javaベースのライブラリーサーバー](Research/u2f-test-server)から、[Pythonベースのライブラリーサーバー](U2FDemoServer/python-u2flib-server)に変更<br>
[Yubico社提供のPythonライブラリーサーバー](https://developers.yubico.com/python-u2flib-server/)を使用し、[サンプル](https://github.com/Yubico/python-u2flib-server/blob/master/examples/u2f_server.py)を若干拡張して[サーバープログラム](U2FDemoServer/python-u2flib-server/u2f_server.py)を作成しました。<br>

- [Chrome U2Fエクステンション](U2FDemoServer/u2f-chrome-extension.crx)をパッケージ化<br>
この結果、エクステンションIDが以前のものから変更になっております。

- 上記変更後のエクステンションIDを反映した、[専用のU2F管理ツール](U2FDemoServer/U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)を用意<br>
ただし機能上の変更がない為、バージョンは、0.1.3のままとしております。

- 関連トピックス<br>[Issue #22](https://github.com/diverta/onecard-fido/issues/22), [Pull request #62](https://github.com/diverta/onecard-fido/pull/62) ご参照

#### [過去の更新履歴はこちら](HISTORY.md)

## BLE U2Fサービス

One CardのBLE上で稼働するFIDO U2F機能です。<br>
U2F v1.2の仕様に準拠しております。

FIDO U2Fに関する情報 : https://fidoalliance.org/download/

FIDO U2F対応デバイスは、USBポートに挿して使用する「YubiKey」が有名ですが、BLE U2Fサービスは、USBポートではなく、One CardのBLEを使用しています。

コード格納場所--->[nRF5_SDK_v13.0.0](nRF5_SDK_v13.0.0)

## U2F管理ツール

PC環境から、BLE U2Fサービスの動作に必要な鍵・証明書の導入などを行うツールです。
macOS版と、Windows版を用意しました。

コード格納場所--->[U2FMaintenanceTool](U2FMaintenanceTool)

## 運用に関する情報

[こちらのページ](Usage/README.md)からご参照いただけます。

## 開発に関する情報

[こちらのページ](Development/README.md)からご参照いただけます。

## 調査に関する情報

[こちらのページ](Research/README.md)からご参照いただけます。

## 障害に関する情報

[Issues](https://github.com/diverta/onecard-fido/issues)からご参照いただけます。
