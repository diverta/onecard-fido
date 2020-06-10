# USB CCIDインターフェース

## 概要
USB CCIDインターフェースをnRF52840上に実装するためのモジュールです。

プラットフォーム（nRF5 SDK）に依存する部分と、依存しない部分に分かれています。

## 前提要件

#### CCIDドライバーのカスタマイズ

macOSでは、CCIDドライバーのサポートデバイス一覧[注1]に、ハードウェアの製品ID（`VID=0xf055`、`PID=0x0001`）が含まれていないと、CCIDインターフェースによる接続ができません。

このため、本件CCIDインターフェースの動作確認を行うにあたり、<b>カスタマイズしたCCIDドライバーを、事前にmacOSに導入</b>しています。<br>
CCIDドライバーのカスタマイズおよび導入手順につきましては、別ドキュメント<b>「[CCIDドライバー修正ビルド手順](../../Research/CCID/BUILDCCIDDRV.md)」</b>をご参照願います。

[注1]macOSにプレインストールされているCCIDドライバーは`/usr/libexec/SmartCardServices/drivers/ifd-ccid.bundle`です。このフパッケージは、`Info.plist`というファイルに、サポートデバイス一覧を内包しています。<br>
[注2]カスタマイズしたCCIDドライバーは、`/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle`に導入します。これにより、別段支障なくプレインストール版ドライバーと共存できるようです。

## 構成

#### モジュール一覧（プラットフォーム非依存）

|#|関数名|説明|
|:---:|:---|:---|
|1-1|ccid.h|CCID関連で共通利用するヘッダー|
|1-2|ccid_main.c/.h|エントリーモジュール|
|1-3|ccid_apdu.c/.h|リクエストAPDU解析、レスポンスAPDU生成を行います。|
|1-4|ccid_piv.c/.h|PIVに関する業務処理を実行します。|
|1-5|ccid_piv_general_auth.c/.h|PIVに必要な各種認証処理を実行します。|
|1-5|ccid_piv_object.c/.h|PIVで使用する各種オブジェクトを管理します。|

#### モジュール一覧（プラットフォーム依存）

|#|関数名|説明|
|:---:|:---|:---|
|2-1|app_usbd_ccid_internal.h|内部利用するヘッダー[注1]|
|2-2|app_usbd_ccid.c/.h|nRF52840用CCID I/F[注1]|
|2-3|usbd_service.c/.h|USB経由で通信する各種I/F(CCID、HID、CDC)への<br>振分処理を実行します。|
|2-4|usbd_service_ccid.c/.h|CCID I/F経由でデータ送受信処理を実行します。|

[注1]これらのモジュールは、後日`usbd_service_ccid.c/.h`に吸収して一本化する予定です。

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
（2020/06/10現在、[nRF52840アプリケーション](../../nRF5_SDK_v15.3.0)に実装されているもの）

|#|INS|名称|説明|
|:---:|:---|:---|:---|
|1|`0xA4`|PIV_INS_SELECT|PIV Appletを実行可能化|
|2|`0xCB`|PIV_INS_GET_DATA|PIVオブジェクトを取得|
|3|`0xFD`|PIV_INS_GET_VERSION|バージョン取得[注1]|
|4|`0xF8`|PIV_INS_GET_SERIAL|Serial number取得|
|5|`0x87`|PIV_INS_GENERAL_AUTHENTICATE|PIV認証を実行[注2]|
|6|`0xDB`|PIV_INS_PUT_DATA|PIVオブジェクトを転送[注3]|

[注1]バージョンは[管理ツール](../../MaintenanceTool/README.md)で参照できるものと同一値になります。<br>
[注2]デフォルト管理用キーを使用した認証機能のみが実装されています。<br>
[注3]転送されたオブジェクトを永続化する機能は未実装です。

