# FIDO2ローカルテストサーバー

## 概要
PC環境で、FIDO 2.0（WebAuthn）サーバー機能をデモするためのツール群です。

macOS上のPython環境下で動作します。

## 構成物件

* <b>python-fido2</b><br>
FIDO2ローカルテストサーバーを格納しているフォルダーです。<br>
Yubico社が無償公開している[「python-fido2」](https://developers.yubico.com/python-fido2/)がベースとなっております。

## 使用環境

### ハードウェア

このサーバー機能のテストには、以下の認証器をご利用ください。<br>

- Yubico セキュリティキー<br>
https://www.amazon.co.jp/Yubico-セキュリティキー-FIDO2-USB-2段階認証/dp/B07BYSB7FK<br>
<img src="assets/0001.png" width="400">

後日対応予定

- [「nRF52840 Dongle」](https://www.mouser.jp/new/nordicsemiconductor/nordic-nrf52840-usb-dongle/)<br>
現在、[USB HIDとBLEによるU2F対応](../nRF5_SDK_v15.2.0)が完了しており、今後はFIDO 2.0（WebAuthn）対応に入る予定です。<br>
<img src="assets/0009.png" width="400">

### ソフトウェア

Chromeブラウザーは、バージョン70以降を使用します。

#### 動作確認済みの環境
macOS Sierra（Version 10.12.6）<br>
Google Chrome（Version 70.0）

なお、以下の物件は不要となります。
- [Chrome U2Fエクステンション](https://github.com/diverta/onecard-fido/tree/master/Research/u2f-ref-code/u2f-chrome-extension)
- [U2F Helper（ヘルパーアプリ）](https://github.com/diverta/onecard-fido/blob/master/Usage/HELPER_INSTALL.md)
- [U2F USB HIDデバイス（ヘルパーデバイス）](https://github.com/diverta/onecard-fido/blob/master/U2FHIDDevice/readme.md)

## 手順書

- <b>[FIDO2ローカルテストサーバー構築手順](FIDO2LOCALSVR.md) </b><br>
Yubico社が無償公開している「python-fido2」をmacOSに導入する手順を掲載しております。

- <b>[WebAuthn確認手順](WEBAUTHNTEST.md) </b><br>
FIDO2ローカルテストサーバーとChromeブラウザー、FIDO 2.0認証器を使用し、WebAuthnの動作を確認する手順を掲載しております。
