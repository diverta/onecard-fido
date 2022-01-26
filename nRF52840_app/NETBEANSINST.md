# NetBeansインストール手順

NetBeansとARM GCC、nRF5 SDKを使用し、nRF52840の開発環境を構築する手順を記載します。

## 前提ソフトウェアのインストール

まず最初に、ARM GCCツールチェイン、CMake、nRF5 SDK、nRF Command Line Toolsをインストールします。<br>
以下の手順書をご参照願います。

- <b>[ARM GCCインストール手順](../nRF52840_app/ARMGCCINST.md)</b>

- <b>[CMakeインストール手順](../nRF5340_app/INSTALLCMAKE.md)</b>

- <b>[nRF5 SDKインストール手順](../nRF52840_app/NR5SDKINST.md)</b>

- <b>[nRF Command Line Toolsインストール手順](../nRF52840_app/NRFCLTOOLINST.md)</b>

## インストール用媒体の取得

こちらのサイトにアクセスします。<br>
https://netbeans.org/downloads/8.2/<br>
下図のような画面に遷移します。

<img src="assets02/0006.jpg" width="600">

「NetBeans IDE ダウンロードバンドル」の「C/C++」をダウンロードします。<br>
「netbeans-8.2-cpp-macosx.dmg」というファイルがダウンロードされます。

## ソフトウェアのインストール

### NetBeansのインストール

ダウンロードした「netbeans-8.2-cpp-macosx.dmg」をダブルクリックすると、インストーラーが起動しますので、画面の指示に従い操作を進めます。

<img src="assets02/0010.jpg" width="400">

アプリケーションフォルダーに「NetBeans」というサブフォルダーが出来ていればインストールは完了です。

<img src="assets02/0011.jpg" width="400">

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
下記例では「`;/Applications/Nordic Semiconductor/bin/`」という文字列を追加しています。

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

以上で、NetBeansのインストールは完了となります。
