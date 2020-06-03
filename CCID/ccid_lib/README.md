# CCID I/F関連モジュール

## 概要
セキュリティデバイス用のインターフェースとして使用されている、USB CCIDインターフェースを実装したモジュールです。

## 構成

#### モジュール一覧

|#|関数名|説明|
|:---:|:---|:---|
|1-1|ccid.h|CCID関連で共通利用するヘッダー|
|1-2|ccid_main.c/.h|エントリーモジュール|
|1-3|ccid_apdu.c/.h|リクエストAPDU解析、レスポンスAPDU生成を行います。|
|1-4|ccid_piv.c/.h|PIVに関する業務処理を実行します。|
|1-5|ccid_piv_object.c/.h|PIVで使用するファイルを管理します。|

## 仕様
本モジュール群で実行できるセキュリティー機能は以下になります。<br>
開発中・開発予定のものも含みます。

|#|名称|説明|
|:---:|:---|:---|
|1|PIV<br>(Personal Identification Verification)|macOS等のスマートカード認証をエミュレートする機能。<br>nRF52840をUSBに装着すると、NFCカードリーダーに<br>スマートカードを置いた状態と等価になります。|
|2|OpenPGP|（サポート機能は検討中）|
|3|OATH|（サポート機能は検討中）|

### PIVの仕様
PIVに関する仕様は下記の通りです。

#### 実行可能命令
本モジュールで実行できるPIV機能は、下記一覧の通りとなっております。<br>
（2020/06/03現在、[nRF52840アプリケーション](../../nRF5_SDK_v15.3.0)に実装されているもの）

|#|INS|名称|説明|
|:---:|:---|:---|:---|
|1|`0xA4`|PIV_INS_SELECT|PIV Appletを実行可能化|
|2|`0xCB`|PIV_INS_GET_DATA|PIVオブジェクトを取得|
|3|`0xFD`|PIV_INS_GET_VERSION|バージョン取得|
|4|`0xF8`|PIV_INS_GET_SERIAL|Serial number取得|


#### 管理対象PIVオブジェクト
本モジュールで管理対象とするPIVオブジェクト（PIVで使用する各種機密データ）は、下記一覧の通りとなっております。<br>
[Yubico PIV Tool](https://developers.yubico.com/yubico-piv-tool/)や、[YubiKey Manager](https://www.yubico.com/products/services-software/download/yubikey-manager/)といった無償ツールを使用して、インストールすることが可能です。[注1][注2]

|#|TAG|名称|説明|
|:---:|:---|:---|:---|
|1|(無し)|Serial number|製品を識別する任意の4バイト値|
|2|`0x01`|X.509 Certificate for Card Authentication|（不要ですが管理対象とします）|
|3|`0x02`|Card Holder Unique Identifier|デバイスを識別する任意のID|
|4|`0x05`|X.509 Certificate for PIV Authentication|PIV認証用の証明書|
|5|`0x07`|Card Capability Container|（不要ですが管理対象とします）|
|6|`0x0A`|X.509 Certificate for Digital Signature|署名用の証明書|
|7|`0x0B`|X.509 Certificate for Key Management|キー管理用の証明書|
|8|`0x0C`|Key History Object|（不要ですが管理対象とします）|

[注1]将来的に、[管理ツール](../../MaintenanceTool/README.md)によるメンテナンスが出来るようにすることを検討しています。<br>
[注2]インストールされたオブジェクトは、nRF52840のFlash ROMに格納されます。

#### ATR
CCIDデバイスのハードウェア特性に関するバイトデータです。[注1][注2]

|#|項目名|値|意味|
|:---:|:---|:---|:---|
|1|TS|`3B`|Direct Convention|
|2|T0|`F7`|Y(1): `b1111`, K: 7 (historical bytes)|
|3|TA1|`11`|Fi=372, Di=1, 372 cycles/ETU <br>(10752 bits/s at 4.00 MHz, <br>13440 bits/s for fMax=5 MHz)|
|4|TB1|`00`|VPP is not electrically connected|
|5|TC1|`00`|Extra guard time: 0|
|6|TD1|`81`|Y(i+1)=`b1000`, Protocol T=1|
|7|TA2|無し||
|8|TB2|無し||
|9|TC2|無し||
|10|TD2|`31`|Y(i+1)=`b0011`, Protocol T=1|
|11|TA3|`FE`|IFSC: 254|
|12|TB3|`65`|Block Waiting Integer: 6, <br>Character Waiting Integer: 5|
|13|TC3|無し||
|14|TD3|無し||
|15|Historical bytes|`43 61 6E 6F 6B 65 79`|15バイト以内の識別名|
|16|TCK|`99`|correct checksum|

[注1] ATRはレジストリー（[smartcard_list.txt](http://ludovic.rousseau.free.fr/softwares/pcsc-tools/smartcard_list.txt)）に未登録です。<br>
[注2] 現在は開発中なので、仮の値となっております。
