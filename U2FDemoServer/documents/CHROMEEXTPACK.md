# Chrome U2Fエクステンションのパッケージ手順

`u2f-chrome-extension.crx`をパッケージする方法について掲載いたします。

## Chromeブラウザー上の操作

### エクステンションのパッケージ

Chromeブラウザーを起動し、デベロッパーモードに変更したのち「拡張機能をパッケージ化」をクリックします。

<img src="assets/0001.png" width="600">

パッケージ化するエクステンションのディレクトリー（`<リポジトリールート>/U2FDemoServer/u2f-chrome-extension`）を入力します。

秘密鍵ファイルは、エクステンションのソースを修正した時（＝エクステンションを作成し直す時）に指定します。<br>
下図は新規にエクステンションを作成する例になりますので、鍵ファイルは指定していません。

<img src="assets/0002.png" width="600">

メッセージが表示され、エクステンション`u2f-chrome-extension.crx`が作成されました。

<img src="assets/0003.png" width="600">

これでエクステンションのパッケージは完了です。

### エクステンションのインストール

上記手順で作成された`u2f-chrome-extension.crx`を、Chromeブラウザーの「拡張機能ページ」（`chrome://extensions/`）にドラッグ＆ドロップします。

<img src="assets/0004.png" width="600">

確認メッセージが表示されるので「拡張機能を追加」をクリックします。

<img src="assets/0005.png" width="600">

エクステンションがChromeに追加された旨のメッセージが表示されます。

<img src="assets/0006.png" width="600">

デベロッパーモードに変更すると、新しくインストールしたエクステンションのID`jgomidmeajgnpflcklfhcbpamnbdfmoo`が確認できます。

<img src="assets/0007.png" width="600">

確認後は、デベロッパーモードを無効にしておくようにします。

<img src="assets/0008.png" width="600">

これでエクステンションのインストールは完了です。
