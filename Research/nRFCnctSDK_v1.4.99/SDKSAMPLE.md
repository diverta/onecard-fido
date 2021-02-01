# nRF Connect SDKサンプル動作確認手順書

Nordic社が用意しているnRF Connect SDKサンプルアプリ「[Bluetooth: Peripheral UART](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/samples/bluetooth/peripheral_uart/README.html)」の動作確認手順について掲載します。

#### [nRF52840 DKを使用したサンプル動作確認手順書はこちら](../../Research/nRFCnctSDK_v1.4.99/SDKSAMPLE52.md)

## 事前準備

#### J-Linkのインストール

PCとNordic開発ボード（nRF5340 DK）をUSBケーブル経由で接続するためのソフトウェア「SEGGER J-Link」を、PCに導入願います。<br>
インストールの詳細につきましては、別途手順書「[NetBeansインストール手順](../../nRF52840_app/NETBEANSINST.md)」の該当章「<b>SEGGER J-Link</b>」をご参照願います。

#### ボードをPCに接続

動作確認に使用する開発ボード（nRF5340 DK）を、USBケーブルでPCに接続します。<br>
Finderで、JLINKという名前のボリュームができていることを確認してください。

<img src="assets01/0000.jpg" width="400">

## サンプルアプリのビルド

サンプルアプリ「Bluetooth: Peripheral UART」をビルドし、ファームウェアイメージファイルを作成します。

#### ビルド準備

必要な環境変数をシェルに準備した後、サンプルアプリディレクトリーに移動します。<br>
以下のコマンドを実行します。

```
. ${HOME}/.zephyrrc
cd ${HOME}/opt/ncs/nrf/samples/bluetooth/peripheral_uart
```

以下は実行例になります。

```
bash-3.2$ . ${HOME}/.zephyrrc
bash-3.2$ cd ${HOME}/opt/ncs/nrf/samples/bluetooth/peripheral_uart
bash-3.2$ ls -al
total 72
drwxr-xr-x  10 makmorit  staff   320  1  5 10:39 .
drwxr-xr-x  27 makmorit  staff   864  1  4 15:13 ..
-rw-r--r--@  1 makmorit  staff  6148  1  5 10:39 .DS_Store
-rw-r--r--   1 makmorit  staff   340  1  4 13:45 CMakeLists.txt
-rw-r--r--   1 makmorit  staff   737  1  4 13:45 Kconfig
-rw-r--r--   1 makmorit  staff  4650  1  4 13:45 README.rst
-rw-r--r--   1 makmorit  staff   968  1  4 13:45 prj.conf
-rw-r--r--   1 makmorit  staff    46  1  4 13:45 prj.overlay
-rw-r--r--   1 makmorit  staff   392  1  4 13:45 sample.yaml
drwxr-xr-x   3 makmorit  staff    96  1  4 13:45 src
bash-3.2$
```

#### ビルドの実行

westツールを使用して、サンプルアプリをビルドします。<br>
以下のコマンドを実行します。<br>
（`build`サブディレクトリーがある場合は、事前に削除します）

```
rm -rf build
west build -b nrf5340dk_nrf5340_cpuapp
```

以下は実行例になります。<br>
ファームウェアイメージファイルは、サンプルアプリディレクトリー配下の`build/zephyr`というサブディレクトリーに作成されるようです。

