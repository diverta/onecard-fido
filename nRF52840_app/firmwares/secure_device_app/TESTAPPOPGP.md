# OpenPGP機能動作確認手順書

## 概要

[GPGツール](https://gpgtools.org)を使用し、[nRF52840アプリケーション](../../../nRF52840_app/firmwares/secure_device_app)のOpenPGP機能に関する動作確認を行う手順について、以下に掲載いたします。

## 作業の準備

動作確認の前に、以下の項目について準備します。

#### MDBT50Q Dongleの準備

[version 0.3.1](https://github.com/diverta/onecard-fido/blob/doc-20210311/nRF52840_app/firmwares/secure_device_app)以降の[nRF52840アプリケーション](../../../nRF52840_app/firmwares/secure_device_app)が書き込まれたMDBT50Q Dongleを、あらかじめ準備します。<br>
具体的な手順は、別ドキュメント「[nRF52840アプリケーション更新手順書](../../../nRF52840_app/firmwares/secure_device_app/UPDATEAPP.md)」をご参照ください。[注1]<br>

MDBT50Q Dongleを、PCのUSBポートに装着すると、下図のように、基板上の緑色のLEDが点滅している状態になります。

<img src="../../../nRF52840_app/firmwares/secure_device_app/assets01/0010.jpg" width="150">

[注1] 導入されているnRF52840アプリケーションが、Version 0.3.0より前の場合は、別ドキュメント「[MDBT50Q Dongleファームウェア（version 0.3.0）導入手順書](../../../nRF52840_app/firmwares/secure_device_app/WRITEAPP_0_3_0.md)」をご参照願います。

#### CCIDドライバーのインストール

まず最初に、macOSにCCIDドライバーをインストールします。<br>
具体的な手順は、別ドキュメント「[CCIDドライバーインストール手順](../../../CCID/INSTALLPRG.md)」をご参照ください。

#### GPGツール群のインストール

GPGツール群「GPG Suite」をPCにインストールしておきます。<br>
具体的な手順は、別ドキュメント<b>「[GPG Suiteインストール手順](../../../CCID/OpenPGP/GPGINSTMAC.md)」</b>をご参照ください。

MDBT50Q DongleがPCのUSBポートに装着されている状態だと、GPGのステータス照会コマンドで接続状況が確認できます。

```
bash-3.2$ gpg --card-status
Reader ...........: Diverta Inc. Secure Dongle
Application ID ...: D276000124010304F1D0000000000000
Application type .: OpenPGP
Version ..........: 3.4
Manufacturer .....: unknown
Serial number ....: 00000000
Name of cardholder: Here is the cardname
：
bash-3.2$
```

#### 秘密鍵のインストール

本プロジェクトでは、秘密鍵の管理を、GPGツール群に委ねています。<br>
すなわち、GPGツールで秘密鍵を生成後、その秘密鍵をMDBT50Q Dongleにインストール（移動）する、といった手順になります。[注1]

GPGツールを使用した秘密鍵インストールの手順につきましては、別ドキュメント<b>「[GPG Suiteによる鍵インストール手順](../../../CCID/OpenPGP/GPGKEYINST.md)」</b>をご参照願います。[注2]

[注1] GPGツールで生成した秘密鍵は、MDBT50Q Dongleにインストール（移動）後、２度と取り出すことが出来ないようになっております。<br>
[注2] この手順はコマンドラインベースで非常に手間がかかるため、将来的に、[FIDO認証器管理ツール](../../../MaintenanceTool/README.md)のような専用GUIアプリを制作し、作業を簡略化させる方向で検討しております。

#### ファイルの暗号化／復号化

GPGツールで生成した秘密鍵／公開鍵により、ファイルの暗号化／復号化ができます。

ファイルの暗号化は、公開鍵を使って実行します。<br>
他方、その暗号されたファイルの復号化を実行する際は、前項にてMDBT50Q Dongleにインストールされた秘密鍵が必要となります。

GPGツールを使用したファイル暗号化／復号化の手順につきましては、別ドキュメント<b>「[OpenPGPを使用したファイル暗号／復号化手順](../../../CCID/OpenPGP/OPGPCRYPTION.md)」</b>をご参照願います。[注2]
