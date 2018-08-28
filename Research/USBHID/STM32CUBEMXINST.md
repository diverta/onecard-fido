# STM32CubeMXプラグイン インストール手順

「System Workbench for STM32F」に、プロジェクト・テンプレートを生成するためのツール「STM32CubeMXプラグイン」をインストールします。

## インストール媒体取得

下記ページを開きます。

STSW-STM32095<br>
https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-configurators-and-code-generators/stsw-stm32095.html

<img src="assets01/0006.png" width="700">

ページ下部の「Get Software」をクリックして、インストール媒体をダウンロードします。

<img src="assets01/0007.png" width="700">

「en.stsw-stm32095.zip」というファイルがダウンロードされます。

<img src="assets01/0008.png" width="500">

こちらは解凍せずに、そのまま保持してください。

## インストール実行

「System Workbench for STM32F」を起動します。

Eclipseメニューから「Install New Software...」を実行します。

<img src="assets01/0009.png" width="700">

表示された画面で「Add...」ボタンをクリックします。

<img src="assets01/0010.png" width="600">

表示されたポップアップの「Archive...」ボタンをクリックします。

<img src="assets01/0011.png" width="350">

ファイル選択ダイアログで、先ほどダウンロードしたzipファイル「en.stsw-stm32095.zip」を選択します。

<img src="assets01/0012.png" width="450">

Name欄はブランクのままで「OK」ボタンをクリックします。

<img src="assets01/0013.png" width="350">


画面の一覧表の「STM32CubeMX_Eclipse_Plugin」をチェックして「Next」ボタンをクリックします。

<img src="assets01/0014.png" width="600">

次の画面でも「Next」ボタンをクリックします。

<img src="assets01/0015.png" width="600">

上のラジオボタンを選択して同意し「Finish」ボタンをクリックします。

<img src="assets01/0016.png" width="600">

以下のような警告が表示されたら「OK」ボタンをクリックします。

<img src="assets01/0017.png" width="350">

Eclipseを再起動する旨のメッセージが表示されるので「Yes」ボタンをクリックします。

<img src="assets01/0018.png" width="350">

Eclipseが再起動されます。

## 起動確認

EclipseメニューでOpen Perspectiveを実行します。

<img src="assets01/0019.png" width="700">

表示された一覧から「STM32CubeMX」を選択し「OK」ボタンをクリックします。

<img src="assets01/0020.png" width="300">

STM32CubeMXが立ち上がることを確認します。

<img src="assets01/0021.png" width="700">

以上で、STM32CubeMXプラグインのインストールは完了です。
