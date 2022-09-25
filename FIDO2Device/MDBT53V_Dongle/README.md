# [開発中] MDBT53V Dongle（rev1）

## 概要

日本国内の技適取得済みであるnRF5340搭載モジュール「MDBT53V」を使用して製作する、USBドングル基板です。<br>
（下図は[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle)。ただし、MDBT53V Dongleもほぼ同イメージになる想定）

<img src="../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/assets/0001.jpg" width="500">

### 特色
- 外形寸法＝5cm x 2cm
- FIDO2機能（WebAuthn）をサポート
- FIDO2機能はUSB HID／BLEの両トランスポートをサポート
- 署名検証機能付きBLEブートローダーを採用し、不正ファームウェアの書込みを抑止
- PIVカード／OpenPGPカードのエミュレーション機能を搭載
- USB給電／ボタン電池の２電源方式
- リアルタイムクロックモジュールを搭載

### [nRF5340アプリケーション](../../nRF5340_app/README.md)

MDBT53V Dongleで使用するファームウェアです。

### [ファームウェア更新手順](../../MaintenanceTool/macOSApp/UPDATEFW_BLE.md)

MDBT53V Dongleのファームウェアを、[FIDO認証器管理ツール（macOS版）](../../MaintenanceTool/macOSApp)により更新する手順について説明しています。

## 動作確認手順

### [Edgeブラウザーを使用したWebAuthnテスト手順（Windows10 PC）](WEBAUTHNTEST.md)

MDBT53V Dongleと、Edgeブラウザー、デモサイトを使用して、WebAuthnのユーザー登録／ログイン（MakeCredential／GetAssertion）の動作確認をする場合の手順を掲載しています。

### [Googleアカウントのログイン確認手順（PC）](PCCHROME.md)

MDBT53V Dongleと、Googleアカウント、Chromeブラウザーを使用して、U2F Register／Authenticateの動作確認をする場合の手順を掲載しています。

### [PIN番号を使用したmacOSログイン確認手順](PIVPINLOGIN.md)

MDBT53V Dongleに設定したPIV機能を使用し、PIN番号によるmacOSログインを行うための確認手順を掲載しています。
