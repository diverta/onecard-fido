# MDBT50Q Dongle用ファームウェア

[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)で使用するファームウェアです。

## nRF52840アプリケーション

このフォルダー（`firmwares`）に格納しています。

| # |ファイル名 |説明 |
|:-:|:-|:-|
|1|`appkg.PCA10059_01.nn.nn.nn.zip`|[MDBT50Q Dongle（rev2）](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2/README.md)専用ファームウェア更新イメージファイル|
|2|`appkg.PCA10059_02.nn.nn.nn.zip`|[MDBT50Q Dongle（rev2.1.2）](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/README.md)専用ファームウェア更新イメージファイル|

#### ファームウェア更新イメージ

[管理ツール](../../MaintenanceTool)に同梱され、管理ツールの[「ファームウェア更新機能」](../../MaintenanceTool/macOSApp/UPDATEFIRMWARE.md)でダウンロードされることを前提としております。<br>
基板名「`PCA10059_01`」または「`PCA10059_02`」を指定し、NetBeansでビルドすると、このファイルが自動生成されるようになっております。

なお、`appkg.<基板名>.nn.nn.nn.zip`の`nn.nn.nn`は、バージョン番号になります。<br>
例えば、MDBT50Q Dongle rev2.1.2（基板名`PCA10059_02`）、バージョン`0.2.13`のファームウェア更新イメージファイル名は、`appkg.PCA10059_02.0.2.13.zip`となります。

#### [nRF52840アプリケーション作成手順書](../../nRF52840_app/firmwares/secure_device_app/BUILDAPP.md)
NetBeansとARM GCC、nRF5 SDKを使用し、[Nordic社提供のサンプルアプリケーション](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/ble_sdk_app_hrs.html)を土台に、nRF52840アプリケーションを作成する手順を記載しています。

#### [nRF52840アプリケーション初回導入手順書](../../nRF52840_app/firmwares/secure_device_app/WRITEAPP.md)
MDBT50Q Dongleに、nRF52840アプリケーションを<b>新規に書き込む</b>手順を記載しています。

## USBブートローダー（署名機能付き）

[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)（rev2以降のバージョン）に対し、[管理ツール](../../MaintenanceTool)から[nRF52840アプリケーション](../../nRF52840_app/README.md)を書込めるようにするためのファームウェアです。<br>
サブフォルダー（`firmwares/secure_bootloader`）に格納しています。

| # |ファイル名 |説明 |
|:-:|:-|:-|
|1|`nrf52840_xxaa_PCA10059_01.hex`|[MDBT50Q Dongle（rev2）](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2/README.md)専用ブートローダー|
|2|`nrf52840_xxaa_PCA10059_02.hex`|[MDBT50Q Dongle（rev2.1.2）](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/README.md)専用ブートローダー|

#### [USBブートローダー（署名機能付き）作成手順書](../../nRF52840_app/firmwares/secure_bootloader/BUILDSBL.md)
NetBeansとARM GCC、nRF5 SDKを使用し、ブートローダーを作成する手順を記載しています。

#### [USBブートローダー書込み手順書](../../nRF52840_app/firmwares/secure_bootloader/WRITESBL.md)
MDBT50Q Dongleに、ブートローダー本体／ソフトデバイス／アプリケーション・ファームウェアをセット導入する手順を記載しています。

## サンプルアプリケーション

[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)の初期動作確認用に使用するサンプルのBLEアプリケーションです。<br>
サブフォルダー（`firmwares/sample_blehrs`）に格納しています。

| # |ファイル名 |説明 |
|:-:|:-|:-|
|1|`appkg.PCA10059_01.zip`|[MDBT50Q Dongle（rev2）](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2/README.md)専用ファームウェア更新イメージファイル|
|2|`appkg.PCA10059_02.zip`|[MDBT50Q Dongle（rev2.1.2）](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/README.md)専用ファームウェア更新イメージファイル|

#### [サンプルアプリケーション作成手順書](../../nRF52840_app/firmwares/sample_blehrs/BUILDHRS.md)
NetBeansとARM GCC、nRF5 SDKを使用し、動作確認用のサンプルアプリケーション「Heart Rate Application」を作成する手順を記載しています。

#### [サンプルアプリケーション動作確認手順書](../../nRF52840_app/firmwares/sample_blehrs/WRITEHRS.md)
MDBT50Q Dongleに「Heart Rate Application」を新規導入し、Androidアプリを使用して動作確認する手順を記載しています。
