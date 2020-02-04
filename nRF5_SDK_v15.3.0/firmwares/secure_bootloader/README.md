# USBブートローダー（署名機能付き）

## 概要
【開発中】MDBT50Q Dongle（rev2.1以降のバージョン）に導入予定のブートローダーです。

## ファイル

このフォルダー（`firmwares/secure_bootloader`）に格納しています。

- [mdbt50q_dongle.hex](mdbt50q_dongle.hex) - ブートローダー本体<br>
後述「ファームウェア作成手順」により作成したものを使用します。

- [s140_nrf52_6.1.1_softdevice.hex](s140_nrf52_6.1.1_softdevice.hex) - ソフトデバイス<br>
Nordic社から提供されているものをそのまま使用します。

## [ファームウェア作成手順](../../../nRF5_SDK_v15.3.0/examples/dfu/secure_bootloader/README.md)

NetBeansとARM GCC、nRF5 SDKを使用し、ブートローダーを作成する手順を記載しています。

## [ファームウェア書込手順](SB_JLINKSWDPROG.md)

MDBT50Q Dongleに、ブートローダー本体／ソフトデバイスをJ-Link経由で書込みする手順を記載しています。
