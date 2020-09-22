# CCIDインターフェース追加対応

## 概要

セキュリティデバイス用のインターフェースとして使用されている、USB CCIDインターフェースを追加する対応になります。

## 目的

PIV Card、OpenPGP Cardなどといったスマートカードのエミュレーションを、nRF52840アプリケーション上で実行させるために必要となります。

#### 当面の目標

nRF52840アプリケーションを搭載した[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)を使用し、PIN番号でmacOSにログインできるようになるところを目指します。<br>
（ご参考：<b>[PIVデバイスを使用したmacOSログイン手順](../Research/CCID/MACPIVLOGIN.md)</b>）

## 手順書

- <b>[CCIDドライバーインストール手順](../CCID/INSTALLPRG.md)</b><br>
[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のCCIDインターフェース専用ドライバー（以下単に「CCIDドライバー」）を、macOS環境にインストールする手順を掲載しています。<br>
WIndows 10環境では、CCIDドライバーのインストールは不要になります。

- <b>[Yubico PIV Tool (command line) macOS版 導入手順](../CCID/PIVTOOLMACINST.md)</b><br>
PIVで使用する証明書等を導入するために利用できる「Yubico PIV Tool (command line) 」を、macOS環境に導入する手順を掲載します。

- <b>[Yubico PIV Tool (command line) Windows版 導入手順](../CCID/PIVTOOLWININST.md)</b><br>
PIVで使用する証明書等を導入するために利用できる「Yubico PIV Tool (command line) 」を、Windows 10環境に導入する手順を掲載します。
