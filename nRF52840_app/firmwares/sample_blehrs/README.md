# サンプルアプリケーション

[MDBT50Q Dongle](../../../FIDO2Device/MDBT50Q_Dongle/README.md)の初期動作確認用に使用するサンプルのBLEアプリケーションです。

## ファームウェア更新イメージファイル

このフォルダー（`firmwares/sample_blehrs`）に格納しています。

| # |ファイル名 |説明 |
|:-:|:-|:-|
|1|`appkg.PCA10059_01.zip`|[MDBT50Q Dongle（rev2）](../../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2/README.md)専用ファームウェア更新イメージファイル|
|2|`appkg.PCA10059_02.zip`|[MDBT50Q Dongle（rev2.1.2）](../../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/README.md)専用ファームウェア更新イメージファイル|

## 各種手順書

#### [サンプルアプリケーション作成手順書](../../../nRF52840_app/firmwares/sample_blehrs/BUILDHRS.md)
NetBeansとARM GCC、nRF5 SDKを使用し、動作確認用のサンプルアプリケーション「Heart Rate Application」を作成する手順を記載しています。

#### [サンプルアプリケーション動作確認手順書](../../../nRF52840_app/firmwares/sample_blehrs/WRITEHRS.md)
MDBT50Q Dongleに「Heart Rate Application」を新規導入し、Androidアプリを使用して動作確認する手順を記載しています。
