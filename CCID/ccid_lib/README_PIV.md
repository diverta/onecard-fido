# PIVカードエミュレーション

最終更新日：2021/3/11

## 概要
PIVカードと同等の機能（PIVカードエミュレーション機能）をnRF52840上に実装するためのモジュールです。

プラットフォーム（nRF5 SDK）に依存する部分と、依存しない部分に分かれています。

## 前提要件

#### 専用CCIDドライバーのインストール（macOSのみ）

PIVカードエミュレーション機能は、USB CCIDインターフェース上で動作します。<br>
他方、macOSに標準導入されているCCIDインターフェースは、[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)をサポートしておりません。

したがって、macOS環境でPIVカードエミュレーション機能を利用するためには、[専用CCIDドライバー](../../CCID/INSTALLPRG.md)[注1]がインストールされている必要があります。

[注1] macOS環境上で、MDBT50Q DongleをUSB CCIDデバイスとして認識できるよう、カスタマイズされています。詳細につきましては[別ドキュメント](../../CCID/ccid_lib/README.md)をご参照願います。

## 構成

#### モジュール一覧（プラットフォーム非依存）

|#|モジュール名|説明|
|:---:|:---|:---|
|1-1|`ccid.h`|CCID関連で共通利用するヘッダー|
|1-2|`ccid_apdu.c/.h`|リクエストAPDU解析、レスポンスAPDU生成を行います。|
|1-3|`ccid_main.c/.h`|エントリーモジュール|
|1-4|`ccid_piv.c/.h`|PIVに関する業務処理を実行します。|
|1-5|`ccid_piv_authenticate.c/.h`|PIVの各種認証処理を実行します。|
|1-6|`ccid_piv_general_auth.c/.h`|PIV認証処理のエントリーモジュール|
|1-7|`ccid_piv_object.c/.h`|PIVで使用する各種オブジェクトを管理します。|
|1-8|`ccid_piv_object_import.c/.h`|PIVで使用する各種オブジェクトのインポート処理を実行します。|
|1-9|`ccid_piv_pin.c/.h`|PIN認証処理を実行します。|
|1-10|`ccid_piv_pin_auth.c/.h`|PINおよびリトライカウンターのチェック処理を実行します。|
|1-11|`ccid_piv_pin_update.c/.h`|PINおよびリトライカウンターの更新処理を実行します。|
|1-12|`ccid_ykpiv.c/.h`|Yubico PIV Tool固有の業務処理を実行します。|
|1-13|`ccid_ykpiv_import_key.c/.h`|Yubico PIV Toolの鍵インポート処理を実行します。|

#### モジュール一覧（プラットフォーム依存）

|#|モジュール名|説明|
|:---:|:---|:---|
|2-1|`ccid_crypto.c/.h`|RSA-2048に関する処理を実行します。|
|2-2|`ccid_flash_piv_object.c/.h`|各種PIVオブジェクトを、MDBT50Q Dongle内の<br>Flash ROM上で管理します。|
|2-3|`usbd_service.c/.h`|USB経由で通信する各種I/F(CCID、HID、CDC)への<br>振分処理を実行します。|
|2-4|`usbd_service_bos.c/.h`|BOSディスクリプター応答に関する処理を実行します。<br>[注1]|
|2-5|`usbd_service_ccid.c/.h`|CCID I/F経由でデータ送受信処理を実行します。|

[注1]一応実装はしましたが、BOSディスクリプター応答は当面サポートしない見込みです。最終更新日の時点での実装では、BOSディスクリプター応答を行わなくても、macOS／Windows 10の両環境で、CCID／PIVデバイスとして認識されるようです。

## 仕様
PIVに関する仕様は下記の通りです。

#### 実行可能命令
本モジュールで実行できるPIV機能は、下記一覧の通りとなっております。<br>
（最終更新日の時点で、[nRF52840アプリケーション](../../nRF52840_app)に実装されているもの）

|#|INS|名称|説明|
|:---:|:---|:---|:---|
|1|`0xA4`|`PIV_INS_SELECT`|PIV Appletを実行可能化|
|2|`0xCB`|`PIV_INS_GET_DATA`|PIVオブジェクトを取得[注5]|
|3|`0x87`|`PIV_INS_GENERAL_AUTHENTICATE`|PIV認証を実行[注2][注5]|
|4|`0xDB`|`PIV_INS_PUT_DATA`|PIVオブジェクトを転送[注3]|
|5|`0x20`|`PIV_INS_VERIFY`|PIN番号を使用した認証を実行[注4][注5]|
|6|`0xFD`|`YKPIV_INS_GET_VERSION`|バージョン取得[注1]|
|7|`0xF8`|`YKPIV_INS_GET_SERIAL`|Serial number取得[注1]|
|8|`0xFF`|`YKPIV_INS_SET_MGMKEY`|管理用パスワード登録[注1]|
|9|`0xFE`|`YKPIV_INS_IMPORT_ASYMMETRIC_KEY`|秘密鍵登録[注1]|
|10|`0xFB`|`YKPIV_INS_RESET`|PIVアプリケーションリセット[注1]|

