# セキュリティデバイス実装対応

最終更新日：2023/1/16

Nordic社のSoC「nRF5」にFIDO2認証機能等を実装するプロジェクトです。

## プログラム

- <b>[nRF5340アプリケーション](nRF5340_app)</b><br>
FIDO2認証機能、PIV／OpenPGPカードエミュレーション機能を実装したファームウェアです。<br>
Nordic社のSoC「nRF5340」で動作し、[MDBT53 Dongle](FIDO2Device/MDBT53_Dongle)（現在開発中）に書き込んで使用します。

- <b>[nRF52840アプリケーション](nRF52840_app)</b><br>
FIDO2認証機能、PIV／OpenPGPカードエミュレーション機能を実装したファームウェアです。<br>
Nordic社のSoC「nRF52840」で動作し、[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)に書き込んで使用します。

- <b>[FIDO認証器管理ツール](MaintenanceTool)</b><br>
PC環境から、[FIDO認証器](FIDO2Device)の動作に必要な各種設定／動作テスト等を行う、デスクトップ・ツールです。<br>
エンドユーザー向けと、ベンダー向けの両タイプを用意しております。<br>
最終更新日現在、[macOS版](MaintenanceTool/macOSApp/README.md)を用意しております。<br>
（Windows版は、ただいま再構築作業中です。しばらくお待ち願います）

- <b>[オープンソースコードライセンスについて](OSSL.md)</b><br>
上記プログラム内で使用されているオープンソースコード（ライブラリー）についての概要を掲載しています。

## ハードウェア

- <b>[MDBT53 Dongle](FIDO2Device/MDBT53_Dongle/README.md)（現在開発中）</b><br>
Nordic社のSoC「nRF5340」を使用したFIDO2認証器です。<br>
日本国内の技適取得済みであるnRF5340搭載モジュール「MDBT53」を使用し、nRF5340 DKを簡略化した仕様で製作しています。<br>
近年PCで採用が増大している小型USBコネクター<b>「USB Type-C プラグ」</b>を装備します。<br>
PCのUSBポート装着時はUSB HID／CCIDデバイスとして稼働し、ボタン乾電池装着時はBLEペリフェラルデバイスとして稼働します。

- <b>[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)</b><br>
Nordic社のSoC「nRF52840」を使用したFIDO2認証器です。<br>
日本国内の技適取得済みであるnRF52840搭載モジュール「MDBT50Q」を使用し、nRF52840 Dongleとほぼ同じ仕様で製作しております。<br>
PCのUSBポート装着時はUSB HID／CCIDデバイスとして稼働し、ボタン乾電池装着時はBLEペリフェラルデバイスとして稼働します。

## 機能イメージについて

詳細につきましては、ドキュメント<b>[「セキュリティデバイス実装対応について」](https://github.com/diverta/onecard-fido/wiki/セキュリティデバイス実装対応について)</b>をご参照願います。

## What's new

#### 2022/11/15

FIDO認証器管理ツール（macOS版／Windows版）を更新いたしました<br>
詳細につきましては、ドキュメント<b>[「Release 202205」](https://github.com/diverta/onecard-fido/releases/tag/Release_202205)</b>をご参照願います。

#### 2022/07/25

FIDO認証器管理ツール（macOS版／Windows版）から、ベンダー向け機能をFIDO認証器開発ツール（macOS版／Windows版）に切り出しました。<br>
これに伴い、nRF5340ファームウェアを修正しております。<br>
詳細につきましては、ドキュメント<b>[「Release 202204」](https://github.com/diverta/onecard-fido/releases/tag/Release_202204)</b>をご参照願います。

#### [過去の更新履歴はこちら](HISTORY.md)
