# OpenPGPカードエミュレーション

最終更新日：2021/3/11

## 概要
OpenPGPカードと同等の機能（OpenPGPカードエミュレーション機能）をnRF52840上に実装するためのモジュールです。

プラットフォーム（nRF5 SDK）に依存する部分と、依存しない部分に分かれています。

## 前提要件

#### 専用CCIDドライバーのインストール（macOSのみ）

OpenPGPカードエミュレーション機能は、USB CCIDインターフェース上で動作します。<br>
他方、macOSに標準導入されているCCIDインターフェースは、[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)をサポートしておりません。

したがって、macOS環境でOpenPGPカードエミュレーション機能を利用するためには、[専用CCIDドライバー](../../CCID/INSTALLPRG.md)[注1]がインストールされている必要があります。

[注1] macOS環境上で、MDBT50Q DongleをUSB CCIDデバイスとして認識できるよう、カスタマイズされています。詳細につきましては[別ドキュメント](../../CCID/ccid_lib/README.md)をご参照願います。

## 構成

#### モジュール一覧（プラットフォーム非依存）

|#|モジュール名|説明|
|:---:|:---|:---|
|1-1|ccid_openpgp_attr.h|属性データ関連処理|
|1-2|ccid_openpgp_crypto.c/.h|署名／復号化処理|
|1-3|ccid_openpgp_data.c/.h|鍵／データオブジェクト更新処理|
|1-4|ccid_openpgp_key_rsa.c/.h|RSA鍵関連処理|
|1-5|ccid_openpgp_key.c/.h|鍵関連処理|
|1-6|ccid_openpgp_object.c/.h|データオブジェクト関連処理|
|1-7|ccid_openpgp_pin.c/.h|PIN認証処理のエントリーモジュール|
|1-8|ccid_openpgp.c/.h|エントリーモジュール|
|1-9|ccid_pin_auth.c/.h|PINおよびリトライカウンター関連処理|
|1-10|ccid_pin.h|PINに関する定義体|

#### モジュール一覧（プラットフォーム依存）

|#|モジュール名|説明|
|:---:|:---|:---|
|2-1|ccid_crypto.c/.h|RSA-2048に関する処理を実行します。|
|2-2|ccid_flash_openpgp_object.c/.h|各種データオブジェクトを、MDBT50Q Dongle内の<br>Flash ROM上で管理します。|
|2-3|usbd_service.c/.h|USB経由で通信する各種I/F(CCID、HID、CDC)への<br>振分処理を実行します。|
|2-4|usbd_service_bos.c/.h|BOSディスクリプター応答に関する処理を実行します。<br>[注1]|
|2-5|usbd_service_ccid.c/.h|CCID I/F経由でデータ送受信処理を実行します。|

[注1]一応実装はしましたが、BOSディスクリプター応答は当面サポートしない見込みです。最終更新日の時点での実装では、BOSディスクリプター応答を行わなくても、macOS／Windows 10の両環境で、CCID／OpenPGPデバイスとして認識されるようです。

## 仕様
OpenPGPに関する仕様は以下になります。

#### 実行可能命令
本モジュールで実行できるOpenPGP機能は、下記一覧の通りとなっております。<br>
（最終更新日の時点で、[nRF52840アプリケーション](../../nRF52840_app)に実装されているもの）

|#|INS|名称|説明|
|:---:|:---|:---|:---|
|1|`0x20`|OPENPGP_INS_VERIFY|PIN番号を使用した認証を実行[注1][注2]|
|2|`0x2A`|OPENPGP_INS_PSO|署名／復号化を実行|
|3|`0x44`|OPENPGP_INS_ACTIVATE|OpenPGP機能を有効化|
|4|`0x47`|OPENPGP_INS_GENERATE_ASYMMETRIC_KEY_PAIR|公開鍵を参照または取得|
|5|`0xA4`|OPENPGP_INS_SELECT|Appletを実行可能化|
|6|`0xCA`|OPENPGP_INS_GET_DATA|データオブジェクトを取得|
|7|`0xDA`|OPENPGP_INS_PUT_DATA|データオブジェクトを転送[注3]|
|8|`0xDB`|OPENPGP_INS_IMPORT_KEY|秘密鍵インポート[注3][注4]|
|9|`0xE6`|OPENPGP_INS_TERMINATE|OpenPGP機能を無効化|

