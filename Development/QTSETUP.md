# Qtプラットフォーム導入手順

## 概要

Windows版U2F管理ツールのGUI化作業で使用する、Qtプラットフォーム（Qtデザイナー＋コンパイラー＋各種ライブラリーの一式）について、Windows環境へ導入する手順を、下記に掲載いたします。

## インストール

まずは[インストーラー](assets/qt-unified-windows-x86-3.0.2-online.exe)を起動します。

<img src="assets/0001.png" width="700">

下図の画面で「Next」をクリックします。

<img src="assets/0002.png" width="400">

リポジトリーの検索が始まり、しばらくの間（５分以上）待ちになります。

<img src="assets/0003.png" width="400">

デフォルト表示のまま「次へ」をクリックします。

<img src="assets/0004.png" width="400">

あらかじめ作成しておいたQtアカウントでログインします。

<img src="assets/0005.png" width="400">

Qtセットアップ画面に遷移するので「次へ」をクリックします。

<img src="assets/0006.png" width="400">

必要なコンポーネントを選択し「次へ」をクリックします。

<img src="assets/0007.png" width="400">

選択コンポーネントは下記の通りになります。
（不要なものもあるかもしれません）
```
Qt 5.10.0
 MSVC 2015 32-bit
 MinGW 5.3.0 32 bit
 UWP x86 (MSVC 2015)
 Sources
 Qt Charts
 Qt Data Visualization
 Qt Purchasing
 Qt Virtual Keyboard
 Qt WebEngine
 Qt Network Authorization
 Qt Remote Objects
 Qt WebGL Streaming Plugin
 Qt Script
Tools
 Qt Creator 4.5.0
 Qt Creator 4.5.0 CDB Debugger
```

ライセンス条項の同意画面に遷移するので、上のラジオボタンをチェックして「次へ」をクリックします。

<img src="assets/0008.png" width="400">

デフォルト表示のまま「次へ」をクリックします。

<img src="assets/0009.png" width="400">

インストール画面に遷移するので「インストール」をクリックします。

<img src="assets/0010.png" width="400">

インストールが開始されると、下図のような画面が表示され、１時間以上待ちになります。

<img src="assets/0011.png" width="400">

インストールが完了したら、下図画面の「次へ」をクリックします。

<img src="assets/0012.png" width="400">

ウィザードの完了画面に遷移します。<br>
Launch Qt CreatorのチェックをOnにして「完了」をクリックします。

<img src="assets/0013.png" width="400">

下図のような画面が表示されれば、Qtプラットフォームの導入は完了です。

<img src="assets/0014.png" width="700">
