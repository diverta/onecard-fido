
# 参考：Yubico PIV Toolビルド手順

PIVカード管理ツールとして、ソースコードが公開されているYubico PIV Toolを、macOS環境上で利用できるよう、ビルド手順を整理しました。

Yubico PIV Tool (command line) のソースコードを参照することにより、PCのプログラムから、どのようにしてPIVデバイスに接続／データ転送を行なっているのかを参照することができると思われます。

## ビルド手順

#### 前提パッケージを導入

前提となるパッケージを、brewなどで事前にインストールしておきます。<br>
当方の環境では`check`なるツールが不足していたので、追加インストールしました。

```
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ brew install check
Updating Homebrew...
==> Auto-updated Homebrew!
Updated 1 tap (homebrew/core).
（中略）
==> Downloading https://github.com/libcheck/check/releases/download/0.14.0/check-0.14.0.tar.gz
==> Downloading from https://github-production-release-asset-2e65be.s3.amazonaws.com/48520045/5eb1d580-4022-11ea-9ec5-7d
######################################################################## 100.0%
==> ./configure --prefix=/usr/local/Cellar/check/0.14.0
==> make install
🍺  /usr/local/Cellar/check/0.14.0: 42 files, 528.5KB, built in 31 seconds
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ echo $?
0
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$
```

#### ソースコードを取得

