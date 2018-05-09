# One Card FIDO対応

One CardにFIDO U2F認証機能を実装するプロジェクトです。

## What's new

#### 2018/05/09（Version 0.1.3）

以下のプログラムを修正しました。<br>

- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)
- [Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

修正点は以下になります。
- Windows版U2F管理コマンドのコードを整理し、Visual Studio 2015ソリューションを再構築しました。（[Pull request #51](https://github.com/diverta/onecard-fido/pull/51)）
- Windows版U2F管理ツールの処理中、画面がフリーズしないようにしました。（[Issue #52](https://github.com/diverta/onecard-fido/issues/52)）
- ヘルスチェック時、U2F Authenticateの所在確認待ちである旨をガイダンスさせるようにしました。（[Issue #55](https://github.com/diverta/onecard-fido/issues/55)）
- U2F管理ツールから、One Card側のペアリング情報を消去できないようにしました。（[Issue #56](https://github.com/diverta/onecard-fido/issues/56)）

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
