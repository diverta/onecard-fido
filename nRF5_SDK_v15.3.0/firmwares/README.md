# FIDO2認証器ファームウェア

## 概要
[FIDO2アプリケーション](../README.md)で使用するファームウェアです。

## ファイル

このフォルダー（`firmwares`）に格納しています。

| # |ファイル名 |説明 |
|:-:|:-|:-|
|1|[`s140_nrf52_6.1.1_softdevice.hex`](s140_nrf52_6.1.1_softdevice.hex)|ソフトデバイス|
|2|[`nrf52840_DK_xxaa.hex`](nrf52840_DK_xxaa.hex)|アプリケーション（nRF52840 DK用）|
|3|`appkg.PCA10059.nn.nn.nn.zip`|[MDBT50Q Dongle（rev2）](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1/README.md)専用ファームウェア更新イメージ|
|4|`appkg.PCA10059_02.nn.nn.nn.zip`|[MDBT50Q Dongle（rev2.1.2）](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/README.md)専用ファームウェア更新イメージ|

#### ファームウェア更新イメージ

管理ツールに同梱され、管理ツールの[ファームウェア更新機能](../../MaintenanceTool/macOSApp/DFUFUNC.md)でダウンロードされることを前提としております。<br>
基板名「`PCA10059`」または「`PCA10059_02`」を指定し、[NetBeansプロジェクト](../../nRF5_SDK_v15.3.0/examples/diverta)でビルドすると、このファイルが自動生成されるようになっております。

なお、`appkg.<基板名>.nn.nn.nn.zip`の`nn.nn.nn`は、バージョン番号になります。<br>
例えば、MDBT50Q Dongle rev2.1.2（基板名`PCA10059_02`）、バージョン`0.2.11`のファームウェア更新イメージファイル名は、`appkg.PCA10059_02.0.2.11.zip`となります。

## サブフォルダー

以下のブートローダーを格納しております。

- <b>[簡易USBブートローダー](open_bootloader)</b>（`firmwares/open_bootloader`）<br>
[MDBT50Q Dongle（rev2）](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1/README.md)に当初導入したブートローダーです。

- <b>[USBブートローダー（署名機能付き）](secure_bootloader)</b>（`firmwares/secure_bootloader`）<br>
[MDBT50Q Dongle（rev2.1.2）](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/README.md)に導入済みのブートローダーです。
