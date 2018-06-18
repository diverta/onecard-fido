# HIDマウスデバイスを試す

NUCLEO（STM32開発環境）＋mbed OSにより、USB HIDデバイスが手軽に試行できるようです。

まずは手始めに、HIDマウスデバイスを試してみました。<br>
こちらの記事を参考にいたしました。<br>
https://qiita.com/eggman/items/9e7dad30e4d2833af17f

## サンプルアプリの書込み

サンプルのmbedアプリケーションをNUCLEOに書き込むため、NUCLEOとPCを接続しておきます。<br>
その後、mbedオンライン・コンパイラーを開いて、下記URLからサンプルアプリをインポートします。<br>
http://os.mbed.com/teams/ST/code/Nucleo_usbmouse/

コンパイルを実行します。

<img src="assets/0005.png" width="700">

ダウンロードフォルダーに、`NucleoF411RE_usbmouse_NUCLEO_F411RE.bin`というファイルが作成されます。<br>
そのアイコンを`NODE_F411RE`というドライブにドラッグ＆ドロップし、NUCLEOに書き込みます。

<img src="assets/0006.png" width="400">

書き込みが完了したら、NUCLEOとPCを切り離しておきます。

## ハードウェアのセットアップ

PCのUSBポートと、NUCLEOのUSB信号線を接続させるため、下図のような治具を作成しました。

<img src="assets/0001.png" width="400">

下記URLの変換基板から配線を出しているだけのものです。<br>
http://akizukidenshi.com/catalog/g/gK-06656/

<img src="assets/0003.png" width="400">

配線の先を、NUCLEOの`CN10`の下図位置に接続します。

<img src="assets/0002.png" width="400">

USB変換基板とNUCLEO間の配線は下記のようになります。

|名称 |USB変換基板側 |NUCLEO側 |備考 |
|:-:|:--|:--|:--|
|D- |Pin #2 |CN10 #14 |図の緑色の線 |
|D+ |Pin #3 |CN10 #12 |図の黄色の線 |
|GND |Pin #5 |CN10 #20 |図の青色の線 |

## 動作確認

PCとUSB変換基板をUSBケーブルで接続したのち、NUCLEOとPCをUSBケーブルで接続します。<br>
イメージは下図のようになります。

<img src="assets/0004.png" width="400">

程なく、下図のようにマウスポインターが画面内を回ります。

<img src="assets/0007.png" width="300">

<img src="assets/0008.png" width="300">

<img src="assets/0009.png" width="300">

<img src="assets/0010.png" width="300">

## システムレポートによる確認

macOSのシステムレポート画面で「USB装置ツリー」を開くと、上記ハードウェアが、HIDデバイスとして下図のように認識されることが確認できます。

<img src="assets/0011.png" width="600">
