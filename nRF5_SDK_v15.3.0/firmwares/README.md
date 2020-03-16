# FIDO2認証器ファームウェア

## 概要
[FIDO2アプリケーション](../README.md)で使用するファームウェアです。

## ファイル

このフォルダー（`firmwares`）に格納しています。

以下のファームウェアは、Nordic社から提供されているアプリ「nRF Connect」を使い、MDBT50Q DongleやnRF52840 DKにダウンロードするようにします。<br>
「nRF Connect」を使用したダウンロード手順は、別ドキュメント<b>「[アプリケーション書込み手順](../../nRF5_SDK_v15.3.0/APPINSTALL.md)」</b>をご参照願います。<br>

| # |ファイル名 |説明 |
|:-:|:-|:-|
|1|[s140_nrf52_6.1.1_softdevice.hex](s140_nrf52_6.1.1_softdevice.hex)|ソフトデバイス|
|2|[nrf52840_xxaa.hex](nrf52840_xxaa.hex)|アプリケーション（MDBT50Q Dongle用）|
|3|[nrf52840_DK_xxaa.hex](nrf52840_DK_xxaa.hex)|アプリケーション（nRF52840 DK用）|

#### 管理ツール同梱用 ファームウェア更新イメージ

- [app_dfu_package.0.2.5.zip](app_dfu_package.0.2.5.zip)<br>
管理ツールに同梱され、管理ツールに搭載予定の[ファームウェア更新機能](../../MaintenanceTool/macOSApp/DFUFUNC.md)でダウンロードされることを前提としております。<br>
（【開発中】次期バージョンMDBT50Q Dongleに導入予定の[USBブートローダー](../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader)を経由して書き込みます）<br>
前述「[nrf52840_xxaa.hex](nrf52840_xxaa.hex)」を、[NetBeansプロジェクト](../../nRF5_SDK_v15.3.0/examples/diverta)でビルドすると、このファイルが自動生成されるようになっております。

## サブフォルダー

以下のブートローダーを格納しております。

- <b>[簡易USBブートローダー](open_bootloader)</b>（`firmwares/open_bootloader`）<br>
[MDBT50Q Dongle（rev2）](../../FIDO2Device/MDBT50Q_Dongle/README.md)に導入済みのブートローダーです。

- <b>[USBブートローダー（署名機能付き）](secure_bootloader)</b>（`firmwares/secure_bootloader`）<br>
【開発中】MDBT50Q Dongle（次期バージョン）に導入予定のブートローダーです。
