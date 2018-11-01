# nRF52840関連情報

nRF52840で動作するアプリの開発に関する各種情報を掲載しています。

## 各種手順

* <b>[NetBeans開発環境構築手順](NETBEANS.md)</b><br>
NetBeansとARM GCC、nRF5 SDKを使用し、nRF52840の開発環境を構築する手順を掲載しています。

* <b>[NFCサンプルアプリ動作確認手順](NDEFSAMPLE.md)</b><br>
サンプルアプリ[`Writable NDEF Message Example`](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.0.0%2Fnfc_writable_ndef_msg.html&cp=4_0_1_4_7_6)を使用して、nRF52840のNFC機能を確認する手順を掲載しています。

* <b>[WIP] [USB HIDサンプルアプリ動作確認手順](HIDSAMPLE.md)</b><br>
サンプルアプリ[`USB HID Generic Example`](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v15.2.0/usbd_hid_generic_example.html?cp=4_0_0_4_5_50_6)を使用して、nRF52840のUSB HID機能を確認する手順を掲載しています。

## ご参考

* <b>[micro-eccビルド手順 (macOS版)](BUILDMECC.md) </b><br>
マイコン環境で使用できる暗号化ライブラリー「micro-ecc」の構築手順を掲載しています。<br>
【2018/10/16追記】nRF52840版 BLE U2Fサービスでは、nRF52840内に実装されている「ARM Cryptocell-310」を使用しますので、micro-eccは使用しません。