下記サイトから yubico-piv-tool-2.0.0.tar.gz をダウンロード後、解凍します。
- <b>[yubico - Smart card drivers and tools](https://www.yubico.com/products/services-software/download/smart-card-drivers-tools/)</b><br>
Yubico PIV Tool (command line) というツールのリンク「Mac OS X download」をクリックしてダウンロードします。

解凍したフォルダー`yubico-piv-tool-2.0.0`を、`${HOME}/opt/`配下に移動します。

```
MacBookPro-makmorit-jp:~ makmorit$ ls -al opt
total 32
drwxr-xr-x   9 makmorit  staff    306  5 15 14:50 .
drwxr-xr-x+ 61 makmorit  staff   2074  5 15 14:32 ..
-rw-r--r--@  1 makmorit  staff  12292  5 15 14:50 .DS_Store
drwxr-xr-x   7 makmorit  staff    238 10  1  2018 gcc-arm-none-eabi-7-2018-q2-update
drwxr-xr-x   5 makmorit  staff    170  6 11  2019 nRF-Command-Line-Tools_9_8_1_OSX
drwxr-xr-x  14 makmorit  staff    476 11  7  2018 nRF5_SDK_15.2.0
drwxr-xr-x  14 makmorit  staff    476  4 29 10:35 nRF5_SDK_15.3.0
drwxr-xr-x   5 makmorit  staff    170 10  1  2018 nRF5x-Command-Line-Tools_9_7_3_OSX
drwxr-xr-x@ 25 makmorit  staff    850  5 15 14:55 yubico-piv-tool-2.0.0
MacBookPro-makmorit-jp:~ makmorit$
```

#### メイクファイルの生成

ソースコードフォルダー`yubico-piv-tool-2.0.0`に移動したら、シェル`configure`を実行し、メイクファイルを生成します。

```
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ PKG_CONFIG_PATH="/usr/local/opt/openssl@1.1/lib/pkgconfig" ./configure
checking for a BSD-compatible install... /usr/bin/install -c
checking whether build environment is sane... yes
checking for a thread-safe mkdir -p... build-aux/install-sh -c -d
（中略）
configure: summary of build options:

  Version:          2.0.0
  Host type:        x86_64-apple-darwin16.7.0
  Install prefix:   /usr/local
  Compiler:         gcc
  CFLAGS:           -g -O2
  CPPFLAGS:         
  Warnings:         
  Backend:          macscard
  OpenSSL version:  1.1.1
  PCSC
          CFLAGS:   
            LIBS:   
  Winscard
            LIBS:   
  Mac PCSC
            LIBS:   -Wl,-framework -Wl,PCSC
  Custom PCSC
            LIBS:   

  YKCS11 debug:    DISABLED
  Hardware tests:  DISABLED

MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$
```

#### ビルドの実行

`make`コマンドを実行し、ビルドを実行します。

```
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ make
Making all in lib
Making all in .
  CC       ykpiv.lo
  CC       util.lo
  CC       internal.lo
  CC       version.lo
  CC       error.lo
  CCLD     libykpiv.la
Making all in tests
make[2]: Nothing to be done for `all'.
Making all in tool
/Applications/Xcode.app/Contents/Developer/usr/bin/make  all-recursive
Making all in .
（中略）
Making all in tests
make[2]: Nothing to be done for `all'.
make[1]: Nothing to be done for `all-am'.
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ echo $?
0
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$
```

以上でビルドは完了になります。

[注] 本件調査では、ソースコードと実際の動作を参照するのが目的であるため、システムへのインストールは行わないものといたします。

## 動作確認

ユーティリティーツール`tool/yubico-piv-tool`の各種コマンドを実行し、動作確認を行います。

PIVデバイス参考実装の「`canokey-stm32`」を書き込んであるNUCLEOのRESETボタンを押してから５秒以内に、コマンドを実行します。

#### list-readers

PIVデバイスの一覧を画面表示します。<br>
以下のコマンドを実行します。

```
cd ${HOME}/opt/yubico-piv-tool-2.0.0/
tool/yubico-piv-tool -v --reader="Kingtrust Multi-Reader" -a list-readers
```

以下は実行例になります。

```
MacBookPro-makmorit-jp:~ makmorit$ cd ${HOME}/opt/yubico-piv-tool-2.0.0/
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ tool/yubico-piv-tool -v --reader="Kingtrust Multi-Reader" -a list-readers
Connect reader 'Kingtrust Multi-Reader' matching 'Kingtrust Multi-Reader'.
Action 'list-readers' does not need authentication.
Now processing for action 'list-readers'.
Kingtrust Multi-Reader
Disconnect card #1588739612.
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$
```

<details><summary>NUCLEOからUART経由で出力されたログはこちら</summary><p>
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
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040005A000000308
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] process_apdu(187): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00FD000000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
0500009000
[DBG] PC_to_RDR_XfrBlock(136): O:
00F8000000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
5EB23E1C9000
[DBG] CCID_Loop(281): Slot power on
```
</p></details>

#### change-pin

デフォルトのPIN`123456`を、`000000`に変更し、PIVデバイスに設定します。<br>
以下のコマンドを実行します。

```
tool/yubico-piv-tool -v --reader="Kingtrust Multi-Reader" --pin=123456 -a change-pin
```

以下は実行例になります。<br>
下記ログには表示されていないのですが、`Enter new pin:`、`Verifying - Enter new pin:`に続いて、`000000`を入力しています。

```
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ tool/yubico-piv-tool -v --reader="Kingtrust Multi-Reader" --pin=123456 -a change-pin
Connect reader 'Kingtrust Multi-Reader' matching 'Kingtrust Multi-Reader'.
Action 'change-pin' does not need authentication.
Now processing for action 'change-pin'.
Enter new pin:
Verifying - Enter new pin:
Successfully changed the pin code.
Disconnect card #1588739612.
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$
```

<details><summary>NUCLEOからUART経由で出力されたログはこちら</summary><p>
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
533B3019D4E739DA739CED39CE739D836858210842108421C84210C3EB34107F0563C809A4CF5E04D618AF5D67CBE8350832303330303130313E00FE009000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10C00
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10500
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10100
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10A00
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10B00
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040005A000000308
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] process_apdu(187): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00FD000000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
0500009000
[DBG] PC_to_RDR_XfrBlock(136): O:
00F8000000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
5EB23E1C9000
[DBG] PC_to_RDR_XfrBlock(136): O:
0024008010313233343536FFFF303030303030FFFF
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
9000
[DBG] CCID_Loop(281): Slot power on
```
</p></details>

#### set-chuid

CHUIDの生成を行い、PIVデバイスにインストールします。<br>
以下のコマンドを実行します。

```
tool/yubico-piv-tool -v --reader="Kingtrust Multi-Reader" -a set-chuid
```

以下は実行例になります。

```
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ tool/yubico-piv-tool -v --reader="Kingtrust Multi-Reader" -a set-chuid
Connect reader 'Kingtrust Multi-Reader' matching 'Kingtrust Multi-Reader'.
Authenticating since action 'set-chuid' needs that.
Successful application authentication.
Now processing for action 'set-chuid'.
Set the CHUID ID to: 7f 05 63 c8 09 a4 cf 5e 04 d6 18 af 5d 67 cb e8
Successfully set new CHUID.
Disconnect card #1588739612.
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$
```

<details><summary>NUCLEOからUART経由で出力されたログはこちら</summary><p>
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
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040005A000000308
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] process_apdu(187): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00FD000000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
0500009000
[DBG] PC_to_RDR_XfrBlock(136): O:
00F8000000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
5EB23E1C9000
[DBG] PC_to_RDR_XfrBlock(136): O:
0087039B047C028000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] piv_general_authenticate(305): Tag 80, pos: 4, len: 0
[DBG] PC_to_RDR_XfrBlock(155): I:
7C0A800896828E7FBC1A57DA9000
[DBG] PC_to_RDR_XfrBlock(136): O:
0087039B167C148008C6853860C48B4B3481080CCBB094E87BAA60
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] piv_general_authenticate(305): Tag 80, pos: 4, len: 8
[DBG] piv_general_authenticate(305): Tag 81, pos: 14, len: 8
[DBG] PC_to_RDR_XfrBlock(155): I:
7C0A8208232FE03414AA696C9000
[DBG] PC_to_RDR_XfrBlock(136): O:
00DB3FFF425C035FC102533B3019D4E739DA739CED39CE739D836858210842108421C84210C3EB34107F0563C809A4CF5E04D618AF5D67CBE8350832303330303130313E00FE00
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] piv_put_data(545): piv-chu length 61
[DBG] piv_put_data(552): length 61
[DBG] PC_to_RDR_XfrBlock(155): I:
9000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC102
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
533B3019D4E739DA739CED39CE739D836858210842108421C84210C3EB34107F0563C809A4CF5E04D618AF5D67CBE8350832303330303130313E00FE009000
[DBG] CCID_Loop(281): Slot power on
```
</p></details>

