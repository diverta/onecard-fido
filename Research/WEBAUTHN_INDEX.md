# WebAuthn関連調査

FIDO 2.0のWeb認証技術である、WebAuthnについて、各種調査を行っています。

## 各種確認

- <b>[Firefox 60のWebAuthnについて](WEBAUTHN_FF60.md) </b><br>
[WebAuthnデモサイト](https://webauthn.io)とFirefox 60を使用し、WebAuthnを試行しました。<br>
サポートされているUSBトークンである「Yubikey NEO」を使用し、WebAuthnの一連処理（Register〜Login）が正しく動作することを確認しました。

- <b>[WebAuthn実装の内部調査](WEBAUTHN_GECKO.md) </b><br>
各種ソースコードを参照し、現状のBLEについてのサポート状況確認という観点で調査しました。<br>
FirefoxのAuthenticate関連実装については、Geckoエンジンのコードを調査しました。<br>
（最近置換えが進んでいるというServoエンジンの方も確認しましたが、Authenticate関連のコードはなかったので、Geckoエンジンに実装されているものを使用しているという判断のもと、調査を行いました）

- <b>[WebAuthnローカルテストサーバー構築手順](WEBAUTHN_LOCAL_TSTSVR.md) </b><br>
テスト用ローカルサーバーを構築し、WebAuthnを試行しました。<br>
[デモサイト](https://webauthn.io)とまったく同じオペレーション結果が、ローカルPCで再現できるようです。<br>
（ただしプロトコルはHTTPです。通信内容は暗号化されていません）<br>
サーバーソフトのコードを修正すれば、ログ採りもできるので、FIDO 2.0対応の本格開発時に、役に立つかと存じます。

## ご参考

- <b>[Apache ローカルSSLサーバー構築手順](APACHE_LOCAL_TSTSVR.md) </b><br>
[WebAuthnデモサイト](https://webauthn.io)と同じように、HTTPSプロトコルによる通信が可能なローカルSSLサーバーを構築する手順をまとめています。

- <b>[ChromeのWebAuthnについて](WEBAUTHN_CHROME.md) </b><br>
[WebAuthnデモサイト](https://webauthndemo.appspot.com/)とChrome 66（現在の公式最新バージョン）を使用し、WebAuthnを試行しました。<br>
サポートされているUSBトークンである「Yubikey NEO」を使用しましたが、未サポートの旨のメッセージが表示されてしまいます。
