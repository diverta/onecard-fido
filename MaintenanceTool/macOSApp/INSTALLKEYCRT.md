# 鍵・証明書の導入手順

## 概要

[FIDO認証器管理ツール](README.md)を使用して、[nRF52840版FIDO認証器](https://github.com/diverta/onecard-fido/tree/master/nRF5_SDK_v15.2.0)に鍵・証明書を導入する手順を掲載します。

## 管理ツールのインストール

[インストール手順](INSTALLPRG.md)を参照し、管理ツールをmacOSにインストールします。

## 鍵・証明書の削除

管理ツールを起動し、USBポートに[nRF52840版FIDO認証器](https://github.com/diverta/onecard-fido/tree/master/nRF5_SDK_v15.2.0)を装着します。

<img src="assets/0006.png" width="450">

次に「鍵・証明書・キーハンドル消去」ボタンをクリックします。

<img src="assets/0007.png" width="450">

確認ダイアログが表示されますので「Yes」をクリックします。

<img src="assets/0008.png" width="450">

FIDO認証器側の処理が成功すると「鍵・証明書・キーハンドル削除処理が成功しました。」と表示されます。

<img src="assets/0009.png" width="450">

## 鍵・証明書のインストール

秘密鍵ファイル（fido2test.pem）、証明書ファイル（fido2test.crt）は、GitHubリポジトリーに作り置きしてあります。<br>

<img src="../WindowsExe/assets/0014.png" width="640">

ファイルのGitHubリポジトリー上の場所は以下の通りです。
- ディレクトリー：[onecard-fido/Research/provisionalCA/](https://github.com/diverta/onecard-fido/blob/master/Research/provisionalCA/)
- 秘密鍵ファイル：[fido2test.pem](https://github.com/diverta/onecard-fido/blob/master/Research/provisionalCA/fido2test.pem)
- 証明書ファイル：[fido2test.crt](https://github.com/diverta/onecard-fido/blob/master/Research/provisionalCA/fido2test.crt)

上記の秘密鍵ファイル（fido2test.pem）、証明書ファイル（fido2test.crt）を、それぞれ「参照」ボタンをクリックして選択します。

<img src="assets/0010.png" width="400">

管理ツール画面の「鍵・証明書ファイルのインストール」ボタンをクリックします。

<img src="assets/0011.png" width="400">

FIDO認証器側の処理が成功すると「鍵・証明書インストールが成功しました。」と表示されます。

<img src="assets/0012.png" width="400">

以上で、鍵・証明書の導入は完了です。
