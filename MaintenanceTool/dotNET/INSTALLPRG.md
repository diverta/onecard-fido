# インストール手順

最終更新日：2022/11/8

## 概要
FIDO認証器管理ツールをWindows環境にインストールする手順を掲載しています。

## インストール媒体の取得

[Windows版 FIDO認証器管理ツール](../../MaintenanceTool/dotNET/MaintenanceToolWin.zip)を、GitHubからダウンロード／解凍します。<br>
該当ページの「Download」ボタンをクリックすると、[MaintenanceToolWin.zip](../../MaintenanceTool/dotNET/MaintenanceToolWin.zip)がダウンロードできます。

<img src="assets01/0004.jpg" width="640">

ダウンロードが完了したら、ダウンロードフォルダーを開きます。

<img src="assets01/0005.jpg" width="650">

Windowsのエクスプローラが表示されますので、ファイルをダブルクリックして開きます。

<img src="assets01/0006.jpg" width="500">

「setup.exe」と「SetupWizard.msi」の２点のファイルが、インストール媒体になります。

<img src="assets01/0007.jpg" width="500">

## インストールの実行

前述の実行ファイル「setup.exe」をダブルクリックして実行してください。

<img src="assets01/0007.jpg" width="500">

最終更新日現在、アプリに署名がされていないため、ダウンロードしたプログラムを実行できない旨のダイアログが表示されます。<br>
「詳細情報」をクリックします。

<img src="../../MaintenanceTool/WindowsExe/assets/0005.png" width="300">

画面表示が変わり「実行ボタン」が表示されますので、その「実行ボタン」をクリックします。

<img src="../../MaintenanceTool/WindowsExe/assets/0006.png" width="300">

インストーラーが起動しますので、指示に従いインストールを進めます。

<img src="../../MaintenanceTool/WindowsExe/assets04/0005.png" width="300">

インストールが正常に完了したら「閉じる」をクリックし、インストーラーを終了させます。

<img src="../../MaintenanceTool/WindowsExe/assets04/0006.png" width="300">

Windowsのスタートメニューに、フォルダー「FIDO Authenticator Tools」とアイコン「FIDO Authenticator Maintenance Tool」が作成されていることを確認します。<br>
アイコンを右クリックし、インストールされたFIDO認証器管理ツールを実行します。

<img src="assets01/0003.jpg" width="350">

「アプリがデバイスに変更を加えることを許可しますか？」というメッセージが表示されます。[注1]<br>
「はい」ボタンをクリックすると、ツールが起動します。

<img src="assets01/0001.png" width="250">

FIDO認証器管理ツールの画面が起動すれば、インストールは完了です。

<img src="assets01/0002.jpg" width="400">

[注1] Windows 10のバージョン「Windows 10 November 2019 Update」以降においては、管理者として実行されていないプログラムの場合、FIDOデバイスとの直接的なUSB通信ができない仕様となったようです。Windows版管理ツールでは、鍵・証明書インストールなどの管理機能を実行時、FIDOデバイスとの直接的なUSB通信が必要なため、管理者として実行させるようにしております。その影響で、ツール起動のたびに「アプリがデバイスに変更を加えることを許可しますか？」というメッセージが表示されてしまいますが、不具合ではありません。
