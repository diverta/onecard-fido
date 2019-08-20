# One Card FIDO対応

One CardにFIDO U2F／WebAuthn認証機能を実装するプロジェクトです。

## プログラム

- <b>[nRF52840版 FIDO2認証器](nRF5_SDK_v15.3.0)</b><br>
Nordic社のSoC「nRF52840」を使用した、FIDO U2F／WebAuthn認証器のファームウェアです。<br>
[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)をはじめ、nRF52840 DK（開発ボード）、nRF52840 Dongle（USBドングル）に対応しています。

- <b>[FIDO認証器管理ツール](MaintenanceTool)</b><br>
FIDO2認証器に、鍵・証明書・PINを導入するために使用する、デスクトップ・ツールです。<br>
[Windows版](MaintenanceTool/WindowsExe)、[macOS版](MaintenanceTool/macOSApp)の両方を用意しております。

## ハードウェア

- <b>[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)</b><br>
Nordic社のSoC「nRF52840」を使用した、FIDO U2F／WebAuthn認証器です。<br>
日本国内の技適取得済みであるnRF52840搭載モジュール「MDBT50Q」を使用し、nRF52840 Dongleとほぼ同じ仕様で製作しております。<br>
PCのUSBポート装着時はUSB HIDデバイスとして稼働し、ボタン乾電池装着時はBLEペリフェラルデバイスとして稼働します。

## 新しい試み

- <b>[mbed OS版 FIDO2.0認証器](STM32F411RE)</b><br>
ST社のマイコン「STM32F411RE」を使用した、FIDO U2F／WebAuthn認証器です。<br>
mbed OSへのアプリケーション移植のほか、NFCタグIC（自己発電機能あり）、セキュアICを導入する新しい試みになります。

## What's new

#### 2019/08/19（Version 0.1.15）

FIDO認証器管理ツール、およびファームウェアを修正しました。<br>

- [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp/MaintenanceTool.pkg)
- [Windows版 FIDO認証器管理ツール](MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)
- [nRF52840版 FIDO2認証器](nRF5_SDK_v15.3.0)

主な修正点は以下になります。
- PINGテスト機能を追加（[#224](https://github.com/diverta/onecard-fido/pull/224)、[#225](https://github.com/diverta/onecard-fido/pull/225) ご参照）
- ボタン長押し判定方法の見直し（[#222](https://github.com/diverta/onecard-fido/pull/222) ご参照）
- 一部ソースコードのリファクタリング（[#215](https://github.com/diverta/onecard-fido/pull/215) ご参照）

#### [過去の更新履歴はこちら](HISTORY.md)

## FIDO2について

#### CTAP2とは

FIDOの新世代パスワードレス認証（<b>WebAuthn</b>）に対応するために用意された、FIDO 2.0の技術仕様です。

[nRF52840版 FIDO2認証器](nRF5_SDK_v15.2.0)では、既にUSB HIDトランスポート、BLEトランスポートに対応しています。<br>
NFCトランスポートは、後日対応予定です。

また、Windows環境（Edgeブラウザー）でのWebAuthnは、PINコード（暗証番号）入力が必須となるのですが、こちらの方もすでに対応済みとなっております。

Windows環境による具体的なテスト方法は、別途手順書[「Edgeブラウザーを使用したWebAuthnテスト手順」](Research/FIDO_2_0/EDGETEST.md)をご参照ください。

## 以前の仕様

以下の項目は、FIDOの旧世代２要素認証（U2F）に関する開発物件になります。<br>
[nRF52840版 FIDO2認証器](nRF5_SDK_v15.2.0)においては、U2FはCTAP2と同居していますので、現在も稼働させることができます。

ただし、U2FはChromeブラウザーのみのサポートであり、かつ将来的にサポートが拡張される予定もないため、現在はメンテナンスをストップさせております。<br>
何卒ご容赦ください。

#### BLE U2Fサービス

One CardのBLE上で稼働するFIDO U2F機能です。<br>
U2F v1.2の仕様に準拠しております。

FIDO U2Fに関する情報 : https://fidoalliance.org/download/

FIDO U2F対応デバイスは、USBポートに挿して使用する「YubiKey」が有名ですが、BLE U2Fサービスは、USBポートではなく、One CardのBLEを使用しています。

コード格納場所--->[nRF5_SDK_v13.0.0](nRF5_SDK_v13.0.0)

#### U2F管理ツール

PC環境から、BLE U2Fサービスの動作に必要な鍵・証明書の導入などを行うツールです。<br>
macOS版と、Windows版を用意しました。

コード格納場所--->[U2FMaintenanceTool](U2FMaintenanceTool)

## 各種情報

- [運用に関する情報](Usage/README.md)

- [開発に関する情報](Development/README.md)

- [調査に関する情報](Research/README.md)

- [障害に関する情報](https://github.com/diverta/onecard-fido/issues)
