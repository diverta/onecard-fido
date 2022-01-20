# セキュリティデバイス実装対応

Nordic社のSoC「nRF5」にFIDO2認証機能等を実装するプロジェクトです。

## プログラム

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

#### 2021/12/15

nRF52840アプリケーションを、nRF5 SDK v17.1.0に移行しました。<br>
これに伴い、FIDO認証器管理ツール、ファームウェアを修正しました。<br>
詳細につきましては、ドキュメント<b>[「Release 202105」](https://github.com/diverta/onecard-fido/releases/tag/Release_202105)</b>をご参照願います。

#### 2021/12/09

FIDO認証器管理ツール（Windows版）を修正しました。<br>
詳細につきましては、ドキュメント<b>[「Release 202104」](https://github.com/diverta/onecard-fido/releases/tag/Release_202104)</b>をご参照願います。

#### 2021/11/24

FIDO認証器管理ツール（macOS版）を修正しました。<br>
詳細につきましては、ドキュメント<b>[「Release 202103」](https://github.com/diverta/onecard-fido/releases/tag/Release_202103)</b>をご参照願います。

#### 2021/04/29

MDBT50Q Dongle Miniを試作いたしました。

- <b>[MDBT50Q Dongle Mini（Rev1）](https://github.com/diverta/onecard-fido/blob/doc-20210429/FIDO2Device/MDBT50Q_Dongle_mini/README.md)</b>

※[ファームウェア](https://github.com/diverta/onecard-fido/blob/doc-20210429/nRF52840_app/firmwares/README.md)は、[MDBT50Q Dongle（Rev2.1.2）](https://github.com/diverta/onecard-fido/blob/doc-20210429/FIDO2Device/MDBT50Q_Dongle/README.md)と同一です。

#### 2021/03/15

nRF52840アプリケーションに、OpenPGPカードエミュレーション機能（ベータ）を新規搭載しました。<br>
（まだ開発途上です）<br>
これに伴い、ファームウェアを修正しました。<br>
詳細につきましては、ドキュメント<b>[「OpenPGPカードエミュレーション対応」](https://github.com/diverta/onecard-fido/blob/doc-20210311/CCID/OpenPGP/README.md)</b>をご参照願います。

#### [過去の更新履歴はこちら](HISTORY.md)
