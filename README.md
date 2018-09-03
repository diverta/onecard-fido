# One Card FIDO対応

One CardにFIDO U2F認証機能を実装するプロジェクトです。

## What's new

#### 2018/09/03

[ヘルパーデバイス（U2F USB HIDデバイス）](U2FHIDDevice/readme.md)を制作しました。<br>
ヘルパーアプリ（U2F Helper）との組み合わせで、One Cardを使用したChromeブラウザーでのU2F認証が可能となりました。

具体的な使用方法は、別途手順書[「Googleアカウントを使ったテスト手順」](Usage/GOOGLEACCTEST.md)をご参照ください。

#### 2018/08/21（Version 0.1.5）

以下のプログラムを修正しました。<br>

- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)
- [Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

修正点は以下になります。
- U2F Helperの新規制作（[Issue #72](https://github.com/diverta/onecard-fido/issues/72)）に伴う機能追加／修正

U2F Helper（ヘルパーアプリ）は、現在製作中のU2F HIDデバイス（ヘルパーデバイス）と連携し、One Cardを使ったU2F認証を、Chromeブラウザーの標準機能で実行するために必要なプログラムです。

- [macOS版U2F Helper](U2FMaintenanceTool/macOSApp/U2FHelper.pkg)
- [Windows版U2F Helper](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

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
