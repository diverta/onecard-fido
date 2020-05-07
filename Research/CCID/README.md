# CCIDインターフェースに関する調査

## 概要
セキュリティデバイス用のインターフェースとして使用されている、USB CCIDインターフェースに関する調査になります。

#### 調査の最終目的

FIDO認証器として試作している[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)を、市販されている「Yubikey NEO」同様、PIVデバイス（Personal Identity Verification Card）として使用できるようにする試みとなります。

## 使用環境

有志の方が公開している「[canokeys/canokey-stm32](https://github.com/canokeys/canokey-stm32)」というPIVデバイスの参考実装を使用します。<br>
STM32の評価基板である「NUCLEO-L432KC」で動作します。

接続テストは、macOS Sierraにインストールした「[OpenSC](https://github.com/OpenSC/OpenSC)」というミドルウェア（ドライバーとライブラリー、各種コマンドのセット）を使用して実施します。

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

## サンプルアプリを動かすまでの手順

「[canokeys/canokey-stm32](https://github.com/canokeys/canokey-stm32)」は、本プロジェクトと同様、Netbeansプロジェクトを作成してバイナリーを作成後、NUCLEOに書き込みます。

具体的な手順等の情報は、後日整理して掲載したいと思います。
