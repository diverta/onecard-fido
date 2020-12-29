# PIN番号を使用したmacOSログイン確認手順

## 概要
[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)に設定した[PIV機能](../../CCID/ccid_lib/README.md)を使用し、PIN番号によるmacOSログインを行うための確認手順を掲載します。

## 準備手順

下記手順書により、MDBT50Q DongleでPIV機能が使えるように準備を行います。

#### [CCIDドライバーインストール手順](../../CCID/INSTALLPRG.md)
[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のCCIDインターフェース専用ドライバー（以下単に「CCIDドライバー」）を、macOS環境にインストールします。

#### [PIV機能の設定手順](../../MaintenanceTool/macOSApp/PIVSETTING.md)
管理ツールをmacOS環境に導入し、MDBT50Q DongleにPIV機能を設定します。<br>
リンク先ドキュメントの「管理ツールのインストール」〜「初期設定の実行」（ID設定の実行／鍵・証明書ファイルのインストール）の章をご参照願います。

## 作業手順

#### スマートカードペアリングの実行

[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)を、PC（MacBook、iMac等）のUSBポートに装着します。<br>
初回装着の場合、以下のようなダイアログが表示されます。

「PIV認証用証明書」を選択し「ペアリング」ボタンをクリックします。

<img src="../../Research/CCID/reference/assets01/0016.png" width="400">

下図のようなダイアログが表示されます。<br>
macOSユーザーのパスワードを入力します。

<img src="../../Research/CCID/reference/assets01/0017.png" width="400">

下図のようなダイアログが表示されます。<br>
デフォルトPIN番号である`123456`を入力します。

<img src="../../Research/CCID/reference/assets01/0018.png" width="400">

下図のようなダイアログが表示されます。<br>
キーチェーンのパスワード（通常はユーザーパスワードと同じ）を入力します。

<img src="../../Research/CCID/reference/assets01/0019.png" width="400">

ダイアログが閉じられ、スマートカードペアリングが完了します。

#### macOSにログイン

いったんmacOSからログオフし、[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)を、PCのUSBポートから外します。

<img src="../../Research/CCID/reference/assets01/0020.png" width="400">

macOSのログイン画面が表示されます。<br>
この後、ふたたび[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)を、PC（MacBook、iMac等）のUSBポートに装着します。

下図のように、ユーザーパスワードではなく、PIN番号を入力するためのボックスが表示されます。

<img src="../../Research/CCID/reference/assets01/0021.jpeg" width="400">

PIN番号を入力して、ログインを実行します。

<img src="../../Research/CCID/reference/assets01/0022.jpeg" width="400">

通常のパスワードによるログインと同様、macOSにログインできます。

<img src="../../Research/CCID/reference/assets01/0023.jpeg" width="400">

以上で、PIN番号によるmacOSログインは完了になります。
