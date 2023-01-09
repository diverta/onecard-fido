# PGP鍵インストール手順書

最終更新日：2022/11/15

## 概要

[FIDO認証器管理ツール](../../MaintenanceTool/macOSApp/MNTTOOL.md)を使用して、[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)にPGP鍵をインストールする手順について掲載します。

## ソフトウェアの準備

PGP鍵のインストールを実行するためには、管理ツール、ファームウェア共に、必要バージョン以降である必要があります。<br>
また、[GPG Suite](https://gpgtools.org)というツールを、PCに別途インストールする必要があります。

#### 管理ツールのバージョン確認
まずは[インストール手順](../../MaintenanceTool/macOSApp/INSTALLPRG.md)を参照し、管理ツールをmacOS環境にインストールします。<br>
次に、下記手順で管理ツールのバージョン確認を行い、<b>Version 0.1.39以降</b>であるかどうか確認します。

管理ツールのユーティリティー画面で「管理ツールのバージョンを参照」をクリックします。

<img src="assets07/0000.jpg" width="400">

表示される画面で、管理ツールのバージョンを確認してください。<br>
（下記例では「Version 0.2.2」となっております）

<img src="assets07/0001.jpg" width="400">

#### ファームウェアのバージョン確認
続いて、下記手順でMDBT50Q Dongleファームウェアのバージョン確認を行い、<b>0.3.4以降</b>であるかどうか確認します。<br>
MDBT50Q DongleをPCのUSBポートに装着した後、管理ツールのユーティリティー画面で「ファームウェアのバージョンを参照」をクリックします。

<img src="assets05/0031.jpg" width="400">

管理ツール下部のメッセージ欄に表示される、ファームウェアのバージョンを確認してください。<br>
（下記例では「0.3.6」となっております）

<img src="assets07/0002.jpg" width="400">

#### GPG Suiteのインストール

管理ツールは、PGP鍵インストール処理時、[GPG Suite](https://gpgtools.org)というツールに同梱の「MacGPG2」を管理ツールで内部利用するため、あらかじめ<b>GPG SuiteがPCにインストールされている</b>必要があります。

<img src="../../CCID/OpenPGP/assets01/0001.jpg" width="400">

GPG Suiteのインストール手順につきましては、別ドキュメント「<b>[GPG Suiteインストール手順書](../../CCID/OpenPGP/GPGINSTMAC.md)</b>」をご参照願います。

## 管理ツールの起動

PGP秘密鍵のインストールは「OpenPGP設定画面」上から行います。

管理ツールを起動し、USBポートにMDBT50Q Dongleを装着します。<br>
管理ツール画面下部のメッセージ欄に「USB HIDデバイスに接続されました。」と表示されることを確認したら、管理ツール画面の「OpenPGP設定」ボタンをクリックします。

<img src="assets07/0003.jpg" width="400">

ホーム画面の上に、OpenPGP設定画面がポップアップ表示されます。

<img src="assets07/0004.jpg" width="400">

## PGP秘密鍵のインストール

OpenPGP機能で使用するPGP秘密鍵を、BT40 Dongleに導入します。

導入が必要な秘密鍵は、以下の３セットになります。<br>
いずれも、本機能で自動生成されます。

- 電子署名用（Signature key）
- 暗号／復号化用（Encryption key）
- PGP認証用（Authentication key）

以下の手順により、３セットの秘密鍵をすべてインストールします。

#### インストール手順

まずはインストールする鍵の個人情報を入力します。<br>
全項目入力必須になります。

- 名前（５文字以上で入力します）
- メールアドレス
- コメント

<img src="assets07/0005.jpg" width="400">

次に、各種ファイルの出力先フォルダーを選択します。<br>
「PGP公開鍵」欄右側の「参照」ボタンをクリックします。

<img src="assets07/0006.jpg" width="400">

フォルダー参照ダイアログから、該当の出力先フォルダーを選択し「選択」ボタンをクリックします。

<img src="assets07/0007.jpg" width="400">

フォルダー欄に、選択された出力先フォルダーのパスが表示されます。<br>
（長いフォルダー名の場合は、マウスカーソルを上から当てると、フルパス名称が小さくToolTip表示されます）

<img src="assets07/0008.jpg" width="400">

同様に、バックアップファイルの出力先フォルダーも選択します。

<img src="assets07/0009.jpg" width="400">

PGP公開鍵、バックアップ両方の出力先フォルダーを選択したら、下部の認証情報欄に、OpenPGP機能で使用する管理用PIN番号を入力します。[注1]

PIN番号を入力したら、下部の「PGP秘密鍵のインストール」ボタンをクリックします。

<img src="assets07/0010.jpg" width="400">

下記のような確認ダイアログが表示されますので、Yesボタンをクリックします。

<img src="assets07/0011.jpg" width="400">

PGP秘密鍵のインストール処理が実行されます。

<img src="assets07/0012.jpg" width="400">

程なく、下図のようなメッセージがポップアップ表示され、処理が完了します。

<img src="assets07/0013.jpg" width="400">

[注1] 管理用PIN番号は初期状態では「`12345678`」となっております。変更したい場合は、別ドキュメント「[OpenPGP機能の各種設定手順](../../MaintenanceTool/macOSApp/PGPSETTING_OPT.md)」をご参照願います。

#### 確認手順

インストールされた証明書は「OpenPGP設定情報画面」で確認できます。<br>
OpenPGP設定画面の左下部のボタン「設定情報を参照」をクリックします。

<img src="assets07/0014.jpg" width="400">

下図のようなOpenPGP設定情報画面がポップアップ表示されます。<br>
以下の３点が設定されていることが確認できます。

- 電子署名用（Signature key）の設定情報
- 暗号／復号化用（Encryption key）の設定情報
- PGP認証用（Authentication key）の設定情報

<img src="assets07/0015.jpg" width="400">

以上で、PGP秘密鍵のインストールは完了です。
