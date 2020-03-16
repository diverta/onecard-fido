# インストール手順

## 概要
FIDO認証器管理ツールをWindows環境にインストールする手順を掲載しています。

## インストール媒体の取得

[Windows版 FIDO認証器管理ツール](U2FMaintenanceToolWin.zip)を、GitHubからダウンロード／解凍します。<br>
該当ページの「Download」ボタンをクリックすると、[MaintenanceToolWin.zip](MaintenanceToolWin.zip)がダウンロードできます。

<img src="assets/0001.jpg" width="640">

ダウンロードが完了したら「開く」を実行します。

<img src="assets/0002.jpg" width="640">

Windowsのエクスプローラが表示されますので、フォルダー「MaintenanceTool」をダブルクリックします。

<img src="assets/0003.png" width="500">

「setup.exe」と「SetupWizard.msi」の２点のファイルが、インストール媒体になります。

<img src="assets/0004.png" width="500">

## インストールの実行

前述の実行ファイル「setup.exe」をダブルクリックして実行してください。

<img src="assets/0004.png" width="500">

2019/12/24現在、アプリに署名がされていないため、ダウンロードしたプログラムを実行できない旨のダイアログが表示されます。<br>
「詳細情報」をクリックします。

<img src="assets/0005.png" width="300">

画面表示が変わり「実行ボタン」が表示されますので、その「実行ボタン」をクリックします。

<img src="assets/0006.png" width="300">

インストーラーが起動しますので、指示に従いインストールを進めます。

<img src="assets04/0005.png" width="300">

インストールが正常に完了したら「閉じる」をクリックし、インストーラーを終了させます。

<img src="assets04/0006.png" width="300">

Windowsのスタートメニューに、フォルダー「FIDO Authenticator Tools」とアイコン「FIDO Authenticator Maintenance Tool」が作成されていることを確認します。<br>
アイコンを右クリックし、インストールされたFIDO認証器管理ツールを「管理者として実行」します。[注1]

<img src="assets04/0007.jpg" width="600">

FIDO認証器管理ツールの画面が起動すれば、インストールは完了です。

<img src="assets04/0004.png" width="400">

[注1] Windows 10の最新バージョン（Windows 10 November 2019 Update）においては、管理者として実行されていないプログラムの場合、FIDOデバイスとの直接的なUSB通信ができない仕様となったようです。Windows版管理ツールでは、鍵・証明書インストールなどの管理機能を実行時、FIDOデバイスとの直接的なUSB通信が必要なため、管理者として実行させる前提としております。<br>
Windows版管理ツールを「管理者として実行」しない場合は、下記のようなエラーメッセージがポップアップ表示されます。

<img src="assets04/0008.jpg" width="400">
