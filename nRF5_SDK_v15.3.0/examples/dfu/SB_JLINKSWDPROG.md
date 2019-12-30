# USBブートローダー書込手順

MDBT50Q Dongleに、USBブートローダーをJ-Link経由で書込みする手順を記載します。

## 書込み準備

書込みに必要な環境と媒体を準備します。

### 動作確認時の環境

- macOS Sierra（10.12.6）
- nRF52840 DK（PCA10056）: プログラムの書込みに使用
- MDBT50Q Dongle（nRF52840）: プログラムの書込み先となるターゲット基板

### ハードウェアの準備

まず最初に、MDBT50Q Dongleの背面にあるボタン電池ケースに、<b>電池が入っていないこと</b>を必ず確認します。

<img src="assets02/0000.png" width="400">

次に、MDBT50Q DongleをPCのUSBポートに装着後、nRF52840 DKと接続します。<br>
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

<img src="assets02/0001.jpg" width="540">

### ファームウェアの準備

ファームウェアは、すでにビルド済みの`.hex`ファイルが、GitHubリポジトリーの以下の場所に格納されています。
- ディレクトリー: [/nRF5_SDK_v15.3.0/firmwares/secure_bootloader/](../../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader)
- アプリケーション: [nrf52840_xxaa.hex](../../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader/nrf52840_xxaa.hex)
- マスターブートレコード: [mbr_nrf52_2.4.1_mbr.hex](../../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader/mbr_nrf52_2.4.1_mbr.hex)
- ソフトデバイス: [s140_nrf52_6.1.1_softdevice.hex](../../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader/s140_nrf52_6.1.1_softdevice.hex)

### 書込み用ツールの準備

書込み用ツール「nRF Connect for Desktop」を、あらかじめPCに導入しておきます。<br>
詳細につきましては、手順書[「nRF Connect for Desktop導入手順」](../../../nRF5_SDK_v15.3.0/NRFCONNECTINST.md)をご参照ください。

## USBブートローダーの書込み

### 書込み準備

nRF Connectを起動します。<br>
画面上部の「Launch app」ボタンをクリックすると、Programmerという項目が表示されます。<br>
右横の「Launch」ボタンをクリックします。

<img src="assets03/0001.png" width="450">

プログラミングツールが起動します。<br>
右側の「File Memory Layout」欄がブランクになっていることを確認します。

ブランクになっていない場合は、右側の「Clear Files」というリンクをクリックして「File Memory Layout」欄をブランクにしてください。

<img src="assets03/0002.png" width="450">

「File Memory Layout」欄に、先述のファイル２点をドラッグ＆ドロップします。<br>
かならず、 [mbr_nrf52_2.4.1_mbr.hex](../../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader/mbr_nrf52_2.4.1_mbr.hex) --> [nrf52840_xxaa.hex](../../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader/nrf52840_xxaa.hex)の順でドラッグ＆ドロップしてください。

２点のファイルが、「File Memory Layout」欄に、下図のように配置されることを確認します。

<img src="assets03/0003.png" width="450">

ここで、MDBT50Q DongleをUSBポートに挿します。<br>
その後、画面左上部の「Select device」プルダウンをクリックして、PCA10056（前述のnRF52840 DK）を選択します。

<img src="assets03/0004.png" width="450">

しばらくすると、左側の「nRF52840」欄に、nRF52840 DKに接続されているnRF52840のメモリーイメージが表示されます。<br>
（図では、新品のMDBT50Q Dongleに接続したところですので、メモリーイメージがブランクになっています）

<img src="assets03/0005.png" width="450">

これで書き込み準備は完了です。

### 書込み実行

画面右下部にある「Erase & write」のリンクをクリックし、書込みをスタートさせます。<br>
下図のように「nRF52840」欄に縞模様が表示され、書込処理が進みます。

<img src="assets03/0006.png" width="450">

しばらくすると、下図のように画面下部のメッセージ欄が赤く変化します。<br>
画面左上部にあるプルダウンから「Close device」を選択し、MDBT50Q Dongleとの接続をすみやかに切断してください。

<img src="assets03/0007.png" width="450">

切断が完了したら、画面の「Quit」を実行して、nRF Connectを終了させます。

<img src="assets03/0008.png" width="450">

## ソフトデバイスの追加書込み

続いて、ソフトデバイス（s140_nrf52_6.1.1_softdevice.hex）を追加で書込みます。

「nRF Command Line Tools」というツールの`nrfjprog`を使用し、下記のコマンドを実行します。

```
TOOL_DIRECTORY=${HOME}/opt/nRF-Command-Line-Tools_9_8_1_OSX/nrfjprog
OUTPUT_DIRECTORY=${HOME}/GitHub/onecard-fido/nRF5_SDK_v15.3.0/firmwares/secure_bootloader

${TOOL_DIRECTORY}/nrfjprog -f nrf52 --program ${OUTPUT_DIRECTORY}/s140_nrf52_6.1.1_softdevice.hex --sectorerase
```


以下は実行例になります。

```
MacBookPro-makmorit-jp:~ makmorit$ TOOL_DIRECTORY=${HOME}/opt/nRF-Command-Line-Tools_9_8_1_OSX/nrfjprog
MacBookPro-makmorit-jp:~ makmorit$ OUTPUT_DIRECTORY=${HOME}/GitHub/onecard-fido/nRF5_SDK_v15.3.0/firmwares/secure_bootloader
MacBookPro-makmorit-jp:~ makmorit$  
MacBookPro-makmorit-jp:~ makmorit$ ${TOOL_DIRECTORY}/nrfjprog -f nrf52 --program ${OUTPUT_DIRECTORY}/s140_nrf52_6.1.1_softdevice.hex --sectorerase
Parsing hex file.
Erasing page at address 0x0.
Erasing page at address 0x1000.
（中略）
Erasing page at address 0x24000.
Erasing page at address 0x25000.
Applying system reset.
Checking that the area to write is not protected.
Programming device.
MacBookPro-makmorit-jp:~ makmorit$
```


その後、MDBT50Q DongleをPCのUSBポートから外し、nRF52840 DKとの配線を外してください。

## ブートローダーの起動確認

MDBT50Q Dongleが、PCのUSBポートに装着されている状態で、MDBT50Q Dongleの基板上のリセットボタンを１回押すと、ブートローダーが起動し、ブートローダーモードに遷移します。

下図のように、基板上のLED2（緑色）、LED3（黄色）が同時点灯していることを確認してください。

<img src="assets03/0009.png" width="350">

これで、USBブートローダーの書込みは完了となります。
