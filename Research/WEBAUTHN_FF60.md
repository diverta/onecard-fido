# Firefox 60のWebAuthnについて

Firefox 60で正式サポートしているWebAuthnについて、各種調査を行いました。

## FirefoxのWebAuthn関連設定

下図は、Firefoxの環境設定画面（about:config）を開き、Web Authentication関連の設定を検索したところです。

<img src="assets01/0001.png" width="700">

U2F関連の旧い設定と思しき「security.webauth.U2F」はデフォルトでfalse（使用不可）になっており、代わりに、新しいWebAuthn設定である「security.webauth.webauthn」がデフォルトでtrue（使用可能）になっています。

そのうえで、Yubikey NEOをはじめとするUSB HIDトークンが「security.webauth.webauthn_enable_usbtoken」の設定により使用可能となっているようです。<br>
こちらもデフォルトでtrue（使用可能）に設定されております。

## デモサイトとYubikey NEOを使った確認

Firefox 60でデモサイトとYubikey NEOを使い、WebAuthnによるユーザー登録と認証ができることを確認しました。

### デモサイトのページ

Yubikey NEOをUSBポートに挿したのち、Firefoxを起動します。<br>
起動したら、WebAuthnデモサイト（[webauthn.io](https://webauthn.io/)）を開きます。

下図のような画面が表示されます。

<img src="assets01/0002.png" width="500">

### ユーザー登録

画面中央のテキストボックスに、登録したいユーザー名（半角英数文字）を入力してから「Register a User/Credential」ボタンをクリックします。

<img src="assets01/0003.png" width="500">

「Registering...」というガイダンスが出たら、あらかじめUSBポートに挿しておいたYubikey NEOを指でタッチします。

<img src="assets01/0002.png" width="500">

「You're logged in!」という画面に遷移すれば、ユーザー登録は成功です。

<img src="assets01/0003.png" width="500">

### 認証

画面中央のテキストボックスに、先ほど登録したユーザー名（半角英数文字）を入力してから「Login with Credential」ボタンをクリックします。

<img src="assets01/0004.png" width="500">

「Logging In...」というガイダンスが出たら、あらかじめUSBポートに挿しておいたYubikey NEOを指でタッチします。

<img src="assets01/0005.png" width="500">

「You're logged in!」という画面に遷移すれば、認証は成功です。

<img src="assets01/0006.png" width="500">
