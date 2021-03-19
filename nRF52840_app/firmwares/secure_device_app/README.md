# nRF52840アプリケーション

[MDBT50Q Dongle](../../../FIDO2Device/MDBT50Q_Dongle/README.md)で動作するアプリケーション・ファームウェアです。

## ファームウェア更新イメージ

このフォルダー（`firmwares/secure_device_app`）に格納しています。

| # |ファイル名 |説明 |
|:-:|:-|:-|
|1|`appkg.PCA10059_01.nn.nn.nn.zip`|[MDBT50Q Dongle（rev2）](../../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2/README.md)専用ファームウェア更新イメージファイル|
|2|`appkg.PCA10059_02.nn.nn.nn.zip`|[MDBT50Q Dongle（rev2.1.2）](../../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/README.md)専用ファームウェア更新イメージファイル|

[管理ツール](../../../MaintenanceTool)に同梱され、管理ツールの[「ファームウェア更新機能」](../../../MaintenanceTool/macOSApp/UPDATEFIRMWARE.md)でダウンロードされることを前提としております。<br>
基板名「`PCA10059_01`」または「`PCA10059_02`」を指定し、NetBeansでビルドすると、このファイルが自動生成されるようになっております。

なお、`appkg.<基板名>.nn.nn.nn.zip`の`nn.nn.nn`は、バージョン番号になります。<br>
例えば、MDBT50Q Dongle rev2.1.2（基板名`PCA10059_02`）、バージョン`0.3.0`のファームウェア更新イメージファイル名は、`appkg.PCA10059_02.0.3.0.zip`となります。

## 各種手順書

#### [nRF52840アプリケーション作成手順書](../../../nRF52840_app/firmwares/secure_device_app/BUILDAPP.md)
NetBeansとARM GCC、nRF5 SDKを使用し、[Nordic社提供のサンプルアプリケーション](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/ble_sdk_app_hrs.html)を土台に、nRF52840アプリケーションを作成する手順を記載しています。

#### [nRF52840アプリケーション初回導入手順書](../../../nRF52840_app/firmwares/secure_device_app/WRITEAPP.md)
MDBT50Q Dongleに、nRF52840アプリケーションを<b>新規に書き込む</b>手順を記載しています。

#### [nRF52840アプリケーション動作確認手順書](../../../nRF52840_app/firmwares/secure_device_app/TESTAPP.md)
管理ツールを使用し、nRF52840アプリケーションの動作確認を行う手順について記載しています。

#### [FIDO仕様適合テスト手順書](../../../FIDO2Device/CONFORMANCE.md)
FIDOアライアンスが無償提供しているテストツール「FIDO Conformance Tools」を使用し、FIDO仕様適合テストを実行する手順を掲載しています。
