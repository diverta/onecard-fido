# USBブートローダー（署名機能付き）

[MDBT50Q Dongle](../../../FIDO2Device/MDBT50Q_Dongle/README.md)（rev2以降のバージョン）に対し、[管理ツール](../../../MaintenanceTool)から[nRF52840アプリケーション](../../../nRF52840_app/firmwares/secure_device_app/README.md)を書込めるようにするためのファームウェアです。

## ファームウェア更新イメージ

このフォルダー（`firmwares/secure_bootloader`）に格納しています。

| # |ファイル名 |説明 |
|:-:|:-|:-|
|1|`nrf52840_xxaa_PCA10059_01.hex`|[MDBT50Q Dongle（rev2）](../../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2/README.md)専用ブートローダー|
|2|`nrf52840_xxaa_PCA10059_02.hex`|[MDBT50Q Dongle（rev2.1.2）](../../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/README.md)専用ブートローダー|

## 各種手順書

#### [USBブートローダー（署名機能付き）作成手順書](../../../nRF52840_app/firmwares/secure_bootloader/BUILDSBL.md)
NetBeansとARM GCC、nRF5 SDKを使用し、ブートローダーを作成する手順を記載しています。

#### [USBブートローダー書込み手順書](../../../nRF52840_app/firmwares/secure_bootloader/WRITESBL.md)
MDBT50Q Dongleに、ブートローダー本体／ソフトデバイス／アプリケーション・ファームウェアをセット導入する手順を記載しています。
