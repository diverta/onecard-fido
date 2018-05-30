# Chromeブラウザーを使用したデモ手順

パッケージ化されたChrome U2Fエクステンションを使用し、U2F管理ツール（エクステンションID修正版）、One Cardを使用して、U2F Register／Authenticateを実行するまでの手順を、以下に掲載いたします。

## U2Fローカルテストサーバーの準備

U2Fローカルテストサーバーを起動します。<br>
以下のコマンドを実行します。

```
MacBookPro-makmorit-jp:~ makmorit$ cd ~/GitHub/onecard-fido/U2FDemoServer/python-u2flib-server
MacBookPro-makmorit-jp:python-u2flib-server makmorit$ python ./u2f_server.py -i www.makmorit.jp
[29/May/2018 15:22:33] Starting server on http://www.makmorit.jp:8081
```

## デモ用プログラムのインストール

本件デモでは、マスターブランチでリリース済みのプログラムは使用できないので、U2Fローカルテストサーバーでの動作に特化した、以下のデモ用プログラムをインストールする必要があります。

- パッケージ化されたChrome U2Fエクステンション
- U2F管理ツール（エクステンションID修正版）

### ファイルのダウンロード

本件デモで使用する、パッケージ化されたChrome U2Fエクステンション、U2F管理ツール（エクステンションID修正版）を、下記場所からダウンロードします。

- パッケージ化されたChrome U2Fエクステンション<br>
[u2f-chrome-extension.crx](../u2f-chrome-extension.crx)

- U2F管理ツール（エクステンションID修正版）<br>
[U2FMaintenanceTool.pkg](../U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)

ダウンロードしたファイルはそれぞれインストールします。

### エクステンションのインストール

ダウンロードした`u2f-chrome-extension.crx`を、Chromeブラウザーの「拡張機能ページ」（`chrome://extensions/`）にドラッグ＆ドロップします。

<img src="assets/0004.png" width="600">

確認メッセージが表示されるので「拡張機能を追加」をクリックします。

<img src="assets/0005.png" width="600">

エクステンションがChromeに追加された旨のメッセージが表示されます。

<img src="assets/0006.png" width="600">

デベロッパーモードに変更すると、新しくインストールしたエクステンションのID`jgomidmeajgnpflcklfhcbpamnbdfmoo`が確認できます。

<img src="assets/0007.png" width="600">

確認後は、デベロッパーモードを無効にしておくようにします。

<img src="assets/0008.png" width="600">

これで、パッケージ化されたChrome U2Fエクステンションのインストールは完了です。

### U2F管理ツールのインストール

ダウンロードした`U2FMaintenanceTool.pkg`のアイコンを右クリックし「開く」を実行してください。<br>
（2018/05/30現在、アプリに署名がされていないので、アイコンをダブルクリックしても実行することができないための措置になります）

以降は表示された画面の指示に従い、インストールを進めます。

### U2F管理ツールの設定

パッケージ化されたChrome U2Fエクステンションが、 U2F管理ツール（エクステンションID修正版）で使用できるようにします。<br>
下図画面の「Chrome設定」をクリックします。

<img src="assets/0014.png" width="450">

処理が成功すれば、パッケージ化されたChrome U2Fエクステンションが、U2F管理ツール（エクステンションID修正版）を呼び出せるようになります。

## Chrome上での操作

あらかじめOne Cardを起動した後、Chromeブラウザーを開き、U2FローカルテストサーバーでRegister／Authenticateを実行します。

### U2F Registerの実行

ローカルテストサーバーをChromeブラウザーで開き、ユーザー名欄に入力後「認証器を登録する」をクリックします。

<img src="assets/0015.png" width="600">

U2F Registerが成功します。

<img src="assets/0016.png" width="600">

### U2F Authenticateの実行

「認証器を使って認証する」をクリックします。その後、One Card上のMAIN SWを１回押します。

<img src="assets/0017.png" width="600">

U2F Authenticateが成功します。

<img src="assets/0018.png" width="600">