#### set-ccc

CCCの生成を行い、PIVデバイスにインストールします。<br>
以下のコマンドを実行します。

```
tool/yubico-piv-tool -v --reader="Kingtrust Multi-Reader" -a set-ccc
```

以下は実行例になります。

```
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ cd ${HOME}/opt/yubico-piv-tool-2.0.0/
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ tool/yubico-piv-tool -v --reader="Kingtrust Multi-Reader" -a set-ccc
Connect reader 'Kingtrust Multi-Reader' matching 'Kingtrust Multi-Reader'.
Authenticating since action 'set-ccc' needs that.
Successful application authentication.
Now processing for action 'set-ccc'.
Set the CCC ID to: e2 77 1c e9 57 47 6a 8c 9f 40 ef dd 2e ce
Successfully set new CCC.
Disconnect card #1588739612.
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$
```

<details><summary>NUCLEOからUART経由で出力されたログはこちら</summary><p>
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
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040005A000000308
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] process_apdu(187): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00FD000000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
0500009000
[DBG] PC_to_RDR_XfrBlock(136): O:
00F8000000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
5EB23E1C9000
[DBG] PC_to_RDR_XfrBlock(136): O:
0087039B047C028000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] piv_general_authenticate(305): Tag 80, pos: 4, len: 0
[DBG] PC_to_RDR_XfrBlock(155): I:
7C0A80088466F817050505209000
[DBG] PC_to_RDR_XfrBlock(136): O:
0087039B167C1480087C1E8DE4B84F658D8108157559816DA4C131
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] piv_general_authenticate(305): Tag 80, pos: 4, len: 8
[DBG] piv_general_authenticate(305): Tag 81, pos: 14, len: 8
[DBG] PC_to_RDR_XfrBlock(155): I:
7C0A8208419610572E3069A69000
[DBG] PC_to_RDR_XfrBlock(136): O:
00DB3FFF3A5C035FC1075333F015A000000116FF02E2771CE957476A8C9F40EFDD2ECEF10121F20121F300F40100F50110F600F700FA00FB00FC00FD00FE00
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] piv_put_data(545): piv-ccc length 53
[DBG] piv_put_data(552): length 53
[DBG] PC_to_RDR_XfrBlock(155): I:
9000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC107
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
5333F015A000000116FF02E2771CE957476A8C9F40EFDD2ECEF10121F20121F300F40100F50110F600F700FA00FB00FC00FD00FE009000
[DBG] CCID_Loop(281): Slot power on
```
</p></details>

#### status

CHUID、CCCの生成・インストールを行なった後のステータス確認になります。<br>
以下のコマンドを実行します。

```
tool/yubico-piv-tool -v --reader="Kingtrust Multi-Reader" -a status
```

以下は実行例になります。

```
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$ tool/yubico-piv-tool -v --reader="Kingtrust Multi-Reader" -a status
Connect reader 'Kingtrust Multi-Reader' matching 'Kingtrust Multi-Reader'.
Action 'status' does not need authentication.
Now processing for action 'status'.
Version:	5.0.0
Serial Number:	1588739612
CHUID:	3019d4e739da739ced39ce739d836858210842108421c84210c3eb34107f0563c809a4cf5e04d618af5d67cbe8350832303330303130313e00fe00
CCC:	f015a000000116ff02e2771ce957476a8c9f40efdd2ecef10121f20121f300f40100f50110f600f700fa00fb00fc00fd00fe00
PIN tries left:	3
Disconnect card #1588739612.
MacBookPro-makmorit-jp:yubico-piv-tool-2.0.0 makmorit$
```

<details><summary>NUCLEOからUART経由で出力されたログはこちら</summary><p>
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
533B3019D4E739DA739CED39CE739D836858210842108421C84210C3EB34107F0563C809A4CF5E04D618AF5D67CBE8350832303330303130313E00FE009000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10C00
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10500
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10100
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10A00
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10B00
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00A4040005A000000308
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] process_apdu(187): applet switched to: 1
[DBG] PC_to_RDR_XfrBlock(155): I:
61114F0600001000010079074F05A0000003089000
[DBG] PC_to_RDR_XfrBlock(136): O:
00FD000000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
0500009000
[DBG] PC_to_RDR_XfrBlock(136): O:
00F8000000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
5EB23E1C9000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC102
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
533B3019D4E739DA739CED39CE739D836858210842108421C84210C3EB34107F0563C809A4CF5E04D618AF5D67CBE8350832303330303130313E00FE009000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC107
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
5333F015A000000116FF02E2771CE957476A8C9F40EFDD2ECEF10121F20121F300F40100F50110F600F700FA00FB00FC00FD00FE009000
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC105
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10A
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10B
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC101
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10D
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10E
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC10F
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC110
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC111
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC112
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC113
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC114
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC115
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC116
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC117
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC118
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC119
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC11A
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC11B
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC11C
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC11D
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC11E
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC11F
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
00CB3FFF055C035FC120
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
6A82
[DBG] PC_to_RDR_XfrBlock(136): O:
0020008000
[DBG] CCID_TimeExtensionLoop(317): send t-ext
[DBG] PC_to_RDR_XfrBlock(155): I:
63C3
[DBG] CCID_Loop(281): Slot power on
```
</p></details>
