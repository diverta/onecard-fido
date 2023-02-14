# nRF Command Line Toolsインストール手順

最終更新日：2023/02/14

デスクトップツール「nRF Command Line Tools」を、macOS環境にインストールする手順について記載します。

## 使用したシステム

PC: iMac (Retina 5K, 27-inch, 2019)<br>
OS: macOS 12.6.3

## インストール用媒体の取得

こちらのサイトにアクセスします。<br>
https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download

下図のような画面に遷移します。

<img src="assets01/0010.jpg" width="550">

サイト中段のプルダウンリストから「macOS」を選択します。

<img src="assets01/0011.jpg" width="550">

右側に表示された「`nrf-command-line-tools-10.19.0-Darwin.dmg`」のリンクをクリックし、ダウンロードを開始させます。

<img src="assets01/0012.jpg" width="550">

「`nrf-command-line-tools-10.19.0-darwin.dmg`」という名前のファイルがダウンロードされます。

## インストールの実行

以下２点のソフトウェアを、必ずセットでインストールします。

- <b>nRF Tools</b><br>
書込みツール`nrfjprog`を含むソフトウェアです。

- <b>J-Link</b><br>
PCとUSB経由で接続中のnRF5340開発ボード（`PCA10095`）を接続するためのミドルウェアです。

### nRF Toolsのインストール

ダウンロードフォルダーにある「`nrf-command-line-tools-10.19.0-darwin.dmg`」を右クリックして、パッケージフォルダーを開きます。

<img src="assets01/0013.jpg" width="550">

パッケージフォルダーが表示されます。<br>
青いアイコン「Install nRF Tools」をダブルクリックして、インストーラを開きます。

<img src="assets01/0014.jpg" width="400">

インストーラの指示に従い、インストール作業を進めます。

<img src="assets01/0015.jpg" width="400">

インストール作業が完了します。

<img src="assets01/0016.jpg" width="400">

### J-Linkのインストール

次に、パッケージフォルダーの黄色いアイコン「Install J-LINK」をダブルクリックして、インストーラを開きます。

<img src="assets01/0018.jpg" width="400">

インストーラの指示に従い、インストール作業を進めます。

<img src="assets01/0019.jpg" width="400">

インストール作業が完了します。

<img src="assets01/0020.jpg" width="400">

最後に、アプリケーションフォルダーに「Nordic Semiconductor」「SEGGER」というフォルダーが作成されていることを確認します。

<img src="assets01/0017.jpg" width="500">

以上で「nRF Command Line Tools」のインストールは完了となります。
