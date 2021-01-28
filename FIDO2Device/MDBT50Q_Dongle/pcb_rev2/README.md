# MDBT50Q Dongle（rev2）

## 概要

日本国内の技適取得済みであるnRF52840搭載モジュール「MDBT50Q」を使用し、nRF52840 Dongleとほぼ同じ仕様で製作した、USBドングル基板です。

<img src="assets/0021.jpg" width="500">

### [MDBT50Q Dongleの概要](HWSUMMARY.md)

基板、動作についての概要を説明しています。

### [MDBT50Q Dongle回路図](FIDO2AUTH_002.pdf)

nRF52840 Dongleをベースとし、電池電源の増設と、LEDの増設、若干の配線変更を行っております。

### [nRF52840アプリケーション](../../../nRF52840_app/README.md)

MDBT50Q Dongleで使用するファームウェアです。

### [ファームウェア更新手順](../../../MaintenanceTool/macOSApp/UPDATEFIRMWARE.md)

MDBT50Q Dongleのファームウェアを、[FIDO認証器管理ツール（macOS版）](../../../MaintenanceTool/macOSApp)により更新する手順について説明しています。

## 動作確認手順

### [Edgeブラウザーを使用したWebAuthnテスト手順（Windows10 PC）](../WEBAUTHNTEST.md)

MDBT50Q Dongleと、Edgeブラウザー、デモサイトを使用して、WebAuthnのユーザー登録／ログイン（MakeCredential／GetAssertion）の動作確認をする場合の手順を掲載しています。

### [Googleアカウントのログイン確認手順（PC）](../PCCHROME.md)

MDBT50Q Dongleと、Googleアカウント、Chromeブラウザーを使用して、U2F Register／Authenticateの動作確認をする場合の手順を掲載しています。

## オプション機能

BLEセントラルサービスを使用したオプション機能です。

### [BLE近接認証](../BLEDAUTH.md)

FIDO認証（WebAuthn／U2F）実行時、MDBT50Q Dongle上のボタンを押す代わりに、スマートフォンなどのBLEデバイスを近づけることにより、認証処理を自動的に続行させます。
