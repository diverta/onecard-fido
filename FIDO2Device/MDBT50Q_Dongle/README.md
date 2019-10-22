# MDBT50Q Dongle（rev2）

## 概要

日本国内の技適取得済みであるnRF52840搭載モジュール「MDBT50Q」を使用し、nRF52840 Dongleとほぼ同じ仕様で製作した、USBドングル基板です。

<img src="assets/0021.jpg" width="500">

### [MDBT50Q Dongleの概要](HWSUMMARY_2.md)

基板、動作についての概要を説明しています。

### [MDBT50Q Dongle回路図](pcb_rev2/FIDO2AUTH_002.pdf)

nRF52840 Dongleをベースとし、電池電源の増設と、LEDの増設、若干の配線変更を行っております。

## 使用ファームウェア

MDBT50Q Dongleのファームウェアは、本プロジェクトで開発中の[FIDO2認証器アプリケーション](../../nRF5_SDK_v15.3.0/README.md)をご使用ください。

### [アプリケーション書込み手順](APPINSTALL.md)

MDBT50Q Dongleにプレインストールされている[簡易USBブートローダー](../../nRF5_SDK_v15.3.0/examples/dfu/README.md)を使用して、MDBT50Q Dongleに[FIDO2認証器アプリケーション](../../nRF5_SDK_v15.3.0/README.md)を書き込む手順を掲載しています。

### [簡易USBブートローダー（ご参考）](../../nRF5_SDK_v15.3.0/examples/dfu/README.md)

PCのUSBポート経由で、MDBT50Q Dongleに[FIDO2認証器アプリケーション](../../nRF5_SDK_v15.3.0/README.md)を書き込むことができるようにするためのファームウェアです。<br>
MDBT50Q Dongleにはすでに導入済みとなっております。

## 動作確認手順

### [Edgeブラウザーを使用したWebAuthnテスト手順（Windows10 PC）](WEBAUTHNTEST.md)

MDBT50Q Dongleと、Edgeブラウザー、デモサイトを使用して、WebAuthnのユーザー登録／ログイン（MakeCredential／GetAssertion）の動作確認をする場合の手順を掲載しています。

### [Googleアカウントのログイン確認手順（PC）](PCCHROME.md)

MDBT50Q Dongleと、Googleアカウント、Chromeブラウザーを使用して、U2F Register／Authenticateの動作確認をする場合の手順を掲載しています。

## デモ機能

BLEセントラルサービスを使用したデモンストレーション機能です。

### [RSSIログ出力](DEMOFUNC_1.md)

MDBT50Q Dongleに近接しているBLEデバイスのRSSI値を、指定間隔（１秒〜９秒）ごとに仮想COMポートにログ出力を行います。

### [BLEデバイスによる自動認証](DEMOFUNC_2.md)

FIDO認証（WebAuthn／U2F）実行時、MDBT50Q Dongle上のボタンを押す代わりに、One CardなどのBLEデバイスを近づけることにより、認証処理を自動的に続行させます。
