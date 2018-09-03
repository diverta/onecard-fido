# Googleアカウントを使ったテスト手順

[Googleアカウントの２段階認証ページ](https://myaccount.google.com/signinoptions/two-step-verification/enroll-welcome)を使用して、One CardのU2F機能（Register/Authenticate）をテストする手順を、以下に掲載いたします。

なお、本手順に掲載している画面イメージは、macOS環境のものですが、手順自体はWindows環境でも同様になります。

## テスト実行前の準備

### Chromeブラウザーについて

テスト事前に、Chromeブラウザーをインストールします。<br>
（インストール済みの場合は不要です）

Googleアカウントの２段階認証は、Chromeブラウザーで標準サポートされるU2F機能を使用しますので、別段のエクステンションは不要です。

### ヘルパーデバイス／ヘルパーアプリについて

Googleアカウントの２段階認証を実行させるためには、[ヘルパーデバイス（U2F USB HIDデバイス）](../U2FHIDDevice/readme.md)が必要となります。

<img src="../U2FHIDDevice/assets/0001.png" width="400">


また、Chromeブラウザーとヘルパーデバイス間連携のため、[ヘルパーアプリ（U2F Helper）](HELPER_INSTALL.md)も必要となります。<br>
ヘルパーアプリのインストール手順は下記のページに掲載しております。

- [U2F Helperインストール手順](HELPER_INSTALL.md)

### One Cardについて

テスト事前に、One Cardをシステム（macOS／Windows）とペアリングさせてください。<br>
ペアリングの手順は、[こちらのページ（One Cardペアリング手順）](PAIRING.md)に掲載しております。

### U2F管理ツールと鍵・証明書について

テスト事前に、U2F管理ツールがシステム（macOS／Windows）にインストールされ、かつ、U2F管理ツールで作成した鍵・証明書がOne Cardにインストールされていることを確認してください。

インストール手順は下記のページに掲載しております。

- [鍵・証明書インストール手順（macOS版）](INSTALL.md)
- [鍵・証明書インストール手順（Windows版）](INSTALL_WINDOWS.md)

## テストの実行

以下の順番で実行します。

- U2F Registerの実行
- U2F Authenticateの実行

### 事前確認

事前にヘルパーデバイス（U2F USB HIDデバイス）がPCに挿してあることをご確認ください。

<img src="assets_gat/0019.png" width="500">

また、ヘルパーアプリ（U2F Helper）が起動していることをご確認ください。

<img src="assets_gat/0020.png" width="500">

One Cardがスリープ状態になっている場合、U2F機能の実行が失敗します。<br>
MAIN SWを１回プッシュし、スリープ状態を解除してください。

<img src="assets_gat/0021.png" width="500">

### U2F Registerの実行

Chromeブラウザーで[Googleアカウントの２段階認証ページ](https://myaccount.google.com/signinoptions/two-step-verification/enroll-welcome)を表示させます。<br>
下図のような画面に遷移しますので、画面右下部の「開始」をクリックします。

<img src="assets_gat/0002.png" width="600">

パスワードを求められたら、パスワードを入力し「次へ」をクリックします。

<img src="assets_gat/0003.png" width="270">

テキストメッセージ、音声通話を使用しないので「別の方法を選ぶ」をクリックします。

<img src="assets_gat/0004.png" width="350">

下図のようなポップアップが表示されたら「セキュリティ キー」をクリックします。

<img src="assets_gat/0005.png" width="360">

下図のような画面に遷移しますので、手元に（電源が投入されスリープ解除されている）One Cardを用意し「次へ」をクリックします。

<img src="assets_gat/0006.png" width="350">

U2Fクライアントが起動し、下図のような画面に遷移します。

<img src="assets_gat/0007.png" width="350">

ヘルパーデバイス／ヘルパーアプリとOne Cardが連携し、One Card上で、U2F Register機能が実行されます。<br>
One Card上では、LEDが点灯してRegister処理中であることが表示されます。


<img src="assets_gat/0011.png" width="400">

しばらくすると、下図のような画面に遷移します。<br>

<img src="assets_gat/0008.png" width="350">

任意のセキュリティ キー名を入力して「完了」をクリックします。

<img src="assets_gat/0009.png" width="350">

２段階認証がセキュリティ キーで有効化された旨が表示されます。

<img src="assets_gat/0010.png" width="600">

これでU2F Registerは完了です。

後述のU2F Authenticateテストのため、Chromeブラウザーからはログアウトしてください。<br>
設定ページからGoogleアカウント右横の「ログアウト」をクリックすると、ログアウトできます。

<img src="assets_gat/0014.png" width="600">


### U2F Authenticateの実行

先ほど使用したGoogleアカウントにより、Chromeブラウザーにログインします。<br>
下図画面で「Chromeにログイン」をクリックします。

<img src="assets_gat/0015.png" width="700">

２段階認証プロセスの画面に遷移します。

<img src="assets_gat/0016.png" width="300">

ヘルパーデバイス／ヘルパーアプリとOne Cardが連携し、One Card上で、U2F Authenticate機能が実行されます。<br>
One Cardを見ると、一番右側のLEDが<font color=ff0000><b>点滅</b></font>しているのが確認できます。<br>
（ユーザー所在確認を求めるため、One Card側の処理が一時的に中断されます）

<img src="../assets/0078.png" width="450">

ここでMAIN SWを１回押しますと、One Card側の処理が再開されます。

<img src="../assets/0079.png" width="450">

One Cardでの実行が完了すると、Chromeブラウザーの画面が再表示されます。

<img src="assets_gat/0017.png" width="700">

下図のような画面に遷移し、Googleアカウントによるログインが行われたことが確認できます。

<img src="assets_gat/0018.png" width="700">


これでU2F Authenticateは完了です。
