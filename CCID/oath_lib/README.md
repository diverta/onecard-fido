# OATHカードエミュレーション

最終更新日：2022/6/21

## 概要
OATHカードと同等の機能（OATHカードエミュレーション機能）をnRF5340上に実装するためのモジュールです。

プラットフォーム（nRF Connect SDK）に依存する部分と、依存しない部分に分かれています。

## 前提要件

#### 専用CCIDドライバーのインストール（macOSのみ）

OpenPGPカードエミュレーション機能は、USB CCIDインターフェース上で動作します。<br>
他方、macOSに標準導入されているOATHインターフェースは、[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)をサポートしておりません。

したがって、macOS環境でOATHカードエミュレーション機能を利用するためには、[専用CCIDドライバー](../../CCID/INSTALLPRG.md)[注1]がインストールされている必要があります。

[注1] macOS環境上で、MDBT50Q DongleをUSB CCIDデバイスとして認識できるよう、カスタマイズされています。詳細につきましては[別ドキュメント](../../CCID/ccid_lib/README.md)をご参照願います。

## 構成

#### モジュール一覧（プラットフォーム非依存）

|#|モジュール名|説明|
|:---:|:---|:---|
|1-1|`ccid.h`|CCID関連で共通利用するヘッダー|
|1-2|`ccid_apdu.c/.h`|リクエストAPDU解析、レスポンスAPDU生成を行います。|
|1-3|`ccid_main.c/.h`|エントリーモジュール|
|1-4|`ccid_oath_account.c/.h`|アカウント関連処理|
|1-5|`ccid_oath_define.h`|OATHに関する定義体|
|1-6|`ccid_oath_object.c/.h`|データオブジェクト関連処理|
|1-7|`ccid_oath_totp.c/.h`|OATH-TOTP関連処理|
|1-8|`ccid_oath.c/.h`|エントリーモジュール|

#### モジュール一覧（プラットフォーム依存）

|#|モジュール名|説明|
|:---:|:---|:---|
|2-1|`app_rtcc.c/.h`|時刻同期関連|
|2-2|`app_settings.c/.h`|データ永続化関連|
|2-3|`app_usb.c/.h`|USBデバイス管理関連|
|2-4|`app_usb_ccid.c/.h`|CCID I/F経由でデータ送受信処理を実行します。|
|2-5|`ccid_flash_oath_object.c/.h`|各種データオブジェクトを、認証器内のFlash ROM上で管理します。|
