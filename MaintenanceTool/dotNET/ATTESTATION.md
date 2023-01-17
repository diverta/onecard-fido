# 鍵・証明書の導入手順

## 概要

[FIDO認証器管理ツール（ベンダー向け）](../../MaintenanceTool/dotNET/DEVTOOL.md)を使用して、FIDO2認証器に鍵・証明書を導入する手順を掲載します。

## 認証器の準備

#### 使用機材

本ドキュメントでは「[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)」を、FIDO2認証器として使用します。

<img src="../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/assets/0001.jpg" width="300">

#### 管理ツールを導入

[FIDO認証器管理ツール（ベンダー向け）](../../MaintenanceTool/dotNET/DEVTOOL.md)を、PC環境（Windows）に導入します。<br>
以下の手順書をご参照願います。

* <b>[FIDO認証器管理ツール（ベンダー向け） インストール手順](../../MaintenanceTool/dotNET/DEVTOOLINST.md) </b>

## 鍵・証明書ファイルのインストール

管理ツール（ベンダー向け）を起動し、USBポートにFIDO2認証器を装着します。

<img src="assets/0028.jpg" width="400">

管理ツール画面下部のメッセージ欄に「USB HIDデバイスに接続されました。」と表示されることを確認したら、管理ツールのメニューから「ベンダー向け機能」を選択します。

<img src="assets/0029.jpg" width="400">

ベンダー向け機能画面が表示されますので「FIDO鍵・証明書のインストール」ボタンをクリックし、FIDO鍵・証明書インストール画面を表示させます。

<img src="assets/0030.jpg" width="400">

秘密鍵ファイル（PEM形式）、証明書ファイル（DER形式）を、それぞれ「参照」ボタンをクリックして選択します。<br>
ファイル選択後、管理ツール画面の「鍵・証明書ファイルのインストール」ボタンをクリックします。

<img src="assets/0010.jpg" width="400">

確認ダイアログが表示されますので「Yes」をクリックします。

<img src="assets/0011.jpg" width="400">

FIDO認証器側の処理が成功すると「鍵・証明書インストールが成功しました。」と表示されます。

<img src="assets/0012.jpg" width="400">

以上で、鍵・証明書ファイルのインストールは完了です。

## 鍵・証明書の削除

いったん導入した鍵・証明書をFIDO2認証器から消去するには「鍵・証明書の削除」機能を使用します。<br>
先述のベンダー向け機能画面を表示してから「FIDO鍵・証明書の削除」ボタンをクリックします。

<img src="assets/0007.jpg" width="400">

確認ダイアログが表示されますので「Yes」をクリックします。

<img src="assets/0008.jpg" width="400">

FIDO認証器側の処理が成功すると「鍵・証明書の削除が成功しました。」と表示されます。

<img src="assets/0009.jpg" width="400">

以上で、鍵・証明書の削除は完了です。
