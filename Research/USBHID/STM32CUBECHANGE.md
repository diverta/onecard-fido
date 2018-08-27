# プロジェクト設定項目修正手順

「STM32CubeMXプラグイン」上で、STM32CubeMXのプロジェクト設定項目を修正し、コードテンプレートに反映させる手順を掲載します。

## 手順

STM32CubeMXプラグインメニューの「Load Project」を実行します。

<img src="assets02/0001.png" width="550">

ファイル選択ダイアログで「.ioc」ファイルを選択して「開く」ボタンをクリックします。

<img src="assets02/0002.png" width="400">

以下は、USBデバイスのConfiguration項目を修正する例になります。<br>
「Middlewares」の「USB_DEVICE」ボタンをクリックします。

<img src="assets02/0003.png" width="700">

下図のような設定画面に切り替わります。<br>
初期状態では、以下のような値が設定されています。

<img src="assets02/0004.png" width="500">

「PRODUCT_STRING」を「U2F USB HID Device」と修正します。<br>
その後「OK」ボタンをクリックします。

<img src="assets02/0005.png" width="500">

メニューから「Save Project」を実行します。

<img src="assets02/0006.png" width="550">

修正内容は、以上の手順だけではコードテンプレートに反映されません。<br>
STM32CubeMXのプロジェクトから、もう一度、コードテンプレートを生成する必要があります。

メニューから「Generate Code」を実行します。

<img src="assets02/0007.png" width="550">

処理が終わると下図のようなポップアップが表示されます。<br>
「Close」をクリックしてポップアップを閉じます。

<img src="assets02/0008.png" width="600">

最後に、メニューから「Close Project」を実行して、STM32CubeMXのプロジェクトを閉じます。

<img src="assets02/0009.png" width="550">

以上で、プロジェクト設定項目の修正は完了です。

### 修正後の確認

修正された設定項目が、生成されたコードテンプレートに反映されていることを確認します。<br>
GitHubデスクトップなどのツールを利用して確認すると確実です。

<img src="assets02/0010.png" width="700">
