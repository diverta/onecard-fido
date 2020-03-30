# USBブートローダー（署名機能付き）

## 概要
【開発中】MDBT50Q Dongle（次期バージョン）に導入予定のブートローダーです。

## ファイル

このフォルダー（`firmwares/secure_bootloader`）に格納しています。

- [nrf52840_xxaa.hex](nrf52840_xxaa.hex) - ブートローダー本体<br>
後述「ファームウェア作成手順」により作成したものを使用します。

## [ファームウェア作成手順](../../../nRF5_SDK_v15.3.0/examples/dfu/secure_bootloader/README.md)

NetBeansとARM GCC、nRF5 SDKを使用し、ブートローダーを作成する手順を記載しています。

## [ファームウェア書込手順](SB_JLINKSWDPROG.md)

MDBT50Q Dongleに、ブートローダー本体／ソフトデバイスをJ-Link経由で書込みする手順を記載しています。
