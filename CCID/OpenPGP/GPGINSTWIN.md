# Gpg4winインストール手順

GPGツール群「Gpg4win」を、Windows環境にインストールする手順について掲載します。

## 概要

[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)のOpenPGPカードエミュレーション機能を、Windows環境上で利用するためには、GPGツール群である「Gpg4win」をインストールする必要があります。

Gpg4winをインストールすると、以下の機能が利用できます。
- ファイルの署名およびその検証
- ファイルの暗号化およびその復号化
- 上記機能を利用するために必要となる秘密鍵／公開鍵の生成

## ダウンロード

こちらのサイトにアクセスします。<br>
https://www.gnupg.org<br>
下図のような画面に遷移します。

<img src="assets02/0001.jpg" width="600">

画面上部の「Download」というプルダウンメニューをクリックします。

<img src="assets02/0002.jpg" width="600">

遷移先画面を下側にスクロールすると「GNUPG BINARY RELEASES」という欄があるので、その中にある「Gpg4win」という青いリンクをクリックします。

<img src="assets02/0003.jpg" width="600">

「Download」画面に遷移するので、中央の「Gpg4win」という緑色のリンクをクリックします。

<img src="assets02/0004.jpg" width="600">

遷移先画面の「$0」をクリック後、表示される「Download」のリンクをクリックすると、ダウンロードが開始されます。

<img src="assets02/0005.jpg" width="600">

「`gpg4win-4.0.0.exe`」というファイルがダウンロードされます。

<img src="assets02/0006.jpg" width="600">

## インストール

ダウンロードされた「`gpg4win-4.0.0.exe`」を開くと、下図のようなインストーラーが起動します。<br>
「Next」をクリックします。

<img src="assets02/0007.jpg" width="350">

画面の案内にしたがってインストールを進めます。<br>
画面入力内容は別段変更せず「Next」ボタンをクリックします。

<img src="assets02/0008.jpg" width="350">

こちらも画面入力内容は別段変更しないままにします。<br>
「Install」ボタンをクリックすると、インストールが開始されます。

<img src="assets02/0009.jpg" width="350">

インストールが完了したら「Next」ボタンをクリックします。

<img src="assets02/0010.jpg" width="350">

遷移先画面の「Finish」ボタンをクリックします。

<img src="assets02/0011.jpg" width="350">

「Kleopatra」というアプリが自動的に起動します。<br>
表示された画面右上の「×」ボタンをクリックして閉じてください。

<img src="assets02/0012.jpg" width="500">

以上でGpg4winのインストールは完了です。
