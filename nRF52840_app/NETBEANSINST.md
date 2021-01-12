# NetBeansインストール手順

NetBeansとARM GCC、nRF5 SDKを使用し、nRF52840の開発環境を構築する手順を記載します。

## 前提ソフトウェアのインストール

まず最初に、ARM GCCツールチェイン、nRF5 SDKをインストールします。<br>
以下の手順書をご参照願います。

- <b>[ARM GCCインストール手順](ARMGCCINST.md)</b>
- <b>[nRF5 SDKインストール手順](NR5SDKINST.md)</b>

## インストール用媒体の取得

### NetBeans

こちらのサイトにアクセスします。<br>
https://netbeans.org/downloads/8.2/<br>
下図のような画面に遷移します。

<img src="assets02/0006.jpg" width="600">

「NetBeans IDE ダウンロードバンドル」の「C/C++」をダウンロードします。<br>
「netbeans-8.2-cpp-macosx.dmg」というファイルがダウンロードされます。

### nRFコマンドラインツール

こちらのサイトにアクセスします。<br>
https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF5-Command-Line-Tools/Download<br>
下図のような画面に遷移します。

<img src="assets02/0004.jpg" width="600">

サイト中段のラジオボタン「9.8.1 macOS」をチェックします。<br>
その後、右上にある「Download file」をクリックし、ダウンロードを開始させます。

<img src="assets02/0005.jpg" width="600">

「nRF-Command-Line-Tools_9_8_1_OSX.tar」という名前のファイルがダウンロードされます。

### SEGGER J-Link

こちらのサイトにアクセスします。<br>
https://www.segger.com/downloads/jlink<br>
下図のような画面に遷移します。

<img src="assets02/0008.jpg" width="600">

「J-Link Software and Documentation Pack」をクリックすると下図の画面のようになります。

<img src="assets02/0009.jpg" width="600">

一覧の上から２番目「J-Link Software and Documentation pack for macOS」の、DOWNLOADボタンをクリックし、ツールをダウンロードします。<br>
「JLink_MacOSX_V646d.pkg」というファイルがダウンロードされます。

## ソフトウェアのインストール

### NetBeansのインストール

ダウンロードした「netbeans-8.2-cpp-macosx.dmg」をダブルクリックすると、インストーラーが起動しますので、画面の指示に従い操作を進めます。

<img src="assets02/0010.jpg" width="400">

アプリケーションフォルダーに「NetBeans」というサブフォルダーが出来ていればインストールは完了です。

<img src="assets02/0011.jpg" width="400">

### SEGGER J-Linkのインストール

ダウンロードした「JLink_MacOSX_V646d.pkg」をダブルクリックすると、インストーラーが起動しますので、画面の指示に従い操作を進めます。

<img src="assets02/0012.jpg" width="400">

アプリケーションフォルダーに「SEGGER」というサブフォルダーが出来ていればインストールは完了です。

<img src="assets02/0013.jpg" width="400">

### NetBeansの設定変更

NetBeansを起動し、Preferencesを実行します。

<img src="assets02/0015.jpg" width="300">

オプション画面が開きますので「C/C++」の「ビルド・ツール」タブを開きます。<br>
画面左側の「ツール・コレクション(C)」下部の「追加」ボタンをクリックします。

<img src="assets02/0016.jpg" width="400">

下図のようなポップアップが表示されるので、以下のように設定します。

- ベース・ディレクトリ - ARM GCCツールチェインのルートディレクトリーを選択
- ツール・コレクション・ファミリ - GNU Mac を選択
- ツール・コレクション名 - GNU_ARM と入力

選択／入力が完了したら「OK」をクリックします。

<img src="assets02/0017.jpg" width="450">

オプション画面に戻ったら、画面右上部の「$PATH」というボタンをクリックします。

<img src="assets02/0018.jpg" width="600">

「実行コマンド・パスの変更」欄に`nrfjprog`（nRF5xコマンドライン・ツールのひとつ）のパスを追加します。<br>
下記例では「`;${HOME}/opt/nRF-Command-Line-Tools_10_9_0_OSX/nrfjprog`」という文字列を追加しています。

入力が完了したら「OK」をクリックします。

<img src="assets02/0019.jpg" width="400">

オプション画面に戻ったら、下図のように残りの項目を次々と設定します。

- Cコンパイラ - arm-none-eabi-gcc
- C++コンパイラ - arm-none-eabi-g++
- アセンブラ - arm-none-eabi-as

設定が完了したら「適用」をクリックします。

<img src="assets02/0020.jpg" width="600">

続いて、再び「GNU_ARM」を選択の上、設定内容が適用されていることを確認後「OK」ボタンをクリックします。

<img src="assets02/0021.jpg" width="600">

以上で、NetBeansとその稼働に必要なソフトウェアのインストールは完了となります。
