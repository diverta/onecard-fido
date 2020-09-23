# PIV機能によるmacOSログイン手順

[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のPIV機能を使用して、macOSにPINでログインする手順を掲載いたします。

## 準備手順

下記手順書により、MDBT50Q DongleでPIV機能が使えるように準備を行います。

- <b>[CCIDドライバーインストール手順](../CCID/INSTALLPRG.md)</b><br>
[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のCCIDインターフェース専用ドライバー（以下単に「CCIDドライバー」）を、macOS環境にインストールします。

- <b>[Yubico PIV Tool (command line) macOS版 導入手順](../CCID/PIVTOOLMACINST.md)</b><br>
PIVで使用する証明書等を導入するために利用できる「Yubico PIV Tool (command line) 」を、macOS環境に導入します。

- <b>[Yubico PIV Toolによる初期データ導入手順](../CCID/YKPIVUSAGE.md)</b><br>
Yubico PIV Tool (command line) を使用して、鍵・証明書などを[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)に導入します。

## 作業手順

#### スマートカードペアリングの実行

[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)を、PC（MacBook Pro）のUSBポートに装着します。<br>
初回装着の場合、以下のようなダイアログが表示されます。

「PIV認証用証明書」を選択し「ペアリング」ボタンをクリックします。

<img src="../Research/CCID/reference/assets01/0016.png" width="400">

下図のようなダイアログが表示されます。<br>
macOSユーザーのパスワードを入力します。

<img src="../Research/CCID/reference/assets01/0017.png" width="400">

下図のようなダイアログが表示されます。<br>
デフォルトPIN番号（以下単に「PIN」と称します）である`123456`を入力します。

<img src="../Research/CCID/reference/assets01/0018.png" width="400">

下図のようなダイアログが表示されます。<br>
キーチェーンのパスワード（通常はユーザーパスワードと同じ）を入力します。

<img src="../Research/CCID/reference/assets01/0019.png" width="400">

ダイアログが閉じられ、スマートカードペアリングが完了します。

#### macOSにログイン

いったんmacOSからログオフし、[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)を、PCのUSBポートから外します。

<img src="../Research/CCID/reference/assets01/0020.png" width="400">

macOSのログイン画面が表示されます。<br>
この後、ふたたび[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)を、PC（MacBook Pro）のUSBポートに装着します。

下図のように、ユーザーパスワードではなく、PINを入力するためのボックスが表示されます。

<img src="../Research/CCID/reference/assets01/0021.jpeg" width="400">

PINを入力して、ログインを実行します。

<img src="../Research/CCID/reference/assets01/0022.jpeg" width="400">

通常のパスワードによるログインと同様、macOSにログインできます。

<img src="../Research/CCID/reference/assets01/0023.jpeg" width="400">

以上で、PIV機能によるmacOSログインは完了になります。
