# FIDO2認証器ファームウェア

## 概要
[FIDO2アプリケーション](../README.md)で使用するファームウェアです。

## ファイル

このフォルダー（`firmwares`）に格納しています。

- [s140_nrf52_6.1.1_softdevice.hex](s140_nrf52_6.1.1_softdevice.hex) - ソフトデバイス

- [nrf52840_xxaa.hex](nrf52840_xxaa.hex) - アプリケーション（MDBT50Q Dongle用）

- [nrf52840_DK_xxaa.hex](nrf52840_DK_xxaa.hex) - アプリケーション（nRF52840 DK用）

これらのファームウェアは、Nordic社から提供されているアプリ「nRF Connect」を使い、MDBT50Q DongleやnRF52840 DKにダウンロードするようにします。<br>
「nRF Connect」を使用したダウンロード手順は、別ドキュメント<b>「[アプリケーション書込み手順](../../FIDO2Device/MDBT50Q_Dongle/APPINSTALL.md)」</b>をご参照願います。<br>

## サブフォルダー

以下のブートローダーを格納しております。

- <b>[簡易USBブートローダー](open_bootloader)</b>（`firmwares/open_bootloader`）<br>
[MDBT50Q Dongle（rev2）](../../FIDO2Device/MDBT50Q_Dongle/README.md)に導入済みのブートローダーです。

- <b>[USBブートローダー（署名機能付き）](secure_bootloader)</b>（`firmwares/secure_bootloader`）<br>
【開発中】MDBT50Q Dongle（次期バージョン）に導入予定のブートローダーです。
