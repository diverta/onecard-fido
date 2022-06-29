# OpenPGPカードエミュレーション対応

## 概要

[nRF5340アプリケーション](../../nRF5340_app)に、OpenPGPカードと同等の機能（OpenPGPカードエミュレーション機能）を追加する対応です。

[OpenPGPカードエミュレーション機能](../../CCID/openpgp_lib/README.md)は、[USB CCIDインターフェース](../../CCID/ccid_lib/README.md)上で動作します。

## 利用例

<img src="assets01/0017.jpg" width="720"><br>
<img src="assets01/0018.jpg" width="720">


## 手順書（macOS環境）

- <b>[PGP鍵を使用したファイル暗号／復号化手順](../../CCID/OpenPGP/OPGPCRYPTION.md)</b><br>
[BT40 Dongle](../../FIDO2Device/BT40Dongle/README.md)にインストールしたPGP鍵を使用し、macOS上でファイルを暗号化／復号化をする手順について掲載しています。

- [CCIDドライバーインストール手順](../../CCID/INSTALLPRG.md)<br>
[BT40 Dongle](../../FIDO2Device/BT40Dongle/README.md)のCCIDインターフェース専用ドライバー（以下単に「CCIDドライバー」）を、macOS環境にインストールする手順について掲載しています。

- [GPG Suiteインストール手順](../../CCID/OpenPGP/GPGINSTMAC.md)<br>
GPGツール群「[GPG Suite](https://gpgtools.org)」を、macOS環境にインストールする手順について掲載しています。<br>
GPG Suiteは、macOS環境へのPGP鍵インストール時に必要となりますので、事前にインストール願います。

- [PGP鍵インストール手順書](../../MaintenanceTool/macOSApp/PGPKEYINST.md)<br>
管理ツールを使用し、[BT40 Dongle](../../FIDO2Device/BT40Dongle/README.md)に、PGP鍵をインストールする手順について掲載しています。

## 手順書（Windows環境）

- <b>[PGP鍵を使用したファイル暗号／復号化手順](../../CCID/OpenPGP/OPGPCRYPTIONWIN.md)</b><br>
[BT40 Dongle](../../FIDO2Device/BT40Dongle/README.md)にインストールしたPGP鍵を使用し、Windows上でファイルを暗号化／復号化をする手順について掲載しています。

- [Gpg4winインストール手順](../../CCID/OpenPGP/GPGINSTWIN.md)<br>
GPGツール群「[Gpg4win](https://www.gnupg.org)」を、Windows環境にインストールする手順について掲載しています。<br>
Gpg4winは、Windows環境へのPGP鍵インストール時に必要となりますので、事前にインストール願います。

- [PGP鍵インストール手順書](../../MaintenanceTool/WindowsExe/PGPKEYINST.md)<br>
管理ツールを使用し、[BT40 Dongle](../../FIDO2Device/BT40Dongle/README.md)に、PGP鍵をインストールする手順について掲載しています。

Windows 10環境では、CCIDドライバーのインストールは不要になります。

## ご参考

- [GPG Suiteによる鍵インストール手順](../../CCID/OpenPGP/GPGKEYINST.md)<br>
PGP鍵のインストールを、管理ツールを使用せず、GPG Suiteだけを使用して行う場合の手順について掲載しています。
