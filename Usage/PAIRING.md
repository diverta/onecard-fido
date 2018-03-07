# One Cardペアリング手順

One CardおよびBLE U2Fサービスを使用するためには、ペアリングが必要となります。

ペアリングを行うためには、One Cardをペアリングモードに変更します。

## 非ペアリングモード--->ペアリングモードへの変更

One Cardに電源を投入すると、非ペアリングモードで起動しますが、これではペアリングを行うことができません。

ペアリングモードに変更させるためには、One CardのMAIN SWを５秒間押し続けます。

<img src="../assets/0012.png" width="500">

One Card上の２番目のLEDが<font color=ff0000><b>点灯</b></font>したら、指をMAIN SWから離します。

<img src="../assets/0013.png" width="500">

これでペアリングモードに変更されました。

## ペアリングの実施

### Windowsの場合

スタートメニューから設定を実行し、デバイスをクリックします。

<img src="../assets/0014.png" width="500">

Bluetoothをクリックします。<br>OneCard_Peripheralが表示されますので、「ペアリング」をクリックします。

<img src="../assets/0015.png" width="500">

しばらくすると「ペアリングの準備完了」の表示が「接続済み」に変わります。

<img src="../assets/0016.png" width="500">

これで、Windowsとのペアリングは完了です。

### macOSの場合

[U2F管理ツール](../U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)をGitHubからダウンロード／解凍します。<br>

<img src="../assets/0070.png" width="600">

ダウンロードされたファイルを右クリックし「開く」を実行してください。<br>
（アイコンをダブルクリックしても、実行することができないための措置になります）

<img src="../assets/0021.png" width="500">

警告画面が表示されますが、続いて「開く」を実行します。

<img src="../assets/0022.png" width="500">

インストーラーが起動しますので、指示に従いインストールを進めます。

<img src="../assets/0071.png" width="500">

インストールが完了すると、アプリケーションフォルダーに、U2F管理ツールのアイコンができます。<br>
アイコンをダブルクリックして実行します。

<img src="../assets/0072.png" width="500">

U2F管理ツールが起動しますので、画面上の「ヘルスチェック実行」ボタンをクリックします。

<img src="../assets/0073.png" width="500">

後述<b>「鍵・証明書のインストール」</b>が行われていないため、下図のようなダイアログが表示されます。<br>
ただし異常ではないので、OKボタンを押してダイアログを閉じます。

<img src="../assets/0018.png" width="500">

その後「終了」ボタンをクリックして、U2F管理画面を終了させます。

<img src="../assets/0074.png" width="500">

macOSの設定画面からBluetooth設定を参照すると、下図のように、ペアリングが自動的に行われていることが確認できます。

<img src="../assets/0019.png" width="500">

これで、macOSとのペアリングは完了です。

## ペアリングモード--->非ペアリングモードへの変更

ペアリングモードから、非ペアリングモードに変更させるためには、One CardのMAIN SWを５秒間押し続けます。

<img src="../assets/0075.png" width="500">

One Card上の２番目のLEDが<font color=ff0000><b>消灯</b></font>したら、指をMAIN SWから離します。

<img src="../assets/0076.png" width="500">

これで非ペアリングモードに変更されました。

【ご参考】<br>
One Cardは、電源Off状態から電源投入時は、常に非ペアリングモードで起動します。
