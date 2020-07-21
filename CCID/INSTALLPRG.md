# CCIDドライバーインストール手順

## 概要
[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のCCIDインターフェース専用ドライバー（以下単に「CCIDドライバー」）を、macOS環境にインストールする手順を掲載しています。


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
