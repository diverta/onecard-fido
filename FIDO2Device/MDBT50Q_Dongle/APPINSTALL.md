# アプリケーション書込み手順

nRF52840 DKに同梱されているJ-LinkのSWDインターフェースを使用して、MDBT50Q Dongleにアプリケーションを書き込む手順を掲載いたします。

## 書込み準備

### 動作確認時の環境

- macOS Sierra（10.12.6）
- nRF52840 DK（PCA10056）: プログラムの書込みに使用
- MDBT50Q Dongle（nRF52840）: プログラムの書込み先となるターゲット基板

### ハードウェアの準備

MDBT50Q DongleをPCのUSBポートに装着後、nRF52840 DKと接続します。<br>
接続するピンの対応関係は以下の通りです。

|ピンの名前 |MDBT50Q Dongle | | nRF52840 DK|
|:--|:-:|:-:|:-:|
|0V |GND  | <-->  |GND|
|SWD IO |PIO  | <-->  |SWDIO|
|SWD Clock |PCLK  | <--  |SWDCLK|
|SWD IO Level |VDD  | -->  |VTG|
|SWD Reset |RST  | <--  |RESET|

[注1] nRF52840 DK上の「P20」というコネクター（オスピン）に接続します。<br>
[注2] MDBT50Q Dongleの回路図はこちら（[FIDO2AUTH_001.pdf](https://github.com/diverta/onecard-fido/blob/master/FIDO2Device/pcb/FIDO2AUTH_001.pdf)）になります。

下図は実際に両者を接続した時のイメージになります。

<img src="assets/0001.jpg" width="540">

### ファームウェアの準備

ファームウェアは、すでにビルド済みの`.hex`ファイルが、GitHubリポジトリーの以下の場所に格納されています。
- ディレクトリー: onecard-fido/nRF5_SDK_v15.3.0/firmwares/
- アプリケーション: [nrf52840_xxaa.hex](../../nRF5_SDK_v15.3.0/firmwares/nrf52840_xxaa.hex)
- ソフトデバイス: [s140_nrf52_6.1.1_softdevice.hex](../../nRF5_SDK_v15.3.0/firmwares/s140_nrf52_6.1.1_softdevice.hex)

[アプリケーション](../../nRF5_SDK_v15.3.0/)をビルドして書込みたい場合は、後述手順「ソースコードからビルドして書込み実行」をご参照ください。

## ファームウェアの書込み実行

前述のファームウェア（`.hex`ファイル群）を、nRFコマンドラインツールの`nrfjprog`[注1]で書込みます。<br>
使用するコマンドは下記になります。

#### Flash ROMを全消去し、ソフトデバイスを書込み（初回の作業）
```
TOOL_DIRECTORY=${HOME}/opt/nRF-Command-Line-Tools_9_8_1_OSX/nrfjprog
OUTPUT_DIRECTORY=${HOME}/GitHub/onecard-fido/nRF5_SDK_v15.3.0/firmwares
${TOOL_DIRECTORY}/nrfjprog -f nrf52 --eraseall
${TOOL_DIRECTORY}/nrfjprog -f nrf52 --program ${OUTPUT_DIRECTORY}/s140_nrf52_6.1.1_softdevice.hex --sectorerase
${TOOL_DIRECTORY}/nrfjprog -f nrf52 --reset
```

#### アプリケーションを書込み（アプリケーション更新時の作業）
```
TOOL_DIRECTORY=${HOME}/opt/nRF-Command-Line-Tools_9_8_1_OSX/nrfjprog
OUTPUT_DIRECTORY=${HOME}/GitHub/onecard-fido/nRF5_SDK_v15.3.0/firmwares
${TOOL_DIRECTORY}/nrfjprog -f nrf52 --program ${OUTPUT_DIRECTORY}/nrf52840_xxaa.hex --sectorerase
${TOOL_DIRECTORY}/nrfjprog -f nrf52 --reset
```

以下は実行例になります。
```
MacBookPro-makmorit-jp:bin makmorit$ TOOL_DIRECTORY=${HOME}/opt/nRF-Command-Line-Tools_9_8_1_OSX/nrfjprog
MacBookPro-makmorit-jp:bin makmorit$ OUTPUT_DIRECTORY=${HOME}/GitHub/onecard-fido/nRF5_SDK_v15.3.0/firmwares
MacBookPro-makmorit-jp:bin makmorit$ ${TOOL_DIRECTORY}/nrfjprog -f nrf52 --program ${OUTPUT_DIRECTORY}/nrf52840_xxaa.hex --sectorerase
Parsing hex file.
Erasing page at address 0x26000.
Erasing page at address 0x27000.
Erasing page at address 0x28000.
Erasing page at address 0x29000.
Erasing page at address 0x2A000.
Erasing page at address 0x2B000.
Erasing page at address 0x2C000.
Erasing page at address 0x2D000.
Erasing page at address 0x2E000.
Erasing page at address 0x2F000.
Erasing page at address 0x30000.
Erasing page at address 0x31000.
Erasing page at address 0x32000.
Erasing page at address 0x33000.
Erasing page at address 0x34000.
Erasing page at address 0x35000.
Erasing page at address 0x36000.
Erasing page at address 0x37000.
Erasing page at address 0x38000.
Erasing page at address 0x39000.
Erasing page at address 0x3A000.
Erasing page at address 0x3B000.
Erasing page at address 0x3C000.
Erasing page at address 0x3D000.
Erasing page at address 0x3E000.
Erasing page at address 0x3F000.
Erasing page at address 0x40000.
Erasing page at address 0x41000.
Erasing page at address 0x42000.
Erasing page at address 0x43000.
Erasing page at address 0x44000.
Erasing page at address 0x45000.
Erasing page at address 0x46000.
Erasing page at address 0x47000.
Erasing page at address 0x48000.
Erasing page at address 0x49000.
Erasing page at address 0x4A000.
Erasing page at address 0x4B000.
Erasing page at address 0x4C000.
Erasing page at address 0x4D000.
Erasing page at address 0x4E000.
Erasing page at address 0x4F000.
Erasing page at address 0x50000.
Erasing page at address 0x51000.
Erasing page at address 0x52000.
Erasing page at address 0x53000.
Erasing page at address 0x54000.
Erasing page at address 0x55000.
Erasing page at address 0x56000.
Erasing page at address 0x57000.
Erasing page at address 0x58000.
Erasing page at address 0x59000.
Erasing page at address 0x5A000.
Erasing page at address 0x5B000.
Erasing page at address 0x5C000.
Applying system reset.
Checking that the area to write is not protected.
Programming device.
MacBookPro-makmorit-jp:bin makmorit$ ${TOOL_DIRECTORY}/nrfjprog -f nrf52 --reset
Applying system reset.
Run.
MacBookPro-makmorit-jp:bin makmorit$
```

[注1]`nrfjprog`（nRFコマンドラインツール）のインストールにつきましては、手順書「[NetBeansインストール手順](../../nRF5_SDK_v15.3.0/NETBEANSINST.md)」をご参照願います。


コマンド実行が終了すると、MDBT50Q Dongleが自動的にリセットされ、アプリケーションがスタートします。<br>
アイドル時であることを表示する緑色のLEDが点滅していることを確認します。

<img src="assets/0014.jpg" width="400">

以上で、MDBT50Q Dongleへのアプリケーション書込みは完了になります。


## ソースコードからビルドして書込み実行

[アプリケーション](../../nRF5_SDK_v15.3.0/)をソースコードからビルドして書き込む場合は、下記手順を実行します。

#### ビルド実行

NetBeansを立ち上げ、プロジェクト「[fido2_authenticator_proj](https://github.com/diverta/onecard-fido/tree/master/nRF5_SDK_v15.3.0/examples/diverta)」を開きます。<br>
（NetBeansにつきましては、手順書「[NetBeansインストール手順](../../nRF5_SDK_v15.3.0/NETBEANSINST.md)」をご参照願います。）

Makefileを参照し、４行目が`TARGET_BOARD     := PCA10059`となっていることを<u><b>必ず確認してください</b></u>。

<img src="assets/0007.png" width="500">

#### 書込み実行

NetBeansのメニュー「プロジェクト(fido2_authenticator_proj)を実行」を実行します。

<img src="assets/0008.jpg" width="600">

ビルドが実施されていない場合はビルド（コンパイル、リンク）が実行され、続いて書き込みが実行されます。<br>
書き込みが完了すると、「実行 FINISHED; 終了値0;」などと表示されます。

<img src="assets/0009.png" width="600">

MDBT50Q Dongleが自動的にリセットされ、アプリケーションがスタートします。<br>
アイドル時であることを表示する緑色のLEDが点滅していることを確認します。

<img src="assets/0014.jpg" width="400">

緑色LEDの点滅確認が終わったら、適宜、NetBeansを終了させてください。

以上で、MDBT50Q Dongleへのアプリケーション書込みは完了になります。
