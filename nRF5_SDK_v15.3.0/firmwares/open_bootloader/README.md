# 簡易USBブートローダー

## 概要
[MDBT50Q Dongle（rev2）](../../../FIDO2Device/MDBT50Q_Dongle/README.md)に導入済みのブートローダーです。

## ファイル

このフォルダー（`firmwares/open_bootloader`）に格納しています。

- [mbr_nrf52_2.4.1_mbr.hex](mbr_nrf52_2.4.1_mbr.hex) - マスターブートレコード<br>
Nordic社から提供されているものをそのまま使用しています。

- [nrf52840_xxaa.hex](nrf52840_xxaa.hex) - アプリケーション（USBブートローダー本体）<br>
後述「ファームウェア作成手順」により作成したものを使用します。

## [ファームウェア作成手順](../../../nRF5_SDK_v15.3.0/examples/dfu/open_bootloader/README.md)

NetBeansとARM GCC、nRF5 SDKを使用し、ブートローダーを作成する手順を記載しています。

## [ファームウェア書込手順](JLINKSWDPROG.md)

MDBT50Q Dongleに、ブートローダーをJ-Link経由で書込みする手順を記載しています。
