# ChromeのWebAuthnについて

Chrome 67以降で正式サポート予定のWebAuthnについて、各種調査を行います。

## デモサイトとYubikey NEOを使った確認

Chrome 67（現在の公式最新バージョン）で、[デモサイト](https://webauthndemo.appspot.com/)とYubikey NEOを使い、WebAuthnによるユーザー登録と認証を行いました。

### ユーザー登録

デモサイトページ上部の「REGISTER NEW CREDENTIAL」をクリックします。

<img src="assets02/0001.png" width="700">

ボタン下部のバーに「Waiting for user touch」というメッセージが表示されたら、Yubikey NEOに指を１回タッチします。

<img src="assets02/0002.png" width="700">

ボタン下部のバーに「Successfully created credential」というメッセージが表示されたら、ユーザー登録は成功です。

<img src="assets02/0003.png" width="700">

### 認証

デモサイトページ上部の「AUTHENTICATE」をクリックします。

<img src="assets02/0003.png" width="700">

ボタン下部のバーに「Waiting for user touch」というメッセージが表示されたら、Yubikey NEOに指を１回タッチします。

<img src="assets02/0004.png" width="700">

ボタン下部のバーに「Successful assertion」というメッセージが表示されたら、ユーザー登録は成功です。

<img src="assets02/0005.png" width="700">
