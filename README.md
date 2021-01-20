# セキュリティデバイス実装対応

Nordic社のSoC「nRF5」にFIDO2認証機能等を実装するプロジェクトです。

## プログラム

- <b>[nRF52840アプリケーション](nRF52840_app)（！！！現在移行作業中！！！）</b><br>
FIDO2認証機能、PIVカードエミュレーション機能を実装したファームウェアです。<br>
Nordic社のSoC「nRF52840」で動作し、[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)に書き込んで使用します。

- <b>[FIDO認証器管理ツール](MaintenanceTool)</b><br>
[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)に、FIDO2認証機能用の鍵・証明書・PINを導入するために使用する、デスクトップ・ツールです。<br>
[Windows版](MaintenanceTool/WindowsExe)、[macOS版](MaintenanceTool/macOSApp)の両方を用意しております。

- <b>[オープンソースコードライセンスについて](OSSL.md)</b><br>
上記プログラム内で使用されているオープンソースコード（ライブラリー）についての概要を掲載しています。

## ハードウェア

- <b>[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)</b><br>
Nordic社のSoC「nRF52840」を使用したFIDO2認証器です。<br>
日本国内の技適取得済みであるnRF52840搭載モジュール「MDBT50Q」を使用し、nRF52840 Dongleとほぼ同じ仕様で製作しております。<br>
PCのUSBポート装着時はUSB HID／CCID／BLEセントラルデバイスとして稼働し、ボタン乾電池装着時はBLEペリフェラルデバイスとして稼働します。

- <b>MDBT50Q Dongle Mini（！！！現在開発中！！！）</b><br>
上記「[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)」を小型化し、USBドングルユースに特化したデバイスです。<br>
近年PCで採用が増大している小型USBコネクター<b>「USB Type-C プラグ」</b>を装備します。<br>
PCのUSBポート装着時はUSB HID／CCID／BLEセントラルデバイスとして稼働します。

## What's new

#### 2021/01/07

FIDO認証器管理ツール、ファームウェアを修正しました。<br>
詳細につきましては、ドキュメント<b>[「Release 202101」](https://github.com/diverta/onecard-fido/releases/tag/Release_202101)</b>をご参照願います。

#### [過去の更新履歴はこちら](HISTORY.md)

## 機能イメージについて

詳細につきましては、ドキュメント<b>[「セキュリティデバイス実装対応について」](https://github.com/diverta/onecard-fido/wiki/セキュリティデバイス実装対応について)</b>をご参照願います。
