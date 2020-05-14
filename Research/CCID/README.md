# CCIDインターフェースに関する調査

## 概要
セキュリティデバイス用のインターフェースとして使用されている、USB CCIDインターフェースに関する調査になります。

#### 調査の最終目的

FIDO認証器として試作している[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)を、市販されている「Yubikey NEO」同様、PIVデバイス（Personal Identity Verification Card）として使用できるようにする試みとなります。

#### 使用環境

有志の方が公開している「[canokeys/canokey-stm32](https://github.com/canokeys/canokey-stm32)」というPIVデバイスの参考実装を使用します。<br>
STM32の評価基板である「NUCLEO-L432KC」で動作します。<br>
接続テストは、macOS Sierraにインストールした「[OpenSC](https://github.com/OpenSC/OpenSC)」というミドルウェア（ドライバーとライブラリー、各種コマンドのセット）を使用して実施します。

#### サンプルアプリを動かすまでの手順

「[canokeys/canokey-stm32](https://github.com/canokeys/canokey-stm32)」は、本プロジェクトと同様、Netbeansプロジェクトを作成してバイナリーを作成後、NUCLEOに書き込みます。<br>
具体的な手順等の情報は、手順書</b>「[canokey-stm32動作確認手順](../../Research/CCID/reference/README.md)」</b>をご参照願います。


## 2020/05/14までの調査内容・結果

canokey-stm32で初期データ投入を試行したのですが、失敗に終わっています。<br>
内容については別紙</b>「[参考：OpenSCコマンド実行時ログ](../../Research/CCID/OPENSCTOOLLOG.md)」</b>に掲載しております。

## 2020/05/11までの調査内容・結果

Yubikey NEOとcanokey-stm32間の動作相違の解析をした結果、以下の機密データが、canokey-stm32にセットアップされていないため、macOSで接続ができないと切り分けています。[注1]
- Card Holder Unique Identifier（CHUID）
- Card Capability Container（CCC）
- X.509 Certificate for Card Authentication
- X.509 Certificate for PIV Authentication
- X.509 Certificate for Digital Signature
- X.509 Certificate for Key Management

USB CCIDインターフェース経由で正しくPIVデバイスとして認識させるためには、上記機密データを、canokey-stm32にセットアップする必要があるかと思われます。

ただし、上記５点の詳細内容や、生成方法については、現時点（2020/05/11 16:12）では不明です。<br>
後日、追って調査したいと思います。

[注1] 他方、Linux系のOS（Ubuntuなど）では、前述の機密データがセットアップされていなくても、デバイススキャンおよび接続ができるようです。おそらくですが、macOSの方がセキュリティー性が高いため、接続できないものと推測しています。

#### macOS上で接続試行

OpenSCを導入済みのmacOSに、Yubikeyを装着した場合は、PIVデバイスとして認識／接続できるのですが、canokey-stm32を装着した場合は、認識すらされません。

確認した事象としては、canokey-stm32側が、macOSからのリクエスト`00CB3FFF055C035FC10200`に対し、下記のエラーを発生して`6A82`応答後、macOS側から切断されてしまう、というものです。

```
[DBG] main(320): Init FS
[DBG] littlefs_init(128): Flash base 0x8028000, 48 blocks (2048 bytes)
[DBG] main(323): Init applets
[DBG] main(331): Init USB
[DBG] main(336): Main Loop
（中略）
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10200
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] CCID_Loop(286): Slot power off
```

リクエスト`00CB3FFF055C035FC10200`というのは、ファイル種別`02`（Card Holder Unique Identifier）をストレージ（Flash ROM）から読出し、内容をレスポンスするコマンドのようです。<br>
下記は、ファームウェア内においてで、読出し要求コマンドに対するファイル種別の判定をしている部分になります。

```
static const char *get_object_path_by_tag(uint8_t tag) {
  switch (tag) {
  case 0x01: // X.509 Certificate for Card Authentication
    return CARD_AUTH_CERT_PATH;
  case 0x02: // Card Holder Unique Identifier
    return CHUID_PATH;
  case 0x05: // X.509 Certificate for PIV Authentication
    return PIV_AUTH_CERT_PATH;
  case 0x07: // Card Capability Container
    return CCC_PATH;
  case 0x0A: // X.509 Certificate for Digital Signature
    return SIG_CERT_PATH;
  case 0x0B: // X.509 Certificate for Key Management
    return KEY_MANAGEMENT_CERT_PATH;
  default:
    return NULL;
  }
}
```

現状、このファイルはストレージに存在しないため、`0x6A82 (SW_FILE_NOT_FOUND)`というエラーを応答しています。

#### KSJavaAPIによる解析

原因を深堀りするために、Javaの問題解析ツール（[KSJavaAPI.jar](https://github.com/grandamp/KSJavaAPI)）を導入して調査しました。<br>
Yubikey NEOに対する解析ツールの出力例は以下になります。

```
- KeySupport PIV API Read Test -

Provider: SunPCSC - Sun PC/SC provider
Available Card Readers:

1: Yubico Yubikey NEO OTP+U2F+CCID

Enter a number of the reader which contains the PIV credential,
and then press [Enter]:
1
CommmandAPDU: 17 bytes, nc=11, ne=256: 00A404000BA00000030800001000010000
ResponseAPDU: 21 bytes, SW=9000: 61114F0600001000010079074F05A000000308
Application Property:
PIV App Property:AID:		000010000100
PIV App Property:Tag Alloc:	4F05A000000308

Command APDU: 00CB3FFF035C017E00
PIV Discovery Object:AID:		
PIV Discovery Object:PIN Policy:
PIV Discovery Object:Global PIN:	The Application PIN is primary.

Card: org.keysupport.nist80073.PIVCard@7b23ec81
Card ATR: 3BFC1300008131FE15597562696B65794E454F7233E1
Command APDU: 00CB3FFF055C035FC10200
CommmandAPDU: 11 bytes, nc=5, ne=256: 00CB3FFF055C035FC10200
Card Holder Unique ID:FASC-N:Agency Code:			9999
Card Holder Unique ID:FASC-N:System Code:			9999
（中略）

Command APDU: 00CB3FFF055C035FC10500
CommmandAPDU: 11 bytes, nc=5, ne=256: 00CB3FFF055C035FC10500
Object not found: Tag: 5FC105
```

ツールで解析した結果、Yubikey NEOに設定されているCHUID、CCCは、以下になるようです。<br>
（４点の X.509 Certificate は、おそらくセキュリティーの観点から、ツールで取得することはできませんでした）

```
chuid:
3019D4E739DA739CED39CE739D836858210842108421384210C3F53410BE5C1D0D26286F30B67DEF481F5EF208350832303330303130313E00FE00
ccc:
F015A000000116FF026F36262391082D7A3AFBA45E9920F10121F20121F300F40100F50110F600F700FA00FB00FC00FD00FE00
```

#### ご参考

Linux系のOSでは、前述の機密データがcanokey-stm32にセットアップされていなくても、`pcsc_scan`により接続ができるようです。<br>
下記はUbuntu OS上でスキャンを実行した時のログですが、USB CCIDインターフェース経由でPIVデバイスとして認識されているようでした。

```
makmorit@Ubuntu-makmorit-jp:~$ pcsc_scan
PC/SC device scanner
V 1.4.25 (c) 2001-2011, Ludovic Rousseau <ludovic.rousseau@free.fr>
Compiled with PC/SC lite version: 1.8.14
Using reader plug'n play mechanism
Scanning present readers...
0: Kingtrust Multi-Reader [OpenPGP PIV OATH] (5EB23E1C) 00 00

Mon May 11 14:02:07 2020
Reader 0: Kingtrust Multi-Reader [OpenPGP PIV OATH] (5EB23E1C) 00 00
  Card state: Card inserted,
  ATR: 3B F7 11 00 00 81 31 FE 65 43 61 6E 6F 6B 65 79 99

ATR: 3B F7 11 00 00 81 31 FE 65 43 61 6E 6F 6B 65 79 99
+ TS = 3B --> Direct Convention
+ T0 = F7, Y(1): 1111, K: 7 (historical bytes)
  TA(1) = 11 --> Fi=372, Di=1, 372 cycles/ETU
    10752 bits/s at 4 MHz, fMax for Fi = 5 MHz => 13440 bits/s
  TB(1) = 00 --> VPP is not electrically connected
  TC(1) = 00 --> Extra guard time: 0
  TD(1) = 81 --> Y(i+1) = 1000, Protocol T = 1
-----
  TD(2) = 31 --> Y(i+1) = 0011, Protocol T = 1
-----
  TA(3) = FE --> IFSC: 254
  TB(3) = 65 --> Block Waiting Integer: 6 - Character Waiting Integer: 5
+ Historical bytes: 43 61 6E 6F 6B 65 79
  Category indicator byte: 43 (proprietary format)
+ TCK = 99 (correct checksum)

Possibly identified card (using /home/makmorit/.cache/smartcard_list.txt):
3B F7 11 00 00 81 31 FE 65 43 61 6E 6F 6B 65 79 99
	Canokey (Other)
	http://canokeys.org/
```


## 2020/05/07までの調査内容・結果

OpenSCを使用し、USB CCIDインターフェースを経由して、PIVサービスに接続し、各種情報を取得できるか調査しました。

結果としては、実装が足りないためか、CCID接続はできるのですが、PIVデバイスとしては正しく動作しないようでした。

主に、macOS上で必要となる、署名（CHUID、CCC）等のデバイス固有データが、サンプルでセットアップされていない様子です。<br>
サンプルのコードを精査しましたが、具体的にどのデバイス固有データをどのようにSTM32に導入するかなどの情報は、残念ながら得られませんでした。

おそらくですが、当方で追加実装が必要な部分ではないか、と考えております。

#### 実行時のログ

OpenSCのユーティリティーコマンドを使用して、接続-->デバイス固有データを取得しましたが、デバイスでセットアップされておらず、結果として取得に失敗したことを確認しています。

```
MacBookPro-makmorit-jp:~ makmorit$ opensc-tool -a -f -v
Using reader with a card: Kingtrust Multi-Reader
Card ATR:
3B F7 11 00 00 81 31 FE 65 43 61 6E 6F 6B 65 79 ;.....1.eCanokey
99                                              .
Connecting to card in reader Kingtrust Multi-Reader...
Using card driver Personal Identity Verification Card.
SELECT FILE failed: File not found
MacBookPro-makmorit-jp:~ makmorit$
```

STM32側からのデバッグログは以下のように出力されました。<br>
`6A82`というAPDUを戻している部分が、データがない旨のレスポンスになります。

`read_file [piv-chu] len==0`、`read_file [piv-ccc] len==0`というログ出力から、CHUID、CCCといったデバイス固有データがセットアップされていない状態と切り分けています。

ただしこのログでは、具体的にどのファイルがどの状態になっているか、の情報が足りていないので、後日、詳細デバッグを出力させるようにしたいと思います。

```
[DBG] main(320): Init FS
[DBG] littlefs_init(128): Flash base 0x8028000, 48 blocks (2048 bytes)
[DBG] main(323): Init applets
[DBG] main(331): Init USB
[DBG] main(336): Main Loop
[DBG] main(349): Touch calibrating...
[DBG] GPIO_Touch_Calibrate(72): touch_threshold 0
[DBG] USBD_CANOKEY_Setup(39): Recipient: 1, Index: 0
[DBG] CCID_Loop(281): Slot power on
[DBG] PC_to_RDR_XfrBlock(136): O:
00A404000BA000000308000010000100
[DBG] process_apdu(178): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00A404000BA000000308000010000100
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] process_apdu(178): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10200
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] piv_get_data(247): read_file [piv-chu] len==0
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040007627601FF000000
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] process_apdu(185): applet not found
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040006A00000000101
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] process_apdu(185): applet not found
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00A404000BE82B0601040181C31F020100
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] process_apdu(185): applet not found
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040C0FD23300000045737445494420763335
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A86
[DBG] PC_to_RDR_XfrBlock(136): O:
00CADF3005
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6D00
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF035C017E08
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
7E124F0BA0000003610C
[DBG] PC_to_RDR_XfrBlock(136): O:
00C000000C
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
080000100001005F2F0240106A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040009A0000003080000100000
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] process_apdu(178): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10708
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] piv_get_data(247): read_file [piv-ccc] len==0
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF035C017E00
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
7E124F0BA0000003080000100001005F2F0240106A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF035C017E08
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
7E124F0BA0000003610C
[DBG] PC_to_RDR_XfrBlock(136): O:
00C000000C
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
080000100001005F2F0240106A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040009A0000003080000100000
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] process_apdu(178): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10708
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] piv_get_data(247): read_file [piv-ccc] len==0
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF035C017E00
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
7E124F0BA0000003080000100001005F2F0240106A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10C08
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] piv_get_data(238): get_object_path_by_tag null
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040009A0000003080000100000
[DBG] CCID_TimeExtensionLoop(318): send t-ext
[DBG] process_apdu(178): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] CCID_Loop(286): Slot power off
```