[注1]PIV本来の仕様ではなく、[Yubico PIV Tool](https://developers.yubico.com/yubico-piv-tool/)用の独自コマンドです。<br>
[注2]デフォルト管理用キーを使用した認証機能のみが実装されています。<br>
[注3]転送されたオブジェクトは、nRF52840のFlash ROMに格納されます。<br>
[注4]PIN番号は平文転送されます。<br>
[注5]PIVでは、FIDO同様共通鍵による暗号化オプションをサポートするようですが、最終更新日の時点で、本プロジェクトでは未実装です。後日、追加実装を検討いたします。

#### 管理対象PIVオブジェクト
本モジュールで管理対象とするPIVオブジェクト（PIVで使用する各種機密データ）は、下記一覧の通りとなっております。<br>
[管理ツール](../../MaintenanceTool/README.md)によるメンテナンスが出来るようになっております。[注1][注2]

|#|TAG|名称|説明|
|:---:|:---|:---|:---|
|1|`0x9B`|Card administration key|管理用パスワード[注3]|
|2|`0x02`|Card Holder Unique Identifier|デバイスを識別する任意のID|
|3|`0x07`|Card Capability Container|デバイスを識別する任意のID|
|4|`0x9A`|PIV Authentication Key|PIV認証用の秘密鍵|
|5|`0x9C`|Digital Signature Key|デジタル署名用の秘密鍵|
|6|`0x9D`|Key Management Key|キー管理用の秘密鍵|
|7|`0x05`|X.509 Certificate for PIV Authentication|PIV認証用の証明書|
|8|`0x0A`|X.509 Certificate for Digital Signature|デジタル署名用の証明書|
|9|`0x0B`|X.509 Certificate for Key Management|キー管理用の証明書|
|10|`0x80`|PIV Card Application PIN|PIN番号[注4]|
|11|`0x81`|PIN Unblocking Key|PUK番号[注4]|

[注1][Yubico PIV Tool](https://developers.yubico.com/yubico-piv-tool/)や、[YubiKey Manager](https://www.yubico.com/products/services-software/download/yubikey-manager/)といった無償ツールを使用して、インストールすることも可能となっています。<br>
[注2]インストールされたオブジェクトは、nRF52840のFlash ROMに格納されます。<br>
[注3]TDEA（Triple Data Encryption Algorithm）暗号のみサポートしています。<br>
[注4]共に固定長（8バイト）となっております。PIN／PUK番号が8バイトに満たない場合、残りの領域は`0xff`で埋められます。

#### CHUID（Card Holder Unique Identifier）
本プロジェクトでCHUIDとして設定する値は、下記の通りダミー値になります。

|#|TAG|名称|内容|意味|
|:---:|:---|:---|:---|:---|
|1|`0x30`|FASC-N|別掲[注1]|ダミーFASC-N|
|2|`0x34`|Global Unique Identifier|HEX文字列[注2]|PIVカードの固有番号|
|3|`0x35`|Expiration Date|`YYYYMMDD`形式[注3]|PIVカードの有効期限|
|4|`0x3e`|Issuer Asymmetric Signature|（設定していません）|署名|
|5|`0xfe`|Error Detection Code|（設定していません）|LRC|

<b>ダミーFASC-Nの内容</b>

|#|名称|内容|意味|
|:---:|:---|:---|:---|
|1|Agency Code|`9999`|ダミー値|
|2|System Code|`9999`|ダミー値|
|3|Credential Number|`999999`|ダミー値|
|4|Credential Series|`0`|ダミー値|
|5|Individual Credential Issue|`1`|ダミー値|
|6|Person Identifier|`0000000000`|ダミー値|
|7|Organizational Category|`3`|3=Commercial Enterprise|
|8|Organizational Identifier|`0000`|ダミー値|
|9|Person/Organization Association Category|`1`|1=Employee|

[注1]本プロジェクトではダミー値を設定しています（内容は「ダミーFASC-Nの内容」をご参照）。<br>
[注2]長さ16バイトのIPv6アドレスです。本プロジェクトでは「CHUID設定機能」でランダム値を設定しています。<br>
[注3]長さ8バイトの半角数字です（`20201201`といった値）。本プロジェクトでは「CHUID設定機能」実行時の1年後の日付を設定しています。

#### CCC（Card Capability Container）
本プロジェクトでCCCとして設定する値は、下記の通りダミー値になります。

|#|TAG|名称|内容|意味|
|:---:|:---|:---|:---|:---|
|1|`0xf0`|Card Identifier|別掲[注1]|ダミーCardID|
|2|`0xf1`|Capability Version|`0x21`|GSC-IS v2.1|
|3|`0xf2`|Capability Grammar Version|`0x21`|GSC-IS v2.1|
|4|`0xf3`|ApplicationsCardURL|（設定していません）||
|5|`0xf4`|PKCS#15|`0x00`|PKCS#15でない|
|6|`0xf5`|Registered Data Model|`0x10`|The data model of the<br> PIV Card Application|
|7|`0xf6`|Access Control Rule Table|（設定していません）||
|8|`0xf7`|Card APDUs|（設定していません）||
|9|`0xfa`|Redirection Tag|（設定していません）||
|10|`0xfb`|Capability Tuples|（設定していません）||
|11|`0xfc`|Status Tuples|（設定していません）||
|12|`0xfd`|Next CCC|（設定していません）||
|13|`0xfe`|Error Detection Code|（設定していません）||

<b>ダミーCardIDの内容</b>

|#|名称|内容|意味|
|:---:|:---|:---|:---|
|1|GSC-RID|`a0 00 00 01 16`|GSC-ISのRID|
|2|manufacturer-Id|`0xff`|ダミー値|
|3|Card-type|`0x02`|2=JavaCard|
|4|Card-Id|HEX文字列[注2]|ダミー値|

[注1]本プロジェクトではダミー値を設定しています（内容は「ダミーCardIDの内容」をご参照）。<br>
[注2]長さ14バイトです。本プロジェクトでは「CHUID設定機能」でランダム値を設定しています。
