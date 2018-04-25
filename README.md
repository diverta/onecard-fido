# One Card FIDO対応

One CardにFIDO U2F認証機能を実装するプロジェクトです。

## What's new

#### 2018/04/25（Version 0.1.1）

以下のプログラムを修正しました。<br>

- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)

修正点は以下になります。
- ディスカバー・タイムアウトを検知できるようにする（[Issue #29](https://github.com/diverta/onecard-fido/issues/29)）
- 完了通知前にBLE接続を切断させるようにする（[Issue #36](https://github.com/diverta/onecard-fido/issues/36)）
- アプリ自身による切断時、接続リトライが行われないようにする（[Issue #37](https://github.com/diverta/onecard-fido/issues/37)）
- 予期しない切断発生時のタイムアウト誤検知を抑止する（[Issue #46](https://github.com/diverta/onecard-fido/issues/46)）
- スキャンタイムアウト時にスキャンを停止させるようにする（[Issue #47](https://github.com/diverta/onecard-fido/issues/47)）
- 接続リトライ上限到達時にメッセージを表示させるようにする（[Issue #48](https://github.com/diverta/onecard-fido/issues/48)）

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