```
bash-3.2$ pwd
/Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart
bash-3.2$
bash-3.2$ rm -rf build
bash-3.2$ west build -b nrf5340dk_nrf5340_cpuapp
-- west build: generating a build system
Including boilerplate (Zephyr base): /Users/makmorit/opt/ncs/zephyr/cmake/app/boilerplate.cmake
-- Application: /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart
-- Zephyr version: 2.4.99 (/Users/makmorit/opt/ncs/zephyr)
-- Found Python3: /usr/local/bin/python3.9 (found suitable exact version "3.9.1") found components: Interpreter
-- Found west (found suitable version "0.8.0", minimum required is "0.7.1")
-- Board: nrf5340dk_nrf5340_cpuapp
-- Cache files will be written to: /Users/makmorit/Library/Caches/zephyr
-- Found toolchain: gnuarmemb (/Users/makmorit/opt/gcc-arm-none-eabi-9-2020-q2-update)
-- Found BOARD.dts: /Users/makmorit/opt/ncs/zephyr/boards/arm/nrf5340dk_nrf5340/nrf5340dk_nrf5340_cpuapp.dts
-- Generated zephyr.dts: /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/zephyr/zephyr.dts
-- Generated devicetree_unfixed.h: /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/zephyr/include/generated/devicetree_unfixed.h
Parsing /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/Kconfig
Loaded configuration '/Users/makmorit/opt/ncs/zephyr/boards/arm/nrf5340dk_nrf5340/nrf5340dk_nrf5340_cpuapp_defconfig'
Merged configuration '/Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/prj.conf'
Configuration saved to '/Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/zephyr/.config'
Kconfig header saved to '/Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/zephyr/include/generated/autoconf.h'
-- The C compiler identification is GNU 9.3.1
-- The CXX compiler identification is GNU 9.3.1
-- The ASM compiler identification is GNU
-- Found assembler: /Users/makmorit/opt/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-gcc
Adding 'hci_rpmsg' sample as child image since CONFIG_BT_RPMSG_NRF53 is set to 'y'

=== child image hci_rpmsg - CPUNET begin ===
Including boilerplate (Zephyr base): /Users/makmorit/opt/ncs/zephyr/cmake/app/boilerplate.cmake
-- Application: /Users/makmorit/opt/ncs/zephyr/samples/bluetooth/hci_rpmsg
-- Zephyr version: 2.4.99 (/Users/makmorit/opt/ncs/zephyr)
-- Found Python3: /usr/local/bin/python3.9 (found suitable exact version "3.9.1") found components: Interpreter
-- Found west (found suitable version "0.8.0", minimum required is "0.7.1")
-- Board: nrf5340dk_nrf5340_cpunet
-- Cache files will be written to: /Users/makmorit/Library/Caches/zephyr
-- Found toolchain: gnuarmemb (/Users/makmorit/opt/gcc-arm-none-eabi-9-2020-q2-update)
-- Found BOARD.dts: /Users/makmorit/opt/ncs/zephyr/boards/arm/nrf5340dk_nrf5340/nrf5340dk_nrf5340_cpunet.dts
-- Generated zephyr.dts: /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/hci_rpmsg/zephyr/zephyr.dts
-- Generated devicetree_unfixed.h: /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/hci_rpmsg/zephyr/include/generated/devicetree_unfixed.h
Parsing /Users/makmorit/opt/ncs/zephyr/Kconfig
Loaded configuration '/Users/makmorit/opt/ncs/zephyr/boards/arm/nrf5340dk_nrf5340/nrf5340dk_nrf5340_cpunet_defconfig'
Merged configuration '/Users/makmorit/opt/ncs/zephyr/samples/bluetooth/hci_rpmsg/prj.conf'
Configuration saved to '/Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/hci_rpmsg/zephyr/.config'
Kconfig header saved to '/Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/hci_rpmsg/zephyr/include/generated/autoconf.h'
-- The C compiler identification is GNU 9.3.1
-- The CXX compiler identification is GNU 9.3.1
-- The ASM compiler identification is GNU
-- Found assembler: /Users/makmorit/opt/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-gcc
-- Build type:  
-- Host:    Darwin/x86_64
-- Target:  Generic/arm
-- Machine: cortexm
-- Looking for include file stdatomic.h
-- Looking for include file stdatomic.h - found
-- Host:    Darwin/x86_64
-- Target:  Generic/arm
-- Machine: cortexm
-- C_FLAGS :  -Wall -Wextra
-- Looking for include file fcntl.h
-- Looking for include file fcntl.h - found
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/hci_rpmsg
=== child image hci_rpmsg - CPUNET end ===

-- Build type:  
-- Host:    Darwin/x86_64
-- Target:  Generic/arm
-- Machine: cortexm
-- Looking for include file stdatomic.h
-- Looking for include file stdatomic.h - found
-- Host:    Darwin/x86_64
-- Target:  Generic/arm
-- Machine: cortexm
-- C_FLAGS :  -Wall -Wextra
-- Looking for include file fcntl.h
-- Looking for include file fcntl.h - found
CMake Warning at /Users/makmorit/opt/ncs/zephyr/CMakeLists.txt:1349 (message):
  __ASSERT() statements are globally ENABLED


-- Configuring done
-- Generating done
-- Build files have been written to: /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build
-- west build: building application
[1/237] Preparing syscall dependency handling

[7/237] Performing build step for 'hci_rpmsg_subimage'
[1/205] Preparing syscall dependency handling

[198/205] Linking C executable zephyr/zephyr_prebuilt.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:      163396 B       256 KB     62.33%
            SRAM:       44092 B        64 KB     67.28%
        IDT_LIST:         168 B         2 KB      8.20%
[205/205] Generating zephyr/merged_CPUNET.hex
[227/237] Linking C executable zephyr/zephyr_prebuilt.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:      125604 B      1016 KB     12.07%
            SRAM:       27420 B       448 KB      5.98%
        IDT_LIST:          88 B         2 KB      4.30%
[237/237] Generating zephyr/merged_domains.hex
bash-3.2$
```

