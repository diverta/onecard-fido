# nRF52840アプリケーション

最終更新日：2023/1/11

## 概要

[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)（nRF52840）上で稼働するアプリケーションです。<br>
[nRF5 SDK v17.1.0](https://infocenter.nordicsemi.com/topic/struct_sdk/struct/sdk_nrf5_latest.html)を使用して開発されています。

## 搭載機能

### FIDO2認証機能

FIDO U2F／WebAuthn（CTAP2）の仕様に準拠したUSB HID／BLEアプリケーションです。[注1]

- <b>PCにおけるユーザー登録／ログイン実行時</b> [注2]<br>
[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)をUSBポートに挿すと、nRF52840のUSB HIDサービスを経由して処理が行われます。

- <b>Androidにおけるログイン実行時</b> [注2]<br>
[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)にボタン電池を装着し、USBポートに接続しない状態で使用すると、nRF52840のBLEサービスを経由してログイン処理が行われます。[注3]

[注1] FIDO U2F／CTAP2に関する情報 : https://fidoalliance.org/download/ <br>
[注2] U2Fではユーザー登録＝Register、ログイン＝Authenticate、WebAuthnではユーザー登録＝MakeCredential、ログイン＝GetAssertionと、実行されるコマンドが異なります。<br>
[注3] 2019/05/07現在、Chrome Android上ではU2F Authenticateコマンドのみのサポートとなっているようです。

### PIVカードエミュレーション機能

PIVカードの仕様に準拠したUSB CCIDアプリケーションです。<br>
詳細につきましては、別ドキュメント（[PIVカードエミュレーション対応](../CCID/PIVCARDEMUL.md)）をご参照願います。

### OpenPGPカードエミュレーション機能

OpenPGPカードの仕様に準拠したUSB CCIDアプリケーションです。<br>
詳細につきましては、別ドキュメント（[OpenPGPカードエミュレーション対応](../CCID/OpenPGP/README.md)）をご参照願います。

## ファームウェア

nRF52840アプリケーションのファームウェアは、フォルダー[`firmwares`](firmwares) に格納しています。<br>
（詳細は別ドキュメント<b>「[MDBT50Q Dongle用ファームウェア](../nRF52840_app/firmwares/README.md)」</b>をご参照）

## 開発環境構築手順

以下の手順書をご参照願います。

- <b>[ARM GCCインストール手順](../nRF52840_app/ARMGCCINST.md)</b>

- <b>[CMakeインストール手順](../nRF5340_app/INSTALLCMAKE.md)</b>

- <b>[nRF5 SDKインストール手順](../nRF52840_app/NR5SDKINST.md)</b>

- <b>[nRF Command Line Toolsインストール手順](../nRF52840_app/NRFCLTOOLINST.md)</b>

- <b>[NetBeansインストール手順](../nRF52840_app/NETBEANSINST.md)</b>

- <b>[nRF Utilインストール手順](../nRF52840_app/NRFUTILINST.md)</b>

- <b>[nRF Connect for Desktop導入手順](../nRF52840_app/NRFCONNECTINST.md)</b>

- <b>[tinycbor導入手順](../nRF52840_app/firmwares/secure_device_app/TINYCBOR.md)</b>