#### 管理対象PIVオブジェクト
本モジュールで管理対象とするPIVオブジェクト（PIVで使用する各種機密データ）は、下記一覧の通りとなっております。<br>
[Yubico PIV Tool](https://developers.yubico.com/yubico-piv-tool/)や、[YubiKey Manager](https://www.yubico.com/products/services-software/download/yubikey-manager/)といった無償ツールを使用して、インストールすることを可能とする予定です。[注1][注2]

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
[注2]インストールされたオブジェクトは、nRF52840のFlash ROMに格納される予定です。

### CCIDの仕様
CCIDインターフェースに関する仕様は下記の通りです。

#### ATRバイト配列
CCIDデバイスのハードウェア特性に関するバイトデータです。[注1]

|#|項目名|値|意味|
|:---:|:---|:---|:---|
|1|TS|`3B`|Direct Convention|
|2|T0|`FD`|Y(1): `b1111`, K: 13 (Historical bytes)|
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
|15|Historical bytes|`53 65 63 75 72 65 20 44`<br>`6f 6e 67 6c 65`|13バイトの識別名[注2]|
|16|TCK|`FB`|checksum|

[注1]ATRはレジストリー（[smartcard_list.txt](http://ludovic.rousseau.free.fr/softwares/pcsc-tools/smartcard_list.txt)）に未登録です。<br>
[注2]`Secure Dongle`という文字列になります。

#### APDUバイト配列
各種業務処理（PIV／OpenPGP／OATH等）では、データを「APDUバイト配列」[注1]に格納し、リクエスト／レスポンスが行われます。<br>
本インターフェースがサポートするAPDUバイト配列は、以下の様式になっております。

<b>コマンドAPDU</b><br>
PCからリクエストされたコマンドとデータを格納します。

|#|項目名|内容|
|:---:|:---|:---|
|1|CLA|コマンド形式を識別するID|
|2|INS|コマンドを識別するID|
|3|P1|コマンドのパラメーター1|
|4|P2|コマンドのパラメーター2|
|5|Lc|データ長[注2]|
|6|データ|リクエストデータを格納[注2]|
|7|Le|レスポンス長[注3]|

<b>レスポンスAPDU</b><br>
コマンドに対するレスポンスデータ／ステータスを格納します。

|#|項目名|内容|
|:---:|:---|:---|
|8|データ|レスポンスデータを格納[注4]|
|9|SW1|レスポンスのステータスワード1|
|10|SW2|レスポンスのステータスワード2|

[注1]APDU＝Application Protocol Data Unit<br>
[注2]データを格納しない場合は、省略されます。<br>
[注3]省略されることがあります。<br>
[注4]省略されることがあります。

#### CCIDセッションの内容

CCIDインターフェースの接続／切断処理において受け渡しされるデータは以下のようになります。

| # |項目 |リクエスト | レスポンス |
|:-:|:-|:-|:-|
|0|スロットステータス取得|`65 00 00 00 00 00 [SEQ] 00`<br>`00 00`|`81 00 00 00 00 00 [SEQ] 00`<br>`81 00`|
|1|スロット接続|`62 00 00 00 00 00 [SEQ] 00`<br>`00 00`|`80 17 00 00 00 00 [SEQ] 00`<br>`81 00 [ATRバイト配列]`<br>[注1]|
|2|各種業務処理<br>(PIV／OpenPGP／OATH等)|`6F [APDU長] 00 00 00 00 [SEQ] 00`<br>`00 00 [コマンドAPDU]`[注2]|`80 [APDU長] 00 00 00 00 [SEQ] 00`<br>`81 00 [レスポンスAPDU]`[注2]|
|3|スロット切断|`63 00 00 00 00 00 [SEQ] 00`<br>`00 00`|`81 00 00 00 00 00 [SEQ] 00`<br>`81 00`|

[注1]ATRバイト配列は前述の内容になります。<br>
[注2]コマンドAPDU／レスポンスAPDU（APDUバイト配列）は前述の内容になります。
