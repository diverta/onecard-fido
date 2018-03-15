# One Card FIDO対応

One CardにFIDO U2F認証機能を実装するプロジェクトです。

## What's new

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
