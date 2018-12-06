# nRF52840 Dongleプログラミング手順

Nordic社から無償公開されているツール「[nRF Connect](https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF-Connect-for-desktop/)」を使用し、nRF52840 Dongleにプログラムを書込む手順を記載します。

## nRF Connect for Desktopの準備

### ダウンロード

こちらのサイトにアクセスします。<br>
https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF-Connect-for-desktop/<br>
下図のような画面に遷移します。

<img src="assets_nrfcon/0001.png" width="600">

使用システムに対応するインストール媒体をダウンロードします。

今回は検証のためmacOSを使用いたします。<br>
一覧の中から「nRF-Connect-macOS」をクリックしてダウンロードを実行すると「nrfconnect-2.6.0.dmg」というファイルがダウンロードされます。

### インストール

ダウンロードされた「nrfconnect-2.6.0.dmg」をダブルクリックしてインストールを実行します。<br>
下図のような画面が表示されたら、アプリケーションアイコンを「Applications」フォルダーにドラッグ＆ドロップします。

<img src="assets_nrfcon/0002.png" width="350">

アプリケーションフォルダーに「nRF Connect」のアイコンができていれば、nRF Connect for Desktopのインストールは完了です。

<img src="assets_nrfcon/0003.png" width="450">

### プログラミングツールのインストール

nRF Connectを起動したら、上の「Add/remove apps」ボタンをクリックします。

<img src="assets_nrfcon/0004.png" width="450">

一覧の中の「Programmer [official]」をインストールします。

<img src="assets_nrfcon/0005.png" width="450">

「Installed」という緑色のアイコンが表示されれば、プログラミングツールのインストールは完了です。

<img src="assets_nrfcon/0006.png" width="450">

以上で、nRF Connect for Desktopの準備ができました。

## プログラムの書込み

### 書込み準備

nRF Connectを起動します。<br>
画面上部の「Launch app」ボタンをクリックすると、Programmerという項目が表示されます。<br>
右横の「Launch」ボタンをクリックします。

<img src="assets_nrfcon/0007.png" width="450">


プログラミングツールが起動します。<br>
nRF52840 Dongleが、PCのUSBポートに挿されていない場合は、画面右上部に「No devices available」と表示されます。

<img src="assets_nrfcon/0008.png" width="450">

ここで、nRF52840 DongleをUSBポートに挿します。

nRF52840 Dongle上のリセットボタンを１回押下します。<br>
nRF52840 Dongle上のLED（赤色）が点滅します。

【ご参考】<br>
nRF52840 Dongleのリセットボタンを押下することにより、ブートローダーモードとなり、アプリケーションプログラムではなく、DFUブートローダープログラムが起動し、nRF Connectのプログラミングツールで書込みが可能となります。

<img src="assets_nrfcon/0012.png" width="600">


その後、画面嬢の「Select device」ボタンをクリックします。

<img src="assets_nrfcon/0009.png" width="450">

ドロップダウンリストから、目的のデバイスを選択します。

<img src="assets_nrfcon/0010.png" width="450">

下図のように、メモリーマップが画面左側に表示されます。<br>
nRF52840 Dongleに、まだプログラミングが書込みされていない場合、下図のようにブランクになっているのがわかります。

<img src="assets_nrfcon/0011.png" width="450">

### 書込み実行

書き込みたいhexファイルを画面の中央部にドラッグ＆ドロップします。

BLEデバイスとして使用する場合は、ソフトデバイスが必要ですので、ソフトデバイス --> アプリケーションの順にドラッグ＆ドロップさせるようにします。<br>
ソフトデバイスは「/nRF5_SDK_15.2.0/components/softdevice/s140/hex/s140_nrf52_6.1.0_softdevice.hex」というファイルを使用します。

書込むアプリケーションは、NetBeans環境で作成した「nrf52840_xxaa.hex」というファイルです。<br>
以下のパスに作成されるかと存じます。
```
GitHub/onecard-fido/nRF5_SDK_v15.2.0/examples/diverta/one_card_peripheral_app/pca10056/s140/armgcc/\_build/nrf52840_xxaa.hex
```

下図は、ソフトデバイス、アプリケーションの.hexファイルをドラッグ＆ドロップした後のイメージになります。

<img src="assets_nrfcon/0013.png" width="450">

画面右下の「Write」ボタンをクリックすると、プログラムの書込み（ダウンロード）がスタートします。

<img src="assets_nrfcon/0014.png" width="450">

プログラム書込みが終了すると、下図のようなメッセージが表示されます。<br>
これはnRF52840 Dongleがブートローダーモードではなく、アプリケーションが実行中になっているためのものですので、異常ではあrません。

適宜、nRF Connectを終了させてください。

<img src="assets_nrfcon/0015.png" width="450">

以上で、nRF52840 Dongleへのプログラムの書込みは完了になります。
