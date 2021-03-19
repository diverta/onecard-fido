# USB CCIDインターフェース

最終更新日：2020/11/26

## 概要
USB CCIDインターフェースをnRF52840上に実装するためのモジュールです。

プラットフォーム（nRF5 SDK）に依存する部分と、依存しない部分に分かれています。

## 前提要件

#### CCIDドライバーのカスタマイズ

macOSでは、CCIDドライバーのサポートデバイス一覧[注1]に、ハードウェアの製品ID（`VID=0xf055`、`PID=0x0001`）が含まれていないと、CCIDインターフェースによる接続ができません。

このため、本件CCIDインターフェースの動作確認を行うにあたり、<b>カスタマイズしたCCIDドライバーを、事前にmacOSに導入</b>しています。<br>
CCIDドライバーのカスタマイズおよび導入手順につきましては、別ドキュメント<b>「[CCIDドライバー修正ビルド手順](../../Research/CCID/BUILDCCIDDRV.md)」</b>をご参照願います。

[注1]macOSにプレインストールされているCCIDドライバーは`/usr/libexec/SmartCardServices/drivers/ifd-ccid.bundle`です。このパッケージは、`Info.plist`というファイルに、サポートデバイス一覧を内包しています。<br>
[注2]カスタマイズしたCCIDドライバーは、`/usr/local/libexec/SmartCardServices/drivers/ifd-ccid.bundle`に導入します。これにより、別段支障なくプレインストール版ドライバーと共存できるようです。

## 仕様
本モジュール群で実行できるセキュリティー機能は以下になります。<br>
開発中・開発予定のものも含みます。

|#|名称|説明|
|:---:|:---|:---|
|1|PIV<br>(Personal Identification Verification)|PIV Cardをエミュレートする機能。[注1]<br>使用例：<br>・macOSでの、PIN番号を使用したログイン認証<br>・ECDSA鍵を使用したSSHの実行<br>・etc...|
|2|OpenPGP|OpenPGP Cardをエミュレートする機能。[注1]<br>GnuPG等のPGPアプリケーションを使用し、電子署名や<br>暗号／復号化といったセキュア処理ができます。|
|3|OATH|（サポート機能は検討中）|

[注1]nRF52840をUSBに装着すると、NFCカードリーダーにスマートカード（PIV CardやOpenPGP Card）を置いた状態と等価になります。

### [PIVの仕様](../../CCID/ccid_lib/README_PIV.md)
PIVカードエミュレーション機能に関する仕様について掲載しています。

### [OpenPGPの仕様](../../CCID/openpgp_lib/README.md)
OpenPGPカードエミュレーション機能に関する仕様について掲載しています。

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
|4|Time extension|無し|`80 00 00 00 00 00 [SEQ] 80`<br>`01 00`[注3]|

[注1]ATRバイト配列は前述の内容になります。<br>
[注2]コマンドAPDU／レスポンスAPDU（APDUバイト配列）は前述の内容になります。<br>
[注3]macOSでは無通信状態が３秒連続すると、タイムアウトとなってしまうため、Time extensionレスポンスをキープアライブ目的で送信します。本プロジェクトでは、PIV機能のRSA-2048認証が３秒を超える処理となるため、Time extensionレスポンスを送信するよう実装しております。
