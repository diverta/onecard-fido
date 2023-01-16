# CCIDドライバーインストール手順

## 概要
CCIDドライバーをmacOS環境にインストールし、[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のCCIDインターフェースを利用できるようにするための手順について掲載しています。

#### ご参考
Windows10 環境では、CCIDドライバーが最初からシステムに組み込まれているため、CCIDドライバーのインストールは不要です。

## インストール媒体の取得

CCIDドライバー「[CCIDDriver.pkg](../CCID/macOSDriver/CCIDDriver.pkg)」を、GitHubからダウンロード／解凍します。<br>
該当ページの「Download」ボタンをクリックすると、[CCIDDriver.pkg](../CCID/macOSDriver/CCIDDriver.pkg)がダウンロードできます。

<img src="assets02/0001.jpg" width="640">

## インストールの実行

ダウンロードされたファイルを右クリックし「開く」を実行してください。<br>
（2020/07/21現在、アプリに署名がされていないので、アイコンをダブルクリックしても実行することができないための措置になります）

<img src="assets02/0002.jpg" width="500">

警告画面が表示されますが、続いて「開く」を実行します。

<img src="assets02/0003.jpg" width="300">

インストーラーが起動しますので、指示に従いインストールを進めます。

<img src="assets02/0004.jpg" width="400">

インストールが完了します。

<img src="assets02/0005.jpg" width="400">

インストーラを閉じたら、PCを再起動します。

<img src="assets02/0006.jpg" width="300">

再起動が完了してログインしたら、MDBT50Q DongleをPCのUSBポートに装着します。<br>
１〜２秒ほどで、下図のような「スマートカードペアリング」画面が表示されます。

<img src="assets02/0007.jpg" width="300">

ここではペアリングせず、キャンセルをクリックして画面を閉じてください。<br>
（2020/07/21現在、PIVアプリケーションがMDBT50Q Dongleに搭載されていないため、macOSとMDBT50Q Dongleのペアリングは不可能となっております）

これで、CCIDドライバーのインストールは完了です。

## アンインストールの実行

CCIDドライバーをアンインストールするためには、ターミナルを開いて、下記のコマンドを実行します。<br>
実行時はパスワードの入力が求められますので、管理者パスワードを入力してください。

```
sudo rm -rfv "/usr/local/libexec/SmartCardServices"
```

下記は実行例になります。

```
bash-3.2$ sudo rm -rfv "/usr/local/libexec/SmartCardServices"
Password:
/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle/Contents/MacOS/libccid.dylib
/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle/Contents/MacOS
/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle/Contents/Info.plist
/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle/Contents
/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle
/usr/local/libexec/SmartCardServices/drivers
/usr/local/libexec/SmartCardServices
bash-3.2$
```

これで、CCIDドライバーのアンインストールは完了です。
