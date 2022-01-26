# nRF5340アプリケーション

最終更新日：2021/10/14

## 概要

現在開発計画中のMDBT53 Dongle（nRF5340）上で稼働するアプリケーションです。<br>
[nRF Connect SDK](https://www.nordicsemi.com/Software-and-tools/Software/nRF-Connect-SDK)を使用し、開発しています。

## 搭載機能

### FIDO機能
[nRF52840アプリケーション](../nRF52840_app/README.md)と等価のFIDO機能（U2F／WebAuthnで使用するFIDO2認証器としての機能）を提供します。

### 管理機能
nRF52840アプリケーションと等価の管理機能を提供します。<br>
FIDO機能で使用する秘密鍵／証明書（Attestation）の導入や、PINコード設定、BLEペアリング実行、ヘルスチェック等の機能があります。

最終更新日現在、いずれも[管理ツール（macOS版）](../MaintenanceTool/macOSApp/README.md)により実行可能となっております。

### [ファームウェア更新機能](../nRF5340_app/DFUFUNC.md)
nRF5340アプリケーションでは、BLE経由、USB経由のファームウェア更新機能（DFU）を用意しています。<br>
開発中／製品化の各局面に応じ、使い分ける事が可能です。

### CCIDインターフェース
nRF52840アプリケーションと等価のCCIDインターフェースを提供します。<br>
最終更新日現在、CCIDインターフェース上で稼働する業務（PIV／OpenPGP機能）は未搭載です。

## ファームウェア

nRF5340アプリケーションのファームウェアは、フォルダー[`firmwares`](../nRF5340_app/firmwares) に格納予定です。

## 開発環境構築手順

以下の手順書をご参照願います。

- <b>[nRF Connect SDKインストール手順書](../nRF5340_app/INSTALLSDK.md)</b>

- <b>[nRF Connect SDK動作確認手順書](../nRF5340_app/CONFIRMSDK.md)</b>

## 技術情報

- <b>[データの永続化について](../nRF5340_app/TECH_ZEP_SETTINGS.md)</b><br>
Zephyrプラットフォームにおけるデータの永続化に関する技術情報を、補足的に掲載しています。

- <b>[ファームウェア更新機能（BLE）](../nRF5340_app/BLEDFU_FUNC_OBJC.md)</b><br>
ZephyrアプリケーションのBLE DFU機能についての技術情報を掲載しています。

- <b>[USB DFU対応に伴うカスタマイズ](../nRF5340_app/CUSTOMIZE.md)</b><br>
USB経由ファームウェア更新（USB DFU）対応のために必要となる、Zephyrプラットフォームのコード修正内容について掲載しています。
