# CMakeインストール手順

最終更新日：2023/02/13

メイクファイル生成コマンド「cmake」を含むツール「CMake」のインストール手順について記載します。

## 使用したシステム

PC: iMac (Retina 5K, 27-inch, 2019)<br>
OS: macOS 12.6.3

## インストール媒体の取得

CMakeの[ダウンロードページ](https://cmake.org/download/)を開きます。

<img src="assets01/0001.jpg" width="550">

ダウンロードページを下にスクロールし、ファイルの一覧を表示させます。<br>
macOS環境にインストールする場合は、ファイル「`cmake-3.25.2-macos-universal.dmg`」をダウンロードします。[注1]

<img src="assets01/0002.jpg" width="550">

[注1] 今回使用したシステムに合わせたインストール媒体を使用しています。

### CMakeのインストール

ダウンロードした`cmake-3.25.2-macos-universal.dmg`を右クリックして「開く」を実行します。

<img src="assets01/0003.jpg" width="460">

以下のようなダイアログが表示された場合は「Agree」をクリックします。

<img src="assets01/0004.jpg" width="390">

パッケージフォルダーが表示されますので、画面左側のアイコン「`CMake.app`」をドラッグし「`Applications`」にドロップしてください。

<img src="assets01/0005.jpg" width="360">

アプリケーションフォルダーにCMakeのアイコンができていれば、インストールは成功です。

<img src="assets01/0006.jpg" width="470">

最後に、ファイルメニューから「取り出す」を選択し、パッケージフォルダーを閉じます。

<img src="assets01/0007.jpg" width="360">

以上で、CMakeのインストールは完了です。