## サンプルアプリの書込み

ビルドしたサンプルアプリのファームウェアイメージファイル`zephyr.hex`を、nRF5340 DKに書込みます。

以下のコマンドを実行します。
```
west -v flash
```

以下は実行例になります。

```
bash-3.2$ west -v flash
ZEPHYR_BASE=/Users/makmorit/opt/ncs/zephyr (origin: configfile)
-- west flash: rebuilding
cmake version 3.18.0 is OK; minimum version is 3.13.1
Running CMake: /Applications/CMake.app/Contents/bin/cmake --build /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build --target west_flash_depends
[0/13] Performing build step for 'spm_subimage'
ninja: no work to do.
[1/5] Performing build step for 'hci_rpmsg_subimage'
ninja: no work to do.
[2/3] cd /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripher...ake/flash && /Applications/CMake.app/Contents/bin/cmake -E echo

-- west flash: using runner nrfjprog
runners.nrfjprog: nrfjprog --ids
Using board 960160943
-- runners.nrfjprog: Flashing file: /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/zephyr/merged_domains.hex
-- runners.nrfjprog: Generating CP_NETWORK hex file /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/zephyr/GENERATED_CP_NETWORK_merged_domains.hex
-- runners.nrfjprog: Generating CP_APPLICATION hex file /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/zephyr/GENERATED_CP_APPLICATION_merged_domains.hex
runners.nrfjprog: nrfjprog --program /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/zephyr/GENERATED_CP_NETWORK_merged_domains.hex --sectorerase -f NRF53 --coprocessor CP_NETWORK --snr 960160943
Parsing hex file.
Erasing page at address 0x1000000.
Erasing page at address 0x1000800.
：
Erasing page at address 0x1027000.
Erasing page at address 0x1027800.
Applying system reset.
Checking that the area to write is not protected.
Programming device.
runners.nrfjprog: nrfjprog --program /Users/makmorit/opt/ncs/nrf/samples/bluetooth/peripheral_uart/build/zephyr/GENERATED_CP_APPLICATION_merged_domains.hex --sectorerase -f NRF53 --coprocessor CP_APPLICATION --snr 960160943
Parsing hex file.
Erasing page at address 0x0.
Erasing page at address 0x1000.
：
Erasing page at address 0x25000.
Erasing page at address 0x26000.
Applying system reset.
Checking that the area to write is not protected.
Programming device.
runners.nrfjprog: nrfjprog --pinreset -f NRF53 --snr 960160943
Applying pin reset.
-- runners.nrfjprog: Board with serial number 960160943 flashed successfully.
bash-3.2$
```

以上で、サンプルアプリの書込みは完了になります。

[注1] nRF5 SDKで開発された[nRF52840アプリケーション](../../nRF52840_app)は、別途「ソフトデバイス（`s140_nrf52_6.1.1_softdevice.hex`などといったファイル）」というファームウェアの事前インストールが必要でしたが、本手順書のビルド手順でファームウェアを作成した場合、ソフトデバイスの事前インストールは不要となっているようです。

## サンプルアプリのデバッグ出力

nRF5340 DK（PCA10095）は、３点の仮想COMポート（USB CDC ACMポート）が用意されているようです。<br>
下記のように、仮想COMポート名は`<ボードのシリアル番号>N`形式（Nは1、3、5のいずれか）となっています。

