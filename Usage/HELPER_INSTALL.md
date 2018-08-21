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

### ダウンロードと実行確認

[U2FMaintenanceToolWin.zip](../U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip) を、GitHubからダウンロードして取得します。

<img src="assets/0053.png" width="600">

U2FMaintenanceToolWin.zipを展開すると、下図のように「U2FMaintenanceToolWin」というフォルダーができるので、任意の場所に配置します。<br>
その後、フォルダー内の実行ファイル「U2FHelper.exe」をダブルクリックします。

<img src="assets_hlp/0012.png" width="500" border="1">

2018/08/21現在、アプリに署名がされていないため、ダウンロードしたプログラムを実行できない旨のダイアログが表示されます。<br>
「詳細情報」をクリックして、実行ボタンを表示させます。

<img src="assets_hlp/0013.png" width="350">

表示された実行ボタンをクリックして、U2F Helperを実行させます。

<img src="assets_hlp/0014.png" width="350">

U2F Helperが起動し、タスクトレイに入っていることを確認します。

<img src="assets_hlp/0015.png" width="350">

### タスクトレイ常駐設定

前述手順で配置したインストール媒体「U2FMaintenanceToolWin」内の「U2FHelper.exe」に対して、ショートカットを作成します。

<img src="assets_hlp/0020.png" width="500" border="1">

その後「ファイル名を指定して実行」で `shell:startup`を実行します。

<img src="assets_hlp/0016.png" width="350">

スタートアップフォルダーが開くので、先ほど作成したショートカットを、スタートアップフォルダーに移動します。

<img src="assets_hlp/0017.png" width="550" border="1">

その後、PCを再起動します。

<img src="assets_hlp/0018.png" width="250">

PCが再起動すると、約30秒ほどでU2F Helperがスタートアップ起動します。

下図のようなポップアップが表示されることがありますが、その場合は「このファイルを開く前に常に確認する(W)」のチェックを外した上で、「開く(O)」ボタンをクリックして、起動処理を続行させてください。

<img src="assets_hlp/0019.png" width="350">

これでU2F Helperのインストールは完了です。
