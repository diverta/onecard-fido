# One Card FIDO対応

One CardにFIDO U2F認証機能を実装するプロジェクトです。

## What's new

#### 2018/09/27（Version 0.1.6）

以下のプログラムを修正しました。<br>

- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)
- [Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

修正点は以下になります。
- U2F Helper制作にともない、不要となった機能「Chrome設定」(Chrome Native Messaging) を、U2F管理ツール画面から削除
- C++で制作したWIndows版U2F管理コマンド（U2FMaintenanceToolCMD.exe）を、C#の画面アプリ内に移植しました（Issue #72）

U2F Helper（ヘルパーアプリ）は、別途製作した[U2F USB HIDデバイス（ヘルパーデバイス）](U2FHIDDevice/readme.md)と連携し、One Cardを使ったU2F認証を、Chromeブラウザーの標準機能で実行するために必要なプログラムです。

- [macOS版U2F Helper](U2FMaintenanceTool/macOSApp/U2FHelper.pkg)
- [Windows版U2F Helper](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

具体的な使用方法は、別途手順書[「Googleアカウントを使ったテスト手順」](Usage/GOOGLEACCTEST.md)をご参照ください。

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
