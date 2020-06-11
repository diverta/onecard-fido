# 参考：OpenSCコマンド実行時ログ

OpenSCに付属のコマンドツールを、NUCLEO（[canokeys/canokey-stm32](https://github.com/canokeys/canokey-stm32)）に対して実行し、挙動を確認した時のログになります。

ログは2020/05/14までに実行したものをまとめました。

## opensc-tool実行時ログ

macOSに接続されているPIVカードリーダーを検出したりするツールのログです。

#### 挙動から推測されること

- NUCLEOをRESETした後、約５秒以内は、macOSからカードがあると判断される
- 約５秒程度経過すると、カードが無くなったと判定される


#### デバイスが切断されてしまった後のケース

NUCLEOをRESETし、１０秒ほど後に`opensc-tool`を実行した時のログです。<br>
ログを見る限り、認識されていますが、カードが無いと判断されます。

```
MacBookPro-makmorit-jp:~ makmorit$ opensc-tool --list-readers
# Detected readers (pcsc)
Nr.  Card  Features  Name
0    No              Kingtrust Multi-Reader
MacBookPro-makmorit-jp:~ makmorit$
```

コマンドが出力したログ

```
P:595; T:0x140736680367040 13:35:08.399 [opensc-tool] ctx.c:851:sc_context_create: ===================================
P:595; T:0x140736680367040 13:35:08.400 [opensc-tool] ctx.c:852:sc_context_create: opensc version: 0.20.0
P:595; T:0x140736680367040 13:35:08.400 [opensc-tool] reader-pcsc.c:865:pcsc_init: PC/SC options: connect_exclusive=0 disconnect_action=0 transaction_end_action=0 reconnect_action=0 enable_pinpad=1 enable_pace=1
P:595; T:0x140736680367040 13:35:08.401 [opensc-tool] reader-pcsc.c:1347:pcsc_detect_readers: called
P:595; T:0x140736680367040 13:35:08.401 [opensc-tool] reader-pcsc.c:1360:pcsc_detect_readers: Probing PC/SC readers
P:595; T:0x140736680367040 13:35:08.401 [opensc-tool] reader-pcsc.c:1411:pcsc_detect_readers: Establish PC/SC context
P:595; T:0x140736680367040 13:35:08.415 [opensc-tool] reader-pcsc.c:1296:pcsc_add_reader: Adding new PC/SC reader 'Kingtrust Multi-Reader'
P:595; T:0x140736680367040 13:35:08.415 [opensc-tool] reader-pcsc.c:333:refresh_attributes: Kingtrust Multi-Reader check
P:595; T:0x140736680367040 13:35:08.417 [opensc-tool] reader-pcsc.c:381:refresh_attributes: current  state: 0x00000012
P:595; T:0x140736680367040 13:35:08.417 [opensc-tool] reader-pcsc.c:382:refresh_attributes: previous state: 0x00000000
P:595; T:0x140736680367040 13:35:08.417 [opensc-tool] reader-pcsc.c:437:refresh_attributes: card absent
P:595; T:0x140736680367040 13:35:08.417 [opensc-tool] reader-pcsc.c:1491:pcsc_detect_readers: Kingtrust Multi-Reader:SCardConnect(DIRECT): 0x00000000
P:595; T:0x140736680367040 13:35:08.417 [opensc-tool] reader-pcsc.c:1114:detect_reader_features: called
P:595; T:0x140736680367040 13:35:08.417 [opensc-tool] reader-pcsc.c:1116:detect_reader_features: Requesting reader features ...
P:595; T:0x140736680367040 13:35:10.167 [opensc-tool] reader-pcsc.c:1137:detect_reader_features: Reader feature 12 found
P:595; T:0x140736680367040 13:35:10.167 [opensc-tool] reader-pcsc.c:1054:part10_detect_max_data: get dwMaxAPDUDataSize property returned 65536
P:595; T:0x140736680367040 13:35:10.168 [opensc-tool] reader-pcsc.c:1247:detect_reader_features: Reader supports transceiving 65536 bytes of data
P:595; T:0x140736680367040 13:35:10.168 [opensc-tool] reader-pcsc.c:1093:part10_get_vendor_product: id_vendor=0483 id_product=0007
P:595; T:0x140736680367040 13:35:10.169 [opensc-tool] reader-pcsc.c:1515:pcsc_detect_readers: returning with: 0 (Success)
P:595; T:0x140736680367040 13:35:10.170 [opensc-tool] sc.c:315:sc_detect_card_presence: called
P:595; T:0x140736680367040 13:35:10.170 [opensc-tool] reader-pcsc.c:445:pcsc_detect_card_presence: called
P:595; T:0x140736680367040 13:35:10.170 [opensc-tool] reader-pcsc.c:333:refresh_attributes: Kingtrust Multi-Reader check
P:595; T:0x140736680367040 13:35:10.171 [opensc-tool] reader-pcsc.c:358:refresh_attributes: returning with: 0 (Success)
P:595; T:0x140736680367040 13:35:10.171 [opensc-tool] reader-pcsc.c:450:pcsc_detect_card_presence: returning with: 0 (Success)
P:595; T:0x140736680367040 13:35:10.172 [opensc-tool] sc.c:320:sc_detect_card_presence: returning with: 0 (Success)
P:595; T:0x140736680367040 13:35:10.172 [opensc-tool] ctx.c:927:sc_release_context: called
P:595; T:0x140736680367040 13:35:10.172 [opensc-tool] reader-pcsc.c:946:pcsc_finish: called
```


#### デバイスが切断される前のケース

NUCLEOをRESETした直後に`opensc-tool`を実行した時のログです。<br>
ログを見る限り、カードがあると認識されているようです。

もちろんこの数秒後に、切断されてしまいます。

```
MacBookPro-makmorit-jp:~ makmorit$ opensc-tool --list-readers
# Detected readers (pcsc)
Nr.  Card  Features  Name
0    Yes             Kingtrust Multi-Reader
MacBookPro-makmorit-jp:~ makmorit$
```

コマンドが出力したログ

```
P:605; T:0x140736680367040 13:38:29.806 [opensc-tool] ctx.c:851:sc_context_create: ===================================
P:605; T:0x140736680367040 13:38:29.807 [opensc-tool] ctx.c:852:sc_context_create: opensc version: 0.20.0
P:605; T:0x140736680367040 13:38:29.807 [opensc-tool] reader-pcsc.c:865:pcsc_init: PC/SC options: connect_exclusive=0 disconnect_action=0 transaction_end_action=0 reconnect_action=0 enable_pinpad=1 enable_pace=1
P:605; T:0x140736680367040 13:38:29.808 [opensc-tool] reader-pcsc.c:1347:pcsc_detect_readers: called
P:605; T:0x140736680367040 13:38:29.808 [opensc-tool] reader-pcsc.c:1360:pcsc_detect_readers: Probing PC/SC readers
P:605; T:0x140736680367040 13:38:29.808 [opensc-tool] reader-pcsc.c:1411:pcsc_detect_readers: Establish PC/SC context
P:605; T:0x140736680367040 13:38:29.822 [opensc-tool] reader-pcsc.c:1296:pcsc_add_reader: Adding new PC/SC reader 'Kingtrust Multi-Reader'
P:605; T:0x140736680367040 13:38:29.822 [opensc-tool] reader-pcsc.c:333:refresh_attributes: Kingtrust Multi-Reader check
P:605; T:0x140736680367040 13:38:29.823 [opensc-tool] reader-pcsc.c:381:refresh_attributes: current  state: 0x00000022
P:605; T:0x140736680367040 13:38:29.824 [opensc-tool] reader-pcsc.c:382:refresh_attributes: previous state: 0x00000000
P:605; T:0x140736680367040 13:38:29.824 [opensc-tool] reader-pcsc.c:437:refresh_attributes: card present, changed
P:605; T:0x140736680367040 13:38:29.825 [opensc-tool] reader-pcsc.c:1500:pcsc_detect_readers: Kingtrust Multi-Reader:SCardConnect(SHARED): 0x00000000
P:605; T:0x140736680367040 13:38:29.825 [opensc-tool] reader-pcsc.c:1114:detect_reader_features: called
P:605; T:0x140736680367040 13:38:29.825 [opensc-tool] reader-pcsc.c:1116:detect_reader_features: Requesting reader features ...
P:605; T:0x140736680367040 13:38:29.826 [opensc-tool] reader-pcsc.c:1137:detect_reader_features: Reader feature 12 found
P:605; T:0x140736680367040 13:38:29.826 [opensc-tool] reader-pcsc.c:1054:part10_detect_max_data: get dwMaxAPDUDataSize property returned 65536
P:605; T:0x140736680367040 13:38:29.826 [opensc-tool] reader-pcsc.c:1247:detect_reader_features: Reader supports transceiving 65536 bytes of data
P:605; T:0x140736680367040 13:38:29.826 [opensc-tool] reader-pcsc.c:1093:part10_get_vendor_product: id_vendor=0483 id_product=0007
P:605; T:0x140736680367040 13:38:29.827 [opensc-tool] reader-pcsc.c:1515:pcsc_detect_readers: returning with: 0 (Success)
P:605; T:0x140736680367040 13:38:29.827 [opensc-tool] sc.c:315:sc_detect_card_presence: called
P:605; T:0x140736680367040 13:38:29.827 [opensc-tool] reader-pcsc.c:445:pcsc_detect_card_presence: called
P:605; T:0x140736680367040 13:38:29.827 [opensc-tool] reader-pcsc.c:333:refresh_attributes: Kingtrust Multi-Reader check
P:605; T:0x140736680367040 13:38:29.828 [opensc-tool] reader-pcsc.c:358:refresh_attributes: returning with: 0 (Success)
P:605; T:0x140736680367040 13:38:29.828 [opensc-tool] reader-pcsc.c:450:pcsc_detect_card_presence: returning with: 1
P:605; T:0x140736680367040 13:38:29.828 [opensc-tool] sc.c:320:sc_detect_card_presence: returning with: 1
P:605; T:0x140736680367040 13:38:29.828 [opensc-tool] ctx.c:927:sc_release_context: called
P:605; T:0x140736680367040 13:38:29.828 [opensc-tool] reader-pcsc.c:946:pcsc_finish: called
```

#### NUCLEOの出力ログ

`6A82`の出力から、最終行`[DBG] CCID_Loop(286): Slot power off`の出力まで約５秒でした。

```
[DBG] main(320): Init FS
[DBG] littlefs_init(128): Flash base 0x8028000, 48 blocks (2048 bytes)
[DBG] main(323): Init applets
[DBG] main(331): Init USB
[DBG] main(336): Main Loop
[DBG] main(349): Touch calibrating...
[DBG] GPIO_Touch_Calibrate(82): touch_threshold 0
[DBG] USBD_CANOKEY_Setup(39): Recipient: 1, Index: 0
[DBG] CCID_Loop(281): Slot power on
[DBG] PC_to_RDR_XfrBlock(136): O:
00A404000BA000000308000010000100
[DBG] process_apdu(187): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00A404000BA000000308000010000100
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] process_apdu(187): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10200
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] CCID_Loop(286): Slot power off
```


## piv-tool実行時ログ

PIVカードリーダーに鍵や証明書、機密データをプロビジョニングしたりするツールのログです。

#### コマンド実行ログ

piv-toolは、macOSにカードリーダーが接続中と判定されているときに実行します。

試しに、CCCをインストールするコマンドを実行しましたが、エラーとなってしまいます。

```
MacBookPro-makmorit-jp:~ makmorit$ piv-tool --object DB00 --in /Users/makmorit/Documents/FIDO/LOGS/202005/0511/ccc.dms
Using reader with a card: Kingtrust Multi-Reader
object tag or length not valid
MacBookPro-makmorit-jp:~ makmorit$
```

以下はコマンドが出力したログ<br>
長いですが全量を掲載します。

```
P:729; T:0x140736680367040 15:19:52.100 [piv-tool] ctx.c:851:sc_context_create: ===================================
P:729; T:0x140736680367040 15:19:52.100 [piv-tool] ctx.c:852:sc_context_create: opensc version: 0.20.0
P:729; T:0x140736680367040 15:19:52.100 [piv-tool] reader-pcsc.c:865:pcsc_init: PC/SC options: connect_exclusive=0 disconnect_action=0 transaction_end_action=0 reconnect_action=0 enable_pinpad=1 enable_pace=1
P:729; T:0x140736680367040 15:19:52.101 [piv-tool] reader-pcsc.c:1347:pcsc_detect_readers: called
P:729; T:0x140736680367040 15:19:52.101 [piv-tool] reader-pcsc.c:1360:pcsc_detect_readers: Probing PC/SC readers
P:729; T:0x140736680367040 15:19:52.101 [piv-tool] reader-pcsc.c:1411:pcsc_detect_readers: Establish PC/SC context
P:729; T:0x140736680367040 15:19:52.114 [piv-tool] reader-pcsc.c:1296:pcsc_add_reader: Adding new PC/SC reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.114 [piv-tool] reader-pcsc.c:333:refresh_attributes: Kingtrust Multi-Reader check
P:729; T:0x140736680367040 15:19:52.116 [piv-tool] reader-pcsc.c:381:refresh_attributes: current  state: 0x00000022
P:729; T:0x140736680367040 15:19:52.116 [piv-tool] reader-pcsc.c:382:refresh_attributes: previous state: 0x00000000
P:729; T:0x140736680367040 15:19:52.116 [piv-tool] reader-pcsc.c:437:refresh_attributes: card present, changed
P:729; T:0x140736680367040 15:19:52.117 [piv-tool] reader-pcsc.c:1500:pcsc_detect_readers: Kingtrust Multi-Reader:SCardConnect(SHARED): 0x00000000
P:729; T:0x140736680367040 15:19:52.117 [piv-tool] reader-pcsc.c:1114:detect_reader_features: called
P:729; T:0x140736680367040 15:19:52.117 [piv-tool] reader-pcsc.c:1116:detect_reader_features: Requesting reader features ...
P:729; T:0x140736680367040 15:19:52.118 [piv-tool] reader-pcsc.c:1137:detect_reader_features: Reader feature 12 found
P:729; T:0x140736680367040 15:19:52.118 [piv-tool] reader-pcsc.c:1054:part10_detect_max_data: get dwMaxAPDUDataSize property returned 65536
P:729; T:0x140736680367040 15:19:52.118 [piv-tool] reader-pcsc.c:1247:detect_reader_features: Reader supports transceiving 65536 bytes of data
P:729; T:0x140736680367040 15:19:52.118 [piv-tool] reader-pcsc.c:1093:part10_get_vendor_product: id_vendor=0483 id_product=0007
P:729; T:0x140736680367040 15:19:52.119 [piv-tool] reader-pcsc.c:1515:pcsc_detect_readers: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.119 [piv-tool] sc.c:315:sc_detect_card_presence: called
P:729; T:0x140736680367040 15:19:52.119 [piv-tool] reader-pcsc.c:445:pcsc_detect_card_presence: called
P:729; T:0x140736680367040 15:19:52.119 [piv-tool] reader-pcsc.c:333:refresh_attributes: Kingtrust Multi-Reader check
P:729; T:0x140736680367040 15:19:52.120 [piv-tool] reader-pcsc.c:358:refresh_attributes: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.120 [piv-tool] reader-pcsc.c:450:pcsc_detect_card_presence: returning with: 1
P:729; T:0x140736680367040 15:19:52.120 [piv-tool] sc.c:320:sc_detect_card_presence: returning with: 1
P:729; T:0x140736680367040 15:19:52.120 [piv-tool] sc.c:315:sc_detect_card_presence: called
P:729; T:0x140736680367040 15:19:52.120 [piv-tool] reader-pcsc.c:445:pcsc_detect_card_presence: called
P:729; T:0x140736680367040 15:19:52.120 [piv-tool] reader-pcsc.c:333:refresh_attributes: Kingtrust Multi-Reader check
P:729; T:0x140736680367040 15:19:52.120 [piv-tool] reader-pcsc.c:358:refresh_attributes: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.120 [piv-tool] reader-pcsc.c:450:pcsc_detect_card_presence: returning with: 1
P:729; T:0x140736680367040 15:19:52.120 [piv-tool] sc.c:320:sc_detect_card_presence: returning with: 1
P:729; T:0x140736680367040 15:19:52.120 [piv-tool] card.c:254:sc_connect_card: called
P:729; T:0x140736680367040 15:19:52.120 [piv-tool] reader-pcsc.c:578:pcsc_connect: called
P:729; T:0x140736680367040 15:19:52.120 [piv-tool] reader-pcsc.c:333:refresh_attributes: Kingtrust Multi-Reader check
P:729; T:0x140736680367040 15:19:52.121 [piv-tool] reader-pcsc.c:358:refresh_attributes: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] reader-pcsc.c:610:pcsc_connect: Initial protocol: T=1
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:279:sc_connect_card: matching configured ATRs
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:323:sc_connect_card: matching built-in ATRs
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'cardos'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'flex'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'cyberflex'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'gpk'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'gemsafeV1'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'asepcos'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'starcos'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'tcos'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'oberthur'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'authentic'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-authentic.c:416:authentic_match_card:
try to match card with ATR (17 bytes):
3B F7 11 00 00 81 31 FE 65 43 61 6E 6F 6B 65 79 ;.....1.eCanokey
99                                              .
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-authentic.c:419:authentic_match_card: card not matched
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'iasecc'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-iasecc.c:345:iasecc_match_card: card not matched
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'belpic'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'incrypto34'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'akis'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'entersafe'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-entersafe.c:138:entersafe_match_card: called
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'epass2003'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-epass2003.c:1143:epass2003_match_card: called
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'rutoken'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-rutoken.c:103:rutoken_match_card: called
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-rutoken.c:109:rutoken_match_card: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'rutoken_ecp'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-rtecp.c:77:rtecp_match_card: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'myeid'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'dnie'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-dnie.c:738:dnie_match_card: called
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-dnie.c:741:dnie_match_card: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'MaskTech'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'atrust-acos'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'westcos'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'esteid2018'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card.c:341:sc_connect_card: trying driver 'coolkey'
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-coolkey.c:2240:coolkey_match_card: called
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-coolkey.c:919:coolkey_apdu_io: called
P:729; T:0x140736680367040 15:19:52.122 [piv-tool] card-coolkey.c:924:coolkey_apdu_io: a4 04 00 7 : 0 0
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] card-coolkey.c:988:coolkey_apdu_io: calling sc_transmit_apdu flags=0 le=0, resplen=0, resp=0x7fff5c44efa0
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] reader-pcsc.c:657:pcsc_lock: called
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] card-coolkey.c:2385:coolkey_card_reader_lock_obtained: called
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] card-coolkey.c:2391:coolkey_card_reader_lock_obtained: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:A4, P1:4, P2:0, data(7) 0x7fff5c450079
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (12 bytes):
00 A4 04 00 07 62 76 01 FF 00 00 00 .....bv.....
P:729; T:0x140736680367040 15:19:52.123 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (2 bytes):
6A 82 j.
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] reader-pcsc.c:709:pcsc_unlock: called
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] card-coolkey.c:995:coolkey_apdu_io: result r=0 apdu.resplen=0 sw1=6a sw2=82
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] card-coolkey.c:875:coolkey_check_sw: sw1 = 0x6a, sw2 = 0x82
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] iso7816.c:128:iso7816_check_sw: File or application not found
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] card-coolkey.c:1003:coolkey_apdu_io: Transmit failed
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] card-coolkey.c:1021:coolkey_apdu_io: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] card.c:341:sc_connect_card: trying driver 'muscle'
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] reader-pcsc.c:657:pcsc_lock: called
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] card-muscle.c:832:muscle_card_reader_lock_obtained: called
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] card-muscle.c:840:muscle_card_reader_lock_obtained: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.142 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.143 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.143 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:A4, P1:4, P2:0, data(6) 0x103d91b70
P:729; T:0x140736680367040 15:19:52.143 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.143 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (11 bytes):
00 A4 04 00 06 A0 00 00 00 01 01 ...........
P:729; T:0x140736680367040 15:19:52.143 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.161 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (2 bytes):
6A 82 j.
P:729; T:0x140736680367040 15:19:52.161 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.161 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.161 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.161 [piv-tool] reader-pcsc.c:709:pcsc_unlock: called
P:729; T:0x140736680367040 15:19:52.161 [piv-tool] muscle.c:276:msc_select_applet: returning with: -1200 (Card command failed)
P:729; T:0x140736680367040 15:19:52.161 [piv-tool] card.c:341:sc_connect_card: trying driver 'sc-hsm'
P:729; T:0x140736680367040 15:19:52.161 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.161 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.161 [piv-tool] reader-pcsc.c:657:pcsc_lock: called
P:729; T:0x140736680367040 15:19:52.162 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.162 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.162 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.162 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:A4, P1:4, P2:0, data(11) 0x7fff5c44fd20
P:729; T:0x140736680367040 15:19:52.162 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.162 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (17 bytes):
00 A4 04 00 0B E8 2B 06 01 04 01 81 C3 1F 02 01 ......+.........
00                                              .
P:729; T:0x140736680367040 15:19:52.162 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (2 bytes):
6A 82 j.
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] reader-pcsc.c:709:pcsc_unlock: called
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] iso7816.c:128:iso7816_check_sw: File or application not found
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] iso7816.c:576:iso7816_select_file: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] card-sc-hsm.c:180:sc_hsm_select_file_ex: Could not select SmartCard-HSM application: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] card-sc-hsm.c:258:sc_hsm_match_card: Could not select SmartCard-HSM application: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] card.c:341:sc_connect_card: trying driver 'mcrd'
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] card-mcrd.c:231:mcrd_match_card: called
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] reader-pcsc.c:657:pcsc_lock: called
P:729; T:0x140736680367040 15:19:52.181 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.182 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.182 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.182 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:A4, P1:4, P2:C, data(15) 0x103d2fd78
P:729; T:0x140736680367040 15:19:52.182 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.182 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (20 bytes):
00 A4 04 0C 0F D2 33 00 00 00 45 73 74 45 49 44 ......3...EstEID
20 76 33 35                                      v35
P:729; T:0x140736680367040 15:19:52.182 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (2 bytes):
6A 86 j.
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] reader-pcsc.c:709:pcsc_unlock: called
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] iso7816.c:128:iso7816_check_sw: Incorrect parameters P1-P2
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] card.c:341:sc_connect_card: trying driver 'setcos'
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] reader-pcsc.c:657:pcsc_lock: called
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:CA, P1:DF, P2:30, data(0) 0x0
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (5 bytes):
00 CA DF 30 05 ...0.
P:729; T:0x140736680367040 15:19:52.198 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.211 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (2 bytes):
6D 00 m.
P:729; T:0x140736680367040 15:19:52.211 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.211 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.211 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.211 [piv-tool] reader-pcsc.c:709:pcsc_unlock: called
P:729; T:0x140736680367040 15:19:52.211 [piv-tool] card.c:341:sc_connect_card: trying driver 'PIV-II'
P:729; T:0x140736680367040 15:19:52.211 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] reader-pcsc.c:657:pcsc_lock: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card-piv.c:3759:piv_card_reader_lock_obtained: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card-piv.c:3764:piv_card_reader_lock_obtained: PIV_STATE_MATCH
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card-piv.c:3800:piv_card_reader_lock_obtained: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card-piv.c:2754:piv_find_discovery: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card-piv.c:990:piv_get_cached_data: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card-piv.c:991:piv_get_cached_data: #10
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card-piv.c:1031:piv_get_cached_data: get #10
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card-piv.c:910:piv_get_data: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card-piv.c:911:piv_get_data: #10
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card-piv.c:936:piv_get_data: get len of #10
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card-piv.c:533:piv_general_io: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:CB, P1:3F, P2:FF, data(3) 0x7fff5c44fce8
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (9 bytes):
00 CB 3F FF 03 5C 01 7E 08 ..?..\.~.
P:729; T:0x140736680367040 15:19:52.212 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.227 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (10 bytes):
7E 12 4F 0B A0 00 00 03 61 0C ~.O.....a.
P:729; T:0x140736680367040 15:19:52.227 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.228 [piv-tool] apdu.c:426:sc_get_response: called
P:729; T:0x140736680367040 15:19:52.228 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.228 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.228 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.228 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.228 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.228 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:C0, P1:0, P2:0, data(0) 0x0
P:729; T:0x140736680367040 15:19:52.228 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.228 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (5 bytes):
00 C0 00 00 0C .....
P:729; T:0x140736680367040 15:19:52.228 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (14 bytes):
08 00 00 10 00 01 00 5F 2F 02 40 10 90 00 ......._/.@...
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] apdu.c:497:sc_get_response: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] card-piv.c:596:piv_general_io: returning with: 8
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] card-piv.c:961:piv_get_data: buffer for #10 *buf=0x0x0 len=20
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] card-piv.c:533:piv_general_io: called
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:CB, P1:3F, P2:FF, data(3) 0x7fff5c44fce8
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (9 bytes):
00 CB 3F FF 03 5C 01 7E 14 ..?..\.~.
P:729; T:0x140736680367040 15:19:52.243 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (22 bytes):
7E 12 4F 0B A0 00 00 03 08 00 00 10 00 01 00 5F ~.O............_
2F 02 40 10 90 00                               /.@...
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:596:piv_general_io: returning with: 20
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:977:piv_get_data: returning with: 20
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:1047:piv_get_cached_data: added #10  0x7ff6a2e05740:20 0x0:0
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:1062:piv_get_cached_data: returning with: 20
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:2614:piv_parse_discovery: Discovery 0x60 0x1e 0x7ff6a2e05742:18
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:2627:piv_parse_discovery: Discovery pinp flags=0x40 0x10
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:2639:piv_parse_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:2659:piv_process_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:2773:piv_find_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:2733:piv_process_ccc: called
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:990:piv_get_cached_data: called
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:991:piv_get_cached_data: #0
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:1031:piv_get_cached_data: get #0
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:910:piv_get_data: called
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card-piv.c:911:piv_get_data: #0
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.261 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] card-piv.c:936:piv_get_data: get len of #0
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] card-piv.c:533:piv_general_io: called
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:CB, P1:3F, P2:FF, data(5) 0x7fff5c44fe78
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (11 bytes):
00 CB 3F FF 05 5C 03 5F C1 07 08 ..?..\._...
P:729; T:0x140736680367040 15:19:52.262 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.279 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (2 bytes):
6A 82 j.
P:729; T:0x140736680367040 15:19:52.279 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.279 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.279 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.279 [piv-tool] iso7816.c:128:iso7816_check_sw: File or application not found
P:729; T:0x140736680367040 15:19:52.279 [piv-tool] card-piv.c:577:piv_general_io: Card returned error
P:729; T:0x140736680367040 15:19:52.279 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.279 [piv-tool] card-piv.c:596:piv_general_io: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.279 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.279 [piv-tool] card-piv.c:977:piv_get_data: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card-piv.c:1062:piv_get_cached_data: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card-piv.c:2742:piv_process_ccc: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card-piv.c:2754:piv_find_discovery: called
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card-piv.c:910:piv_get_data: called
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card-piv.c:911:piv_get_data: #10
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card-piv.c:961:piv_get_data: buffer for #10 *buf=0x0x7fff5c44fec0 len=256
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card-piv.c:533:piv_general_io: called
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:CB, P1:3F, P2:FF, data(3) 0x7fff5c44fe28
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (9 bytes):
00 CB 3F FF 03 5C 01 7E 00 ..?..\.~.
P:729; T:0x140736680367040 15:19:52.280 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (22 bytes):
7E 12 4F 0B A0 00 00 03 08 00 00 10 00 01 00 5F ~.O............_
2F 02 40 10 90 00                               /.@...
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] card-piv.c:596:piv_general_io: returning with: 20
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] card-piv.c:977:piv_get_data: returning with: 20
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] card-piv.c:2614:piv_parse_discovery: Discovery 0x60 0x1e 0x7fff5c44fec2:18
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] card-piv.c:2639:piv_parse_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] card-piv.c:2773:piv_find_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] reader-pcsc.c:709:pcsc_unlock: called
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] card-piv.c:3011:piv_finish: called
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] card.c:355:sc_connect_card: matched: Personal Identity Verification Card
P:729; T:0x140736680367040 15:19:52.298 [piv-tool] card-piv.c:3307:piv_init: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] reader-pcsc.c:657:pcsc_lock: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card-piv.c:3759:piv_card_reader_lock_obtained: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card-piv.c:3764:piv_card_reader_lock_obtained: PIV_STATE_MATCH
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card-piv.c:3800:piv_card_reader_lock_obtained: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card-piv.c:2754:piv_find_discovery: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card-piv.c:990:piv_get_cached_data: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card-piv.c:991:piv_get_cached_data: #10
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card-piv.c:1031:piv_get_cached_data: get #10
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card-piv.c:910:piv_get_data: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card-piv.c:911:piv_get_data: #10
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card-piv.c:936:piv_get_data: get len of #10
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card-piv.c:533:piv_general_io: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:CB, P1:3F, P2:FF, data(3) 0x7fff5c44fbb8
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (9 bytes):
00 CB 3F FF 03 5C 01 7E 08 ..?..\.~.
P:729; T:0x140736680367040 15:19:52.299 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.314 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (10 bytes):
7E 12 4F 0B A0 00 00 03 61 0C ~.O.....a.
P:729; T:0x140736680367040 15:19:52.314 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.314 [piv-tool] apdu.c:426:sc_get_response: called
P:729; T:0x140736680367040 15:19:52.314 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.314 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.314 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.314 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.314 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.314 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:C0, P1:0, P2:0, data(0) 0x0
P:729; T:0x140736680367040 15:19:52.315 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.315 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (5 bytes):
00 C0 00 00 0C .....
P:729; T:0x140736680367040 15:19:52.315 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (14 bytes):
08 00 00 10 00 01 00 5F 2F 02 40 10 90 00 ......._/.@...
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] apdu.c:497:sc_get_response: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] card-piv.c:596:piv_general_io: returning with: 8
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] card-piv.c:961:piv_get_data: buffer for #10 *buf=0x0x0 len=20
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] card-piv.c:533:piv_general_io: called
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:CB, P1:3F, P2:FF, data(3) 0x7fff5c44fbb8
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (9 bytes):
00 CB 3F FF 03 5C 01 7E 14 ..?..\.~.
P:729; T:0x140736680367040 15:19:52.330 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (22 bytes):
7E 12 4F 0B A0 00 00 03 08 00 00 10 00 01 00 5F ~.O............_
2F 02 40 10 90 00                               /.@...
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:596:piv_general_io: returning with: 20
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:977:piv_get_data: returning with: 20
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:1047:piv_get_cached_data: added #10  0x7ff6a4200000:20 0x0:0
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:1062:piv_get_cached_data: returning with: 20
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:2614:piv_parse_discovery: Discovery 0x60 0x1e 0x7ff6a4200002:18
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:2627:piv_parse_discovery: Discovery pinp flags=0x40 0x10
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:2639:piv_parse_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:2659:piv_process_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:2773:piv_find_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:2733:piv_process_ccc: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:990:piv_get_cached_data: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:991:piv_get_cached_data: #0
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:1031:piv_get_cached_data: get #0
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:910:piv_get_data: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:911:piv_get_data: #0
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:936:piv_get_data: get len of #0
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card-piv.c:533:piv_general_io: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:CB, P1:3F, P2:FF, data(5) 0x7fff5c44fd48
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (11 bytes):
00 CB 3F FF 05 5C 03 5F C1 07 08 ..?..\._...
P:729; T:0x140736680367040 15:19:52.349 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (2 bytes):
6A 82 j.
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] iso7816.c:128:iso7816_check_sw: File or application not found
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card-piv.c:577:piv_general_io: Card returned error
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card-piv.c:596:piv_general_io: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card-piv.c:977:piv_get_data: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card-piv.c:1062:piv_get_cached_data: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card-piv.c:2742:piv_process_ccc: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card-piv.c:2754:piv_find_discovery: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card-piv.c:910:piv_get_data: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card-piv.c:911:piv_get_data: #10
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card-piv.c:961:piv_get_data: buffer for #10 *buf=0x0x7fff5c44fd90 len=256
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card-piv.c:533:piv_general_io: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:CB, P1:3F, P2:FF, data(3) 0x7fff5c44fcf8
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.367 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (9 bytes):
00 CB 3F FF 03 5C 01 7E 00 ..?..\.~.
P:729; T:0x140736680367040 15:19:52.368 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (22 bytes):
7E 12 4F 0B A0 00 00 03 08 00 00 10 00 01 00 5F ~.O............_
2F 02 40 10 90 00                               /.@...
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] card-piv.c:596:piv_general_io: returning with: 20
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] card-piv.c:977:piv_get_data: returning with: 20
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] card-piv.c:2614:piv_parse_discovery: Discovery 0x60 0x1e 0x7fff5c44fd92:18
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] card-piv.c:2639:piv_parse_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] card-piv.c:2773:piv_find_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] card-piv.c:3326:piv_init: Max send = 0 recv = 0 card->type = 14001
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] card-piv.c:3429:piv_init: PIV card-type=14001 card_issues=0x00000102
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] card-piv.c:2815:piv_process_history: called
P:729; T:0x140736680367040 15:19:52.385 [piv-tool] card-piv.c:990:piv_get_cached_data: called
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] card-piv.c:991:piv_get_cached_data: #11
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] card-piv.c:1031:piv_get_cached_data: get #11
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] card-piv.c:910:piv_get_data: called
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] card-piv.c:911:piv_get_data: #11
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] card-piv.c:936:piv_get_data: get len of #11
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] card-piv.c:533:piv_general_io: called
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:CB, P1:3F, P2:FF, data(5) 0x7fff5c44f8d8
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (11 bytes):
00 CB 3F FF 05 5C 03 5F C1 0C 08 ..?..\._...
P:729; T:0x140736680367040 15:19:52.386 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (2 bytes):
6A 82 j.
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] iso7816.c:128:iso7816_check_sw: File or application not found
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:577:piv_general_io: Card returned error
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:596:piv_general_io: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:977:piv_get_data: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:1062:piv_get_cached_data: returning with: -1201 (File not found)
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:3001:piv_process_history: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:990:piv_get_cached_data: called
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:991:piv_get_cached_data: #10
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:1004:piv_get_cached_data: found #10 0x7ff6a4200000:20 0x0:0
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:1062:piv_get_cached_data: returning with: 20
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:2614:piv_parse_discovery: Discovery 0x60 0x1e 0x7ff6a4200002:18
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:2627:piv_parse_discovery: Discovery pinp flags=0x40 0x10
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:2639:piv_parse_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card-piv.c:2659:piv_process_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.400 [piv-tool] reader-pcsc.c:709:pcsc_unlock: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card-piv.c:3475:piv_init: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card.c:385:sc_connect_card: card info name:'Personal Identity Verification Card', type:14001, flags:0x0, max_send/recv_size:255/256
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card.c:1521:sc_card_sm_check: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card.c:1526:sc_card_sm_check: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card.c:397:sc_connect_card: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] reader-pcsc.c:657:pcsc_lock: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card-piv.c:3759:piv_card_reader_lock_obtained: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card-piv.c:2754:piv_find_discovery: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card-piv.c:910:piv_get_data: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card-piv.c:911:piv_get_data: #10
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card-piv.c:961:piv_get_data: buffer for #10 *buf=0x0x7fff5c450590 len=256
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card-piv.c:533:piv_general_io: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] apdu.c:546:sc_transmit_apdu: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card.c:473:sc_lock: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] apdu.c:513:sc_transmit: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] apdu.c:363:sc_single_transmit: called
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] apdu.c:370:sc_single_transmit: CLA:0, INS:CB, P1:3F, P2:FF, data(3) 0x7fff5c4504f8
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] reader-pcsc.c:297:pcsc_transmit: reader 'Kingtrust Multi-Reader'
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] reader-pcsc.c:298:pcsc_transmit:
Outgoing APDU (9 bytes):
00 CB 3F FF 03 5C 01 7E 00 ..?..\.~.
P:729; T:0x140736680367040 15:19:52.401 [piv-tool] reader-pcsc.c:216:pcsc_internal_transmit: called
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] reader-pcsc.c:307:pcsc_transmit:
Incoming APDU (22 bytes):
7E 12 4F 0B A0 00 00 03 08 00 00 10 00 01 00 5F ~.O............_
2F 02 40 10 90 00                               /.@...
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] apdu.c:382:sc_single_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] apdu.c:535:sc_transmit: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] card-piv.c:596:piv_general_io: returning with: 20
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] card-piv.c:977:piv_get_data: returning with: 20
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] card-piv.c:2614:piv_parse_discovery: Discovery 0x60 0x1e 0x7fff5c450592:18
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] card-piv.c:2639:piv_parse_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] card-piv.c:2773:piv_find_discovery: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] card-piv.c:3800:piv_card_reader_lock_obtained: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] card.c:513:sc_lock: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] card.c:523:sc_unlock: called
P:729; T:0x140736680367040 15:19:52.419 [piv-tool] reader-pcsc.c:709:pcsc_unlock: called
P:729; T:0x140736680367040 15:19:52.420 [piv-tool] card.c:414:sc_disconnect_card: called
P:729; T:0x140736680367040 15:19:52.420 [piv-tool] card-piv.c:3011:piv_finish: called
P:729; T:0x140736680367040 15:19:52.420 [piv-tool] reader-pcsc.c:642:pcsc_disconnect: Kingtrust Multi-Reader:SCardDisconnect returned: 0x00000000
P:729; T:0x140736680367040 15:19:52.420 [piv-tool] card.c:436:sc_disconnect_card: returning with: 0 (Success)
P:729; T:0x140736680367040 15:19:52.420 [piv-tool] ctx.c:927:sc_release_context: called
P:729; T:0x140736680367040 15:19:52.420 [piv-tool] reader-pcsc.c:946:pcsc_finish: called

```

#### NUCLEOの出力ログ

こちらもやや長いですが、全量を掲載します。


```
[DBG] main(320): Init FS
[DBG] littlefs_init(128): Flash base 0x8028000, 48 blocks (2048 bytes)
[DBG] main(323): Init applets
[DBG] main(331): Init USB
[DBG] main(336): Main Loop
[DBG] main(349): Touch calibrating...
[DBG] GPIO_Touch_Calibrate(82): touch_threshold 0
[DBG] USBD_CANOKEY_Setup(39): Recipient: 1, Index: 0
[DBG] CCID_Loop(281): Slot power on
[DBG] PC_to_RDR_XfrBlock(136): O:
00A404000BA000000308000010000100
[DBG] process_apdu(187): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00A404000BA000000308000010000100
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] process_apdu(187): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10200
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:  <-- ここからが piv-tool 実行時のログ
00A4040007627601FF000000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] process_apdu(194): applet not found
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040006A00000000101
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] process_apdu(194): applet not found
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00A404000BE82B0601040181C31F020100
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] process_apdu(194): applet not found
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040C0FD23300000045737445494420763335
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A86
[DBG] PC_to_RDR_XfrBlock(136): O:
00CADF3005
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6D00
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF035C017E08
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
7E124F0BA0000003610C
[DBG] PC_to_RDR_XfrBlock(136): O:
00C000000C
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
080000100001005F2F0240109000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF035C017E14
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
7E124F0BA0000003080000100001005F2F0240109000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10708
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF035C017E00
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
7E124F0BA0000003080000100001005F2F0240109000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF035C017E08
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
7E124F0BA0000003610C
[DBG] PC_to_RDR_XfrBlock(136): O:
00C000000C
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
080000100001005F2F0240109000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF035C017E14
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
7E124F0BA0000003080000100001005F2F0240109000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10708
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF035C017E00
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
7E124F0BA0000003080000100001005F2F0240109000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10C08
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF035C017E00
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
7E124F0BA0000003080000100001005F2F0240109000
[DBG] CCID_Loop(286): Slot power off
```
