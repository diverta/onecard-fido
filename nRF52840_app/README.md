# nRF52840アプリケーション

## 概要

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
詳細につきましては、別ドキュメント（[CCIDインターフェース追加対応](../CCID/README.md)）をご参照願います。

## ハードウェア

[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)＋PCのUSBポートの組み合わせによるWebAuthn操作等を想定しています。[注1]

[注1] PCのUSBポートに装着するとUSB HIDデバイスとして動作しますが、ボタン電池を装着し、USBポートに接続しない状態で使用すると、BLEセントラルデバイスとして稼働するよう実装されています。<br>

## ファームウェア

「nRF52840アプリケーション」のファームウェアは、フォルダー[`firmwares`](firmwares/README.md) に格納しています。

## 開発環境構築手順

以下の手順書をご参照願います。

- <b>[nRF5 SDKインストール手順](NR5SDKINST.md)</b>

## 動作確認手順

[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)の関連ドキュメントをご参照願います。
