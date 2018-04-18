# One Card FIDO対応

One CardにFIDO U2F認証機能を実装するプロジェクトです。

## What's new

#### 2018/04/18

以下のプログラムを修正しました。<br>
（Version 0.1.0）

- [BLE U2Fサービス](nRF5_SDK_v13.0.0)
- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)
- [Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

修正点は以下になります。
- nRF52において、NRF_ERROR_INVALID_STATEにおけるレスポンス実行を回避（[Issue #40](https://github.com/diverta/onecard-fido/issues/40)）
- U2F管理ツールのバージョンが確認できるようにする（[Issue #43](https://github.com/diverta/onecard-fido/issues/43)）

本リリースから便宜的にバージョン番号を付与させていただきたく存じます。

#### 2018/04/10

nRF52側のFIDO機能でエラー発生時、発生箇所／原因がU2F管理ツールのログを参照して特定できるよう、以下のプログラムを修正しました。

- [BLE U2Fサービス](nRF5_SDK_v13.0.0)
- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)
- [Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

あわせて、[FIDO機能のステータスワード一覧](nRF5_SDK_v13.0.0/FIDOSWLIST.md)を別途まとめました。

#### 2018/04/03

[macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)を更新しました。<br>
以下の不具合を修正しております。
- U2F Authenticate実行後、Chromeブラウザーがハング (Issue #25)
- 再起動したChromeブラウザーで、U2F Authenticateが再度失敗 (Issue #26)
- BLEリクエストが送信されないことがある (Issue #27)
- BLEリクエストは送信されるが、２件目以降のフレームが受信されないことがある (Issue #28)
- macOSで「Bluetooth: Off」時にChromeブラウザーがハング (Issue #30)
- U2F Registerでエラーレスポンスを受信時、Chromeブラウザーがハング (Issue #32)

これに伴い、[U2Fデモサイト用のChromeエクステンション](U2FMaintenanceTool/u2f-chrome-extension.zip)も同時に更新しました。

#### 2018/03/15

[Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)を更新しました。<br>
秘密鍵・自己署名証明書の作成から、One Cardへのインストールまでの作業を、Windows版U2F管理ツールを使って出来るようになりました。<br>
手順書は以下になります。
- [One Cardペアリング手順](Usage/PAIRING.md)<br>
One Cardに、BLEを使用して秘密鍵・証明書をインストールするためには、まず最初にペアリングが必要になります。

- [鍵・証明書インストール手順](Usage/INSTALL_WINDOWS.md)<br>
秘密鍵・自己署名証明書の作成およびインストール手順についての説明になります。

- [U2Fデモサイトを使ったテスト手順](Usage/DEMOSITETEST.md)<br>
[U2Fデモサイト](https://crxjs-dot-u2fdemo.appspot.com/)を使用して、One CardのU2F機能をテストする手順です。

#### 2018/03/08

[OpenSSL未導入のmacOSで、U2F管理ツールが起動時にクラッシュしてしまう障害](https://github.com/diverta/onecard-fido/issues/20)を解消いたしました。

#### 2018/03/07

[macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)を更新しました。<br>
秘密鍵・自己署名証明書の作成から、One Cardへのインストールまでの作業を、macOS版U2F管理ツールを使って出来るようになりました。<br>
手順書は以下になります。
- [One Cardペアリング手順](Usage/PAIRING.md)<br>
One Cardに、BLEを使用して秘密鍵・証明書をインストールするためには、まず最初にペアリングが必要になります。

- [鍵・証明書インストール手順](Usage/INSTALL.md)<br>
秘密鍵・自己署名証明書の作成およびインストール手順についての説明になります。<br>
（以前の手順書にあった、nrfutilやOpenSSL等の追加インストールは、不要になりました）

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
