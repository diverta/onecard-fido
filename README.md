# セキュリティデバイス実装対応

Nordic社のSoC「nRF5」にFIDO2認証機能等を実装するプロジェクトです。

## プログラム

- <b>[nRF5340アプリケーション](nRF5340_app)</b><br>
FIDO2認証機能を実装したファームウェアです。<br>
Nordic社のSoC「nRF5340」で動作し、[BT40 Slim Board](FIDO2Device/BT40SlimBoard)（現在開発中）に書き込んで使用します。

- <b>[nRF52840アプリケーション](nRF52840_app)</b><br>
FIDO2認証機能、PIV／OpenPGPカードエミュレーション機能を実装したファームウェアです。<br>
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

- <b>[MDBT50Q Dongle Mini](FIDO2Device/MDBT50Q_Dongle_mini/README.md)</b><br>
上記「[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)」を小型化し、USBドングルユースに特化したデバイスです。<br>
近年PCで採用が増大している小型USBコネクター<b>「USB Type-C プラグ」</b>を装備します。<br>
PCのUSBポート装着時はUSB HID／CCID／BLEセントラルデバイスとして稼働します。

## 機能イメージについて

詳細につきましては、ドキュメント<b>[「セキュリティデバイス実装対応について」](https://github.com/diverta/onecard-fido/wiki/セキュリティデバイス実装対応について)</b>をご参照願います。

## What's new

#### 2022/03/31

nRF5340アプリケーションのBLE関連機能をバージョンアップしました。<br>
これに伴い、FIDO認証器管理ツール（macOS版／Windows版）、ファームウェアを修正しております。<br>
詳細につきましては、ドキュメント<b>[「Release 202203」](https://github.com/diverta/onecard-fido/releases/tag/Release_202203)</b>をご参照願います。

#### 2022/03/03

管理ツールに、OpenPGP機能で使用するPIN番号等の設定メニューを新設しました。<br>
これに伴い、FIDO認証器管理ツール（macOS版／Windows版）、ファームウェアを修正しております。<br>
詳細につきましては、ドキュメント<b>[「Release 202202」](https://github.com/diverta/onecard-fido/releases/tag/Release_202202)</b>をご参照願います。

#### 2022/01/24

管理ツールに、OpenPGP機能設定メニューを新設しました。<br>
これに伴い、FIDO認証器管理ツール（macOS版／Windows版）、ファームウェアを修正しております。<br>
詳細につきましては、ドキュメント<b>[「Release 202201」](https://github.com/diverta/onecard-fido/releases/tag/Release_202201)</b>をご参照願います。

#### 2021/12/15

nRF52840アプリケーションを、nRF5 SDK v17.1.0に移行しました。<br>
これに伴い、FIDO認証器管理ツール、ファームウェアを修正しました。<br>
詳細につきましては、ドキュメント<b>[「Release 202105」](https://github.com/diverta/onecard-fido/releases/tag/Release_202105)</b>をご参照願います。

#### [過去の更新履歴はこちら](HISTORY.md)
