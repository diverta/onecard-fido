# nRF52840 Dongle調査用アプリ

## ble_peripheral_logger_proj

Nordicの[USB CDCサービスのサンプルアプリ](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.2.0%2Fusbd_cdc_acm_example.html&cp=4_0_0_4_5_50_3)を改修し、Dongle側で収集したRSSIログを、USBポートを経由しPCのコンソールアプリに送信・表示できるアプリケーションに転化させています。

最大３台までのBLEデバイスについて、１秒間隔で各々のデバイスのRSSI値をCSV形式で出力します。

### 動作確認環境

- macOS または Windows 10
- NetBeans IDE（8.2）
- Java SE Runtime Environment（1.8.0_131）
- nRF52840 Dongle（PCA10059）

### nRF52840 Dongleの準備

NetBeansでプロジェクトを作成・ビルドを行っています。<br>
（NetBeansに関しては、[こちらのドキュメント](../../../Development/nRF52840/NETBEANS.md)をご参照）<br>
NetBeansによりビルドされたHEXファイルは、サブディレクトリー [ble_peripheral_logger_firmwares](ble_peripheral_logger_firmwares) に格納しております。

[ble_peripheral_logger_firmwares](ble_peripheral_logger_firmwares)配下に格納された２本のHEXファイルを、下記手順によりnRF52840 Dongleに書込みます。<br>
　<b>[nRF52840 Dongleプログラミング手順](https://github.com/diverta/onecard-fido/blob/master/Development/nRF52840/NRFCONNECTINST.md)</b>

### BLEデバイスの準備

Nordicの[BLE UARTサンプルアプリ](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v15.2.0/ble_sdk_app_nus_eval.html?cp=4_0_0_4_1_2_24)を導入したBLEペリフェラルデバイスについて、RSSI値を測定します。

従いまして、別途 nRF52840 DK、または nRF52832 DK をご用意いただき、前述サンプルアプリを導入願います。<br>
こちらでも、[NetBeansプロジェクト](https://github.com/diverta/onecard-fido/tree/master/nRF5_SDK_v15.2.0/examples/ble_peripheral)を、以下の通り用意しております。
- nRF52832 DK用 - [https://github.com/diverta/onecard-fido/tree/master/nRF5_SDK_v15.2.0/examples/ble_peripheral/ble_app_uart_test](../examples/ble_peripheral/ble_app_uart_test)
- nRF52840 DK用 - [https://github.com/diverta/onecard-fido/tree/master/nRF5_SDK_v15.2.0/examples/ble_peripheral/ble_app_uart_test_nRF52840](../examples/ble_peripheral/ble_app_uart_test_nRF52840)

### 動作確認方法

まず、Windows 10環境での確認用としては、シリアル通信用のコンソールアプリ（Tera Term 等）を別途インストールしておきます。<br>
macOS環境の場合は、システムに同梱のターミナルアプリ上で、screenコマンドを実行すれば確認できます。

次に、nRF52840 Dongleを、PCのUSBポートに装着します。<br>
その後、シリアル通信用のコンソールアプリ（Tera Term や screenコマンド）を起動します。

コンソールアプリの接続先としましては、nRF52840 Dongleの仮想COMポートを接続するようにしてください。<br>
接続ボーレートは 115200 になります。

#### ログ出力形式

RSSI値のログデータは、以下のCSV形式で出力されます。<br>
```
<秒通番>,<デバイス1のBluetoothアドレス>,<デバイス1のRSSI値>,<デバイス2のBluetoothアドレス>,<デバイス2のRSSI値>,<デバイス3のBluetoothアドレス>,<デバイス3のRSSI値>
```
デバイスが検出されない場合は、ブランクのCSVが表示されます。

#### Windows環境での確認例

下図は、Tera Term Version 4.95 を使用して確認した例になります。<br>
`COM5`というのが、仮想COMポート名です。

<img src="assets/0001.png" width="640">

#### macOS環境での確認例
下図は、screenコマンドを使用して確認した例になります。<br>
`/dev/tty.usbmodem1411`というのが、仮想COMポート名です。

<img src="assets/0002.png" width="640">
