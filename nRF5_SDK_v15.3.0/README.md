# nRF52840版 FIDO2アプリケーション

## 概要
FIDO U2F／WebAuthn（CTAP2）の仕様に準拠したUSB HID／BLEアプリケーションです。

- PCにおけるユーザー登録／ログイン実行時[注1]：[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)をUSBポートに挿すと、nRF52840のUSB HIDサービスを経由して処理が行われます。

- Androidにおけるログイン実行時[注1]：[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)にボタン電池を装着し、USBポートに接続しない状態で使用すると、nRF52840のBLEサービスを経由してログイン処理が行われます。[注2][注3]

[注1] U2Fではユーザー登録＝Register、ログイン＝Authenticate、WebAuthnではユーザー登録＝MakeCredential、ログイン＝GetAssertionと、実行されるコマンドが異なります。<br>
[注2] nRF52840 DKの「nRF USB端子」をUSBポートに接続しない状態で使用すると、同じく、nRF52840のBLEサービスを経由してログイン処理が行われます。
[注3] 2019/05/07現在、Chrome Android上ではU2F Authenticateコマンドのみのサポートとなっているようです。

FIDO U2F／CTAP2に関する情報 : https://fidoalliance.org/download/

## ハードウェア

[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)＋PCのUSBポートの組み合わせによるWebAuthn操作を想定しています。[注1]

ただし、nRF52840 DK（開発用ボード）でも動作させることが可能です。[注2]<br>
この場合は、J-Link（プログラミングで使用するUSBポート）経由のUARTにより、デバッグログを表示させることができます。

[注1] PCのUSBポートに装着するとUSB HIDデバイスとして動作しますが、ボタン電池を装着し、USBポートに接続しない状態で使用すると、BLEセントラルデバイスとして稼働するよう実装されています。<br>
[注2] nRF USB端子をUSBケーブル経由でPCに接続した場合はUSB HIDデバイス、未接続の場合はBLEペリフェラルデバイスとして動作します（プログラムにより自動的に切り替えが行われます）。<br>

## ファームウェア

「nRF52840版 FIDO2アプリケーション」で使用するファームウェアは下記の２本になります。<br>
フォルダー[`firmwares`](firmwares/README.md) に格納しています。

- s140_nrf52_6.1.1_softdevice.hex - ソフトデバイス

- nrf52840_xxaa.hex - アプリケーション（MDBT50Q Dongle用）

- nrf52840_DK_xxaa.hex - アプリケーション（nRF52840 DK用）

## 開発環境構築手順

以下の手順書をご参照願います。

- <b>[NetBeansインストール手順](NETBEANSINST.md)</b>

## 動作確認手順

[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)の関連ドキュメントをご参照願います。
