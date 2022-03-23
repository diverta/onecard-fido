# nRF5340アプリケーション

最終更新日：2022/3/15

## 概要

現在開発中の[BT40 Slim Board](https://github.com/diverta/onecard-fido/tree/master/FIDO2Device/BT40SlimBoard)上で稼働するアプリケーションです。<br>
[nRF Connect SDK v1.9.1](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.9.1/nrf/)を使用し、開発しています。

## 搭載機能

### FIDO機能
[nRF52840アプリケーション](../nRF52840_app/README.md)と等価のFIDO機能（U2F／WebAuthnで使用するFIDO2認証器としての機能）を提供します。

### 管理機能
nRF52840アプリケーションと等価の管理機能を提供します。<br>
FIDO機能で使用する秘密鍵／証明書（Attestation）の導入や、PINコード設定、BLEペアリング実行、ヘルスチェック等の機能があります。

最終更新日現在、いずれも管理ツール（[macOS版](../MaintenanceTool/macOSApp/README.md)／[Windows版](../MaintenanceTool/WindowsExe/README.md)）により実行可能となっております。

### ファームウェア更新機能
nRF5340アプリケーションでは、BLE経由のファームウェア更新機能（DFU）を用意しています。<br>
こちらも、管理ツール（[macOS版](../MaintenanceTool/macOSApp/UPDATEFW_BLE.md)／[Windows版](../MaintenanceTool/WindowsExe/UPDATEFW_BLE.md)）により実行可能となっております。

なお、USB経由のDFUもサポート出来るようですが、対応に伴い[Zephyrプラットフォームのカスタマイズ](../nRF5340_app/CUSTOMIZE.md)が必要となってしまうため、本プロジェクトでは採用は見送っております。

### CCIDインターフェース
nRF52840アプリケーションと等価のCCIDインターフェースを提供します。<br>
最終更新日現在、CCIDインターフェース上で稼働する業務（PIV／OpenPGP機能）は未搭載です。

## ファームウェア

nRF5340アプリケーションのファームウェアは、フォルダー[`firmwares`](../nRF5340_app/firmwares) に格納しています。

## 開発環境構築手順

以下の手順書をご参照願います。

- <b>[nRF Connect SDKインストール手順書](../nRF5340_app/INSTALLSDK.md)</b>

- <b>[nRF Connect SDK動作確認手順書](../nRF5340_app/CONFIRMSDK.md)</b>

## 技術情報

- <b>[nRF5340アプリケーションについて](../nRF5340_app/TECH_APP_SOURCES.md)</b><br>
nRF5340アプリケーションに関する技術情報を掲載しています。

- <b>[nRF5340基板について](../nRF5340_app/TECH_BOARD.md)</b><br>
nRF5340アプリケーションを搭載する基板についての技術情報を掲載しています。

- <b>[データの永続化について](../nRF5340_app/TECH_ZEP_SETTINGS.md)</b><br>
Zephyrプラットフォームにおけるデータの永続化に関する技術情報を、補足的に掲載しています。

- <b>ファームウェア更新機能（BLE）</b><br>
nRF5340アプリケーションのBLE DFU機能についての技術情報を掲載しています。<br>
・[macOS版](../nRF5340_app/BLEDFU_FUNC_OBJC.md)<br>
・[Windows版](../nRF5340_app/BLEDFU_FUNC_CSHARP.md)

- <b>[技術情報補足](../nRF5340_app/TECHNICAL.md)</b><br>
nRF5340や、Zephyrプラットフォームに関する技術情報を、補足的に掲載しています。

## サンプルによる調査

- <b>[BLEペアリングサンプル動作確認手順書](../nRF5340_app/SAMPLE_SCONLY.md)</b><br>
Nordic社から公開されている、BLEペアリングのサンプルアプリ「Bluetooth: Peripheral SC-only」の動作確認手順について掲載しています。
