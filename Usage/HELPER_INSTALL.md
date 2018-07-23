# U2F Helperインストール手順

U2F HIDデバイスを使用し、ChromeブラウザーのU2Fクライアントと、One CardのFIDO機能を連携させるために必要となる、U2F Helperのインストール手順を掲載いたします。

## U2F Helperのインストール（macOS版）

[U2F Helper](../U2FMaintenanceTool/macOSApp/U2FHelper.pkg)を、GitHubからダウンロード／解凍します。<br>

<img src="assets_hlp/0001.png" width="600">

ダウンロードされたファイルを右クリックし「開く」を実行してください。<br>
（2018/07/23現在、アプリに署名がされていないので、アイコンをダブルクリックしても実行することができないための措置になります）

<img src="assets_hlp/0002.png" width="500">

警告画面が表示されますが、続いて「開く」を実行します。

<img src="assets_hlp/0003.png" width="300">

インストーラーが起動しますので、指示に従いインストールを進めます。

<img src="assets_hlp/0004.png" width="450">

インストールが完了すると、アプリケーションフォルダーに、U2F Helperのアイコンができます。

<img src="assets_hlp/0005.png" width="500">

この後、システム環境設定画面を開き「ユーザとグループ」をクリックします。

<img src="assets_hlp/0006.png" width="500">

「ユーザとグループ」画面の「ログイン項目」タブをクリックしたら、一覧左下の「＋」ボタンをクリックします。

<img src="assets_hlp/0007.png" width="450">

表示されたファイル選択ダイアログで、アプリケーションフォルダーのU2F Helperのアイコンを選択し「追加」をクリックします。

<img src="assets_hlp/0008.png" width="450">

ログイン項目にU2F Helperが追加されたことを確認します。

<img src="assets_hlp/0009.png" width="450">

いったんログオフします。

<img src="assets_hlp/0010.png" width="200">

再度ログインすると、ステータスバーにU2F Helperのアイコンが表示されます。

<img src="assets_hlp/0011.png" width="660">


これでU2F Helperのインストールは完了です。

## U2F Helperのインストール（Windows版）

Windows版は、後日作成予定です。
