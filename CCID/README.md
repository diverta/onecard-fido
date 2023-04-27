# CCIDインターフェース対応

最終更新日：2023/1/16

## 概要

セキュリティデバイス用のインターフェースとして使用されている、[USB CCIDインターフェース](../CCID/ccid_lib/README.md)を、[nRF52840アプリケーション](../nRF52840_app)／[nRF5340アプリケーション](../nRF5340_app)に追加実装します。

## 目的

PIV Card、OpenPGP Cardなどといったスマートカードのエミュレーションを、[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)上で実行させるために必要となります。

## 機能

#### [PIVカードエミュレーション](../CCID/PIV/README.md)

PIVカードの仕様に準拠した機能です。<br>
MDBT50Q DongleをPCに装着すると、macOSにPIN番号を使ってログインできるようになります。<br>
（最終更新日現在、Windows Helloには未対応です）

#### [OpenPGPカードエミュレーション](../CCID/OpenPGP/README.md)

OpenPGPカードの仕様に準拠した機能です。<br>
PGP公開鍵でファイルを暗号化すると、PGP秘密鍵がインストールされたMDBT50Q Dongleを使用時のみ復号化可、とすることができます。<br>
（最終更新日現在、macOS環境、Windows環境に対応しています。）

## 各種手順書

- <b>[PIN番号を使用したmacOSログイン確認手順](../CCID/PIV/PIVPINLOGIN.md)</b><br>
MDBT50Q DongleのPIV機能を使用し、PIN番号によりmacOSにログインするための確認手順について掲載します。

- <b>[CCIDドライバーインストール手順](../CCID/INSTALLPRG.md)</b><br>
CCIDドライバーをmacOS環境にインストールし、MDBT50Q DongleのCCIDインターフェースを利用できるようにするための手順について掲載しています。<br>
（Windows環境では、CCIDドライバーが最初からシステムに組み込まれているため、CCIDドライバーのインストールは不要です）

- <b>[CCIDドライバー修正ビルド手順](../CCID/BUILDCCIDDRV.md)</b><br>
macOSにプレインストールされているCCIDドライバーを修正ビルドする手順について掲載しています。