[注1]デフォルト管理用キーを使用した認証機能のみが実装されています。<br>
[注2]PIN番号は平文転送されます。<br>
[注3]転送されたデータは、nRF52840のFlash ROMに格納されます。上書きが可能です。<br>
[注4]ひとたびインポートされた秘密鍵を参照または取得することは出来ません。<br>

#### 管理対象データオブジェクト
本モジュールで管理対象とするデータオブジェクト（OpenPGPで使用する各種機密データ）は、下記一覧の通りとなっております。[注1]

|#|TAG|名称|説明|
|:---:|:---|:---|:---|
|1|`0x01`|TAG_OPGP_PW1|認証用PIN番号[注2][注4]|
|2|`0x02`|TAG_OPGP_PW3|管理用PIN番号[注2][注5]|
|3|`0x03`|TAG_OPGP_RC|リセット用PIN番号[注2]|
|4|`0xFC`|TAG_ATTR_TERMINATED|OpenPGP機能無効化フラグ|
|5|`0xC4`|TAG_PW_STATUS|PWステータス|
|6|`0xC1`|TAG_ALGORITHM_ATTRIBUTES_SIG|鍵アルゴリズム属性（署名鍵）[注3]|
|7|`0xC2`|TAG_ALGORITHM_ATTRIBUTES_DEC|鍵アルゴリズム属性（暗号化鍵）[注3]|
|8|`0xC3`|TAG_ALGORITHM_ATTRIBUTES_AUT|鍵アルゴリズム属性（認証鍵）[注3]|
|9|`0x04`|TAG_KEY_SIG_STATUS|鍵ステータス（署名鍵）|
|10|`0x05`|TAG_KEY_DEC_STATUS|鍵ステータス（暗号化鍵）|
|11|`0x06`|TAG_KEY_AUT_STATUS|鍵ステータス（認証鍵）|
|12|`0x07`|TAG_KEY_SIG|鍵ペア（署名鍵）|
|13|`0x08`|TAG_KEY_DEC|鍵ペア（暗号化鍵）|
|14|`0x09`|TAG_KEY_AUT|鍵ペア（認証鍵）|
|15|`0xC7`|TAG_KEY_SIG_FINGERPRINT|鍵指紋（署名鍵）|
|16|`0xC8`|TAG_KEY_DEC_FINGERPRINT|鍵指紋（暗号化鍵）|
|17|`0xC9`|TAG_KEY_AUT_FINGERPRINT|鍵指紋（認証鍵）|
|18|`0xCE`|TAG_KEY_SIG_GENERATION_DATES|鍵作成日（署名鍵）|
|19|`0xCF`|TAG_KEY_DEC_GENERATION_DATES|鍵作成日（暗号化鍵）|
|20|`0xD0`|TAG_KEY_AUT_GENERATION_DATES|鍵作成日（認証鍵）|
|21|`0x93`|TAG_DIGITAL_SIG_COUNTER|署名カウンター|

[注1]内部生成／インストールされたオブジェクトは、nRF52840のFlash ROMに格納されます。<br>
[注2]将来的に、[管理ツール](../../MaintenanceTool/README.md)のようなGUIツールによるメンテナンスが出来るようにすることを検討しています。<br>
[注3]RSA-2048のみサポートしています。<br>
[注4]可変長（最大64バイト）となっておりますが、最終更新日の時点で、デフォルトのPIN番号（`123456`）を変更する機能は未実装です。<br>
[注5]可変長（最大64バイト）となっておりますが、最終更新日の時点で、デフォルトのPIN番号（`12345678`）を変更する機能は未実装です。
