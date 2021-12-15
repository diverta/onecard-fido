# MDBT50Q Dongle用ファームウェア

[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)で使用するファームウェアです。

## ファームウェアの種類

以下の３点を格納しています。<br>
いずれも、[nRF5 SDK v17.1.0](https://infocenter.nordicsemi.com/topic/struct_sdk/struct/sdk_nrf5_latest.html)を利用して制作されています。

### [nRF52840アプリケーション](../../nRF52840_app/firmwares/secure_device_app/README.md)

MDBT50Q Dongleで使用するアプリケーション・ファームウェアです。<br>
サブフォルダー（[`secure_device_app`](../../nRF52840_app/firmwares/secure_device_app)）に格納しています。

### [USBブートローダー（署名機能付き）](../../nRF52840_app/firmwares/secure_bootloader/README.md)

MDBT50Q Dongle（rev2以降のバージョン）に対し、[管理ツール](../../MaintenanceTool)から[nRF52840アプリケーション](../../nRF52840_app/firmwares/secure_device_app/README.md)を書込めるようにするためのファームウェアです。<br>
サブフォルダー（[`secure_bootloader`](../../nRF52840_app/firmwares/secure_bootloader)）に格納しています。

### [サンプルアプリケーション](../../nRF52840_app/firmwares/sample_blehrs/README.md)

MDBT50Q Dongleの初期動作確認用に使用するサンプルのBLEアプリケーションです。<br>
サブフォルダー（[`sample_blehrs`](../../nRF52840_app/firmwares/sample_blehrs)）に格納しています。