```
bash-3.2$ ls -1 /dev/tty.usbmodem*
/dev/tty.usbmodem0009601609431
/dev/tty.usbmodem0009601609433
/dev/tty.usbmodem0009601609435
bash-3.2$
```

上記３点のうち、UARTデバッグプリントは下図のように、`/dev/tty.usbmodem0009601609435`に出力されたのを確認しています。

<img src="assets01/0024.jpg" width="600">

## サンプルアプリのテスト

サンプルアプリのテストには、Androidのスマートフォンを使用します。<br>
（今回はHUAWEI社のスマートフォン「nova lite 2」を使用）

#### テスト準備（Android側）

AndroidにインストールしたnRF Connectアプリを起動し、BLEデバイスをスキャンすると、下図のように一覧に「Nordic_UART_Service」というデバイスがリストされます。<br>
一覧右部の「Connect」をタップして、接続します。

<img src="assets02/0001.jpg" width="150">

下図のようにCLIENTタブに移りますので、右上の「DISCONNECT」というラベルの横にあるメニューをタップします。

<img src="assets02/0002.jpg" width="150">

「Discover services」を選択し、サービスディスカバリーを実行します。

<img src="assets02/0003.jpg" width="150">

一覧表示された「Nordic UART Service」をタップします。

<img src="assets02/0004.jpg" width="150">

キャラクタリスティックが一覧されますので「TX Characteristic」の横のアイコン（下に３本の矢印が向いているアイコン）をタップします。

<img src="assets02/0005.jpg" width="150">

「TX Characteristic」の横のアイコンの形が変わり、データ受信可能状態となります。

<img src="assets02/0006.jpg" width="150">

これでAndroid側の準備は完了です。

#### テスト準備（PC側）

先述したUARTプリント出力用のターミナルを開きます。<br>
（UART送信データの入力用ターミナルを兼ねます）

以下のようなコマンドを、ターミナルから実行します。

```
bash-3.2$ ls -1 /dev/tty.usbmodem*
/dev/tty.usbmodem0009601609431
/dev/tty.usbmodem0009601609433
/dev/tty.usbmodem0009601609435
bash-3.2$ screen /dev/tty.usbmodem0009601609435 115200
```

末尾が`5`になっている端末名を指定して、screenコマンドを実行すると、以下のようなUARTプリント出力用のターミナルが表示されます。

<img src="assets02/0008.jpg" width="600">

これでPC側の準備は完了です。

#### テスト実行（PC-->Android）

PC側のターミナル上で「`qwerty`」とタイプし[ENTER]キーを押下します。<br>
（ターミナルには入力文字がエコーバックされないのでご注意）

<img src="assets02/0008.jpg" width="600">

BLEトランスポートを経由し、Android側で入力文字が受信され、画面上の「TX Characteristic」に「`Value: qwerty`」と表示される事を確認します。

<img src="assets02/0007.jpg" width="150">

#### テスト実行（Android-->PC）

Android画面上の「RX Characteristic」右横のアイコンをタップすると、下図のようなテキストボックスが表示されます。<br>
文字「`asdfg`」を入力し「SEND」をタップします。

<img src="assets02/0009.jpg" width="150">

BLEトランスポートを経由し、Android側から入力文字が送信され、PC側のターミナル上に「`asdfg`」と表示される事を確認します。

<img src="assets02/0010.jpg" width="600">

最後に、Android画面右上の「DISCONNECT」をタップし、nRF5340 DKとの接続を切断してください。<br>
下図のように「DISCONNECT」の表示が「CONNECT」と変化し、切断された事を示します。

<img src="assets02/0011.jpg" width="150">

以上で、サンプルアプリのテストは完了となります。

## ご参考

#### 確認時の不具合

2021/01/06現在、残念ながら、nRF5340 DK上でサンプルが正常に動作せず、確認できておりませんでした。<br>
はっきりした原因はいまだ不明ですが、以下２点を見直したところ、nRF5340 DK上でサンプルが正常に動作するようになりました。

- アプリケーションを書込みする際は、事前にFlash ROMを全て消去する<br>
Flash ROM消去は、nRF Connect for desktopのProgrammerツールで実行できます。

- [SEGGER RTTViewer](../../Research/nRFCnctSDK_v1.4.99/INSTALLRTTVW.md)を使用せず、ターミナルにUARTデバッグプリントを表示させるようにする
