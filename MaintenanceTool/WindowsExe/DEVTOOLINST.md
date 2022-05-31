# Windows版開発ツール インストール手順

最終更新日：2022/5/12

## 概要
FIDO認証器開発ツールをWindows環境にインストールする手順を掲載しています。

## インストール媒体の取得

[Windows版 FIDO認証器開発ツール](DevelopmentToolWin.zip)を、GitHubからダウンロード／解凍します。<br>
該当ページの「Download」ボタンをクリックすると、[DevelopmentToolWin.zip](DevelopmentToolWin.zip)がダウンロードできます。

<img src="assets08/0021.jpg" width="640">

ダウンロードが完了したら「DevelopmentToolWin.zip」を開きます。

<img src="assets08/0022.jpg" width="640">

Windowsのエクスプローラが表示されたら、フォルダー「DevelopmentTool」をダブルクリックします。

<img src="assets08/0023.jpg" width="500">

「setup.exe」と「SetupWizard.msi」の２点のファイルが、インストール媒体になります。

<img src="assets08/0024.jpg" width="500">

## インストールの実行

前述の実行ファイル「setup.exe」をダブルクリックして実行してください。

<img src="assets08/0024.jpg" width="500">

最終更新日現在、アプリに署名がされていないため、ダウンロードしたプログラムを実行できない旨のダイアログが表示されます。<br>
「詳細情報」をクリックします。

<img src="assets08/0025.jpg" width="300">

画面表示が変わり「実行ボタン」が表示されますので、その「実行ボタン」をクリックします。

<img src="assets08/0026.jpg" width="300">

インストーラーが起動しますので、指示に従いインストールを進めます。

<img src="assets08/0027.jpg" width="300">

インストールが正常に完了したら「閉じる」をクリックし、インストーラーを終了させます。

<img src="assets08/0028.jpg" width="300">

Windowsのスタートメニューに、フォルダー「FIDO Authenticator Tools」とアイコン「FIDO Authenticator Development Tool」が作成されていることを確認します。<br>
アイコンを右クリックし、インストールされたFIDO認証器開発ツールを「管理者として実行」します。[注1]

<img src="assets08/0002.jpg" width="500">

FIDO認証器開発ツールの画面が起動すれば、インストールは完了です。

<img src="assets08/0001.jpg" width="400">

[注1] Windows 10のバージョン「Windows 10 November 2019 Update」以降においては、管理者として実行されていないプログラムの場合、FIDOデバイスとの直接的なUSB通信ができない仕様となったようです。Windows版開発ツールでは、鍵・証明書インストールなどの管理機能を実行時、FIDOデバイスとの直接的なUSB通信が必要なため、管理者として実行させる前提としております。<br>
Windows版開発ツールを「管理者として実行」しない場合は、下記のようなエラーメッセージがポップアップ表示されます。

<img src="assets08/0003.jpg" width="200">
