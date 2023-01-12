# CTAP2ヘルスチェック実行手順

## 概要

[FIDO認証器管理ツール](../../MaintenanceTool/dotNET/README.md)を使用して、FIDO2認証器のヘルスチェックを実行する手順を掲載します。

#### 処理内容

「CTAP2ヘルスチェック実行」は、管理ツールが擬似WebAuthnクライアント（Webブラウザーの代わり）となり、認証器に対して、FIDO 2.0の仕様に準拠したユーザー登録・ログインの各処理を実行する機能です。

CTAP2ヘルスチェックは、USB HID経由またはBLE経由で実行されます。<br>
このページでは、USB HID経由のヘルスチェックについて説明しています。

## 認証器の準備

#### 使用機材

本ドキュメントでは「[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)」を、FIDO2認証器として使用します。

<img src="../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/assets/0001.jpg" width="300">

#### 管理ツールを導入

[FIDO認証器管理ツール](../../MaintenanceTool/dotNET/README.md)を、PC環境（Windows）に導入します。<br>
以下の手順書をご参照願います。

* <b>[インストール手順（Windows版）](../../MaintenanceTool/dotNET/INSTALLPRG.md) </b>

#### ファームウェアを更新

最新ファームウェアを、FIDO2認証器に書込み、ファームウェアを更新します。<br>
（最新ファームウェアは、FIDO認証器管理ツールに同梱されています）<br>
以下の手順書をご参照願います。

* <b>[ファームウェア更新手順書](../../MaintenanceTool/dotNET/UPDATEFW_USB.md)</b>

#### PIN設定

PC環境に導入した管理ツールを使用し、PINコード（暗証番号）の設定をします。<br>
以下の手順書をご参照願います。

* <b>[PINコードの設定手順（Windows版）](../../MaintenanceTool/dotNET/SETPIN.md) </b>

## CTAP2ヘルスチェックの実行

管理ツールを起動し、USBポートにFIDO2認証器を装着します。

<img src="assets02/0011.jpg" width="400">

管理ツール画面下部のメッセージ欄に「USB HIDデバイスに接続されました。」と表示されることを確認したら、管理ツールのヘルスチェック実行画面で「USB > CTAP2ヘルスチェック実行」をクリックします。

<img src="assets02/0012.jpg" width="400">

PIN入力画面がポップアップ表示されますので、認証器に設定したPINコード（暗証番号）を入力し「OK」をクリックします。

<img src="assets02/0013.jpg" width="400">

ヘルスチェック処理が進み、ほどなく下図のようなメッセージが表示され、ユーザー所在確認が要求されます。

<img src="assets02/0014.jpg" width="400">

FIDO2認証器上の緑色LEDが点滅し始めますので、基板上のボタンを１回プッシュします。

<img src="../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/assets/0003.jpg" width="300">

ヘルスチェック処理が成功すると「CTAP2ヘルスチェックが成功しました。」というメッセージが表示されます。

<img src="assets02/0015.jpg" width="400">

これで、CTAP2ヘルスチェックの実行は完了です。
