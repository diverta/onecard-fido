# CCIDドライバーインストール手順

最終更新日：2023/3/9

## 概要
CCIDドライバーをmacOS環境にインストールし、[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のCCIDインターフェースを利用できるようにするための手順について掲載しています。

#### ご参考
Windows環境では、CCIDドライバーが最初からシステムに組み込まれているため、CCIDドライバーのインストールは不要です。

## インストール媒体の取得

CCIDドライバーを、GitHubからダウンロード／解凍します。<br>
Apple Silicon向け、Intel mac向けに、対応するCCIDドライバーを用意しています。

- Apple Silicon向け：[`CCIDDriver_arm64.pkg`](../CCID/macOSDriver/CCIDDriver_arm64.pkg)
- Intel mac向け：[`CCIDDriver_x86.pkg`](../CCID/macOSDriver/CCIDDriver_x86.pkg)

該当ページの「Download」ボタンをクリックすると、CCIDドライバーのインストール用パッケージがダウンロードできます。

<img src="assets02/0001.jpg" width="640">

## インストールの実行

ダウンロードされたファイルを右クリックし「開く」を実行してください。<br>
（最終更新日現在、アプリに署名がされていないので、アイコンをダブルクリックしても実行することができないための措置になります）

<img src="assets02/0002.jpg" width="450">

警告画面が表示されますが、続いて「開く」を実行します。

<img src="assets02/0003.jpg" width="450">

インストーラーが起動しますので、指示に従いインストールを進めます。

<img src="assets02/0004.jpg" width="400">

インストールが完了します。

<img src="assets02/0005.jpg" width="400">

インストーラを閉じたら、PCを再起動します。

<img src="assets02/0006.jpg" width="360">

再起動が完了してログインしたら、MDBT50Q DongleをPCのUSBポートに装着します。<br>
１〜２秒ほどで、下図のような「スマートカードペアリング」画面が表示される場合がありますが、ここではペアリングせず画面を閉じてください。

<img src="assets02/0007.jpg" width="360">

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
