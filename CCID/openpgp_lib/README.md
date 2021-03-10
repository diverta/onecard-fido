# OpenPGPカードエミュレーション機能

## 概要
OpenPGPカードと同等の機能（OpenPGPカードエミュレーション機能）をnRF52840上に実装するためのモジュールです。

プラットフォーム（nRF5 SDK）に依存する部分と、依存しない部分に分かれています。

## 前提要件

#### 専用CCIDドライバーのインストール（macOSのみ）

OpenPGPカードエミュレーション機能は、USB CCIDインターフェース上で動作します。<br>
他方、macOSに標準導入されているCCIDインターフェースは、[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)をサポートしておりません。

したがって、macOS環境でOpenPGPカードエミュレーション機能を利用するためには、[専用CCIDドライバー](../../CCID/INSTALLPRG.md)[注1]がインストールされている必要があります。

[注1] macOS環境上で、MDBT50Q DongleをUSB CCIDデバイスとして認識できるよう、カスタマイズされています。詳細につきましては[別ドキュメント](CCID/ccid_lib/README.md)をご参照願います。
