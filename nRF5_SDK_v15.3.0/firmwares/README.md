# FIDO2認証器ファームウェア

## 概要
[FIDO2アプリケーション](../README.md)で使用するファームウェアです。

## ファイル

このフォルダー（`firmwares`）に格納しています。

| # |ファイル名 |説明 |
|:-:|:-|:-|
|1|[s140_nrf52_6.1.1_softdevice.hex](s140_nrf52_6.1.1_softdevice.hex)|ソフトデバイス|
|2|[nrf52840_xxaa.hex](nrf52840_xxaa.hex)|アプリケーション（MDBT50Q Dongle用）|
|3|[nrf52840_DK_xxaa.hex](nrf52840_DK_xxaa.hex)|アプリケーション（nRF52840 DK用）|
|4|`app_dfu_package.nn.nn.nn.zip`|管理ツール同梱用 ファームウェア更新イメージ|

#### 管理ツール同梱用 ファームウェア更新イメージ

管理ツールに同梱され、管理ツールの[ファームウェア更新機能](../../MaintenanceTool/macOSApp/DFUFUNC.md)でダウンロードされることを前提としております。<br>
前述「[nrf52840_xxaa.hex](nrf52840_xxaa.hex)」を、[NetBeansプロジェクト](../../nRF5_SDK_v15.3.0/examples/diverta)でビルドすると、このファイルが自動生成されるようになっております。

なお、`app_dfu_package.nn.nn.nn.zip`の`nn.nn.nn`は、バージョン番号になります。<br>
例えば、バージョン`0.2.8`のファームウェア更新イメージファイル名は、`app_dfu_package.0.2.8.zip`となります。

## サブフォルダー

以下のブートローダーを格納しております。

- <b>[簡易USBブートローダー](open_bootloader)</b>（`firmwares/open_bootloader`）<br>
[MDBT50Q Dongle（rev2）](../../FIDO2Device/MDBT50Q_Dongle/README.md)に導入済みのブートローダーです。

- <b>[USBブートローダー（署名機能付き）](secure_bootloader)</b>（`firmwares/secure_bootloader`）<br>
【開発中】MDBT50Q Dongle（次期バージョン）に導入予定のブートローダーです。
