# CCIDインターフェース追加対応

## 概要

セキュリティデバイス用のインターフェースとして使用されている、USB CCIDインターフェースを追加する対応になります。

## 目的

PIV Card、OpenPGP Cardなどといったスマートカードのエミュレーションを、nRF52840アプリケーション上で実行させるために必要となります。

#### 現状

2020/12/29現在、PIV Cardエミュレーションを実装済みです。<br>
この結果、nRF52840アプリケーションを搭載した[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)を使用し、PIN番号でmacOSにログインできるようになるところまで実現できています。<br>
（詳細につきましては、下記手順書をご参照）

## 手順書

- <b>[CCIDドライバーインストール手順](../CCID/INSTALLPRG.md)</b><br>
[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のCCIDインターフェース専用ドライバー（以下単に「CCIDドライバー」）を、macOS環境にインストールする手順を掲載しています。<br>
WIndows 10環境では、CCIDドライバーのインストールは不要になります。

- <b>[PIV機能の設定手順](../MaintenanceTool/macOSApp/PIVSETTING.md)</b><br>
[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)に対し、[FIDO認証器管理ツール（macOS版）](../MaintenanceTool/macOSApp/README.md)を使用して、PIV機能に必要な各種設定を行う手順を掲載します。

- <b>[PIN番号を使用したmacOSログイン確認手順](../FIDO2Device/MDBT50Q_Dongle/PIVPINLOGIN.md)</b><br>
[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のPIV機能を使用して、macOSにPIN番号でログインする手順を掲載します。

## 手順書（ご参考）

- <b>[Yubico PIV Tool (command line) macOS版 導入手順](../CCID/PIVTOOLMACINST.md)</b><br>
PIVで使用する証明書等を導入するために利用できる「Yubico PIV Tool (command line) 」を、macOS環境に導入する手順を掲載します。

- <b>[Yubico PIV Tool (command line) Windows版 導入手順](../CCID/PIVTOOLWININST.md)</b><br>
PIVで使用する証明書等を導入するために利用できる「Yubico PIV Tool (command line) 」を、Windows 10環境に導入する手順を掲載します。

- <b>[Yubico PIV Toolによる初期データ導入手順](../CCID/YKPIVUSAGE.md)</b><br>
Yubico PIV Tool (command line) を使用して、鍵・証明書などを[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)に導入する手順を掲載します。

- <b>[Yubico PIV Toolによる各種手順](../CCID/YKPIVUSAGE_1.md)</b><br>
Yubico PIV Tool (command line) を使用した（前項以外の）各種手順を掲載します。

## 開発情報（ご参考）

- <b>[PIN番号を使用したmacOSログイン時の動作](../FIDO2Device/MDBT50Q_Dongle/PIVPINLOGIN_DEV.md)</b><br>
[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のPIV機能を使用して、macOSにPIN番号でログイン時、PC〜nRF52840間で行われるやり取りについて掲載しています。
