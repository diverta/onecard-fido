# MDBT50Q Dongle

## 概要

日本国内の技適取得済みであるnRF52840搭載モジュール「MDBT50Q」を使用し、nRF52840 Dongleとほぼ同じ仕様で製作した、USBドングル基板です。

<img src="assets/0000.jpg" width="500">

MDBT50Q Dongleの回路図はこちら --> <b>[FIDO2AUTH_001.pdf](https://github.com/diverta/onecard-fido/blob/master/FIDO2Device/pcb/FIDO2AUTH_001.pdf)</b>

## 使用ファームウェア

MDBT50Q Dongleは、回路がnRF52840 Dongleと同等ですので、本プロジェクトで開発中の[FIDO2認証器アプリケーション](https://github.com/diverta/onecard-fido/tree/master/nRF5_SDK_v15.3.0)がそのまま利用できます。

アプリケーションの書込み手順はこちら --> <b>[アプリケーション書込み手順](APPINSTALL.md)</b>

## 動作確認手順

### WebAuthn

MDBT50Q Dongleと、Edgeブラウザー、デモサイトを使用して、WebAuthnのユーザー登録／ログイン（MakeCredential／GetAssertion）の動作確認をする場合は、以下の手順で進めるようにします。

- <b>[Edgeブラウザーを使用したWebAuthnテスト手順](WEBAUTHNTEST.md)</b>
