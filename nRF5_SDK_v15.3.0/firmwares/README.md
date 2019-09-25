# FIDO2認証器ファームウェア

## 概要
[FIDO2認証器](../README.md)で使用するファームウェアです。

## ファイル

このフォルダー（`firmwares`）に格納しています。

- [s140_nrf52_6.1.1_softdevice.hex](s140_nrf52_6.1.1_softdevice.hex) - ソフトデバイス

- [nrf52840_xxaa.hex](nrf52840_xxaa.hex) - アプリケーション（MDBT50Q Dongle用）

- [nrf52840_DK_xxaa.hex](nrf52840_DK_xxaa.hex) - アプリケーション（nRF52840 DK用）

これらのファームウェアは、Nordic社から提供されているアプリ「nRF Connect」を使い、MDBT50Q DongleやnRF52840 DKにダウンロードするようにします。<br>
「nRF Connect」を使用したダウンロード手順は、別ドキュメント<b>「[アプリケーション書込み手順](../../FIDO2Device/MDBT50Q_Dongle/APPINSTALL.md)」</b>をご参照願います。<br>
　
## 簡易USBブートローダー

サブフォルダー（`firmwares/open_bootloader`）に格納しています。

前述アプリ「nRF Connect」により、MDBT50Q Dongleに、USB経由でソフトデバイス／アプリケーションの書き込みをするために必要なファームウェアになります。<br>
詳細につきましては[ソースコードのドキュメント](../../nRF5_SDK_v15.3.0/examples/dfu/README.md)をご参照ください。
