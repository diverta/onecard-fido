# CCIDインターフェース追加対応

## 概要

セキュリティデバイス用のインターフェースとして使用されている、USB CCIDインターフェースを追加する対応になります。

## 目的

PIV Card、OpenPGP Cardなどといったスマートカードのエミュレーションを、nRF52840アプリケーション上で実行させるために必要となります。

## 機能

#### PIVカードエミュレーション

PIVカードの仕様に準拠した機能です。<br>
[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)をPCに装着すると、[macOSにPIN番号を使ってログイン](../FIDO2Device/MDBT50Q_Dongle/PIVPINLOGIN.md)できるようになります。<br>
（Windows環境には未対応）

詳細につきましては、別ドキュメント（[PIVカードエミュレーション対応](../CCID/PIVCARDEMUL.md)）をご参照願います。

#### OpenPGPカードエミュレーション

OpenPGPカードの仕様に準拠した機能です。<br>
PGP公開鍵でファイルを暗号化すると、PGP秘密鍵がインストールされた[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)を使用時のみ復号化可、とすることができます。<br>
（macOS環境、Windows環境の両方に対応）

詳細につきましては、別ドキュメント（[OpenPGPカードエミュレーション対応](../CCID/OpenPGP/README.md)）をご参照願います。
