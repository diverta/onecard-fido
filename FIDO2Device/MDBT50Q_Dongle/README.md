# MDBT50Q Dongle（rev2.2）

## 概要

日本国内の技適取得済みであるnRF52840搭載モジュール「MDBT50Q」を使用し、nRF52840 Dongleとほぼ同じ仕様で製作した、USBドングル基板です。

<img src="../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_2/assets/0001.jpg" width="500">

#### 特色
- 外形寸法＝5cm x 2cm
- FIDO2機能（WebAuthn）をサポート
- FIDO2機能はUSB HID／BLEの両トランスポートをサポート
- USB給電／ボタン電池の２電源方式
- リアルタイムクロック・カレンダーを搭載し、電池装着時は時刻同期が可能
- 署名検証機能付きUSBブートローダーを採用し、不正ファームウェアの書込みを抑止
- CCIDインターフェースを装備し、PIV/OpenPGPカードエミュレーション、OATH機能実行が可能

### [MDBT50Q Dongleの概要](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_2/README.md)

基板、動作についての概要を説明しています。

### [MDBT50Q Dongle回路図](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_2/FIDO2AUTH_0022.pdf)

nRF52840 Dongleをベースとし、電池電源の増設と、LED／RTCC（リアルタイムクロック・カレンダー）の増設、若干の配線変更を行っております。

### [nRF52840アプリケーション](../../nRF52840_app/README.md)

MDBT50Q Dongleで使用するファームウェアです。

### [ファームウェア更新手順](../../MaintenanceTool/macOSApp/UPDATEFIRMWARE.md)

MDBT50Q Dongleのファームウェアを、[FIDO認証器管理ツール](../../MaintenanceTool/README.md)により更新する手順について説明しています。

## 動作確認手順

### [Edgeブラウザーを使用したWebAuthnテスト手順（Windows10 PC）](WEBAUTHNTEST.md)

MDBT50Q Dongleと、Edgeブラウザー、デモサイトを使用して、WebAuthnのユーザー登録／ログイン（MakeCredential／GetAssertion）の動作確認をする場合の手順を掲載しています。

### [Googleアカウントのログイン確認手順（PC）](PCCHROME.md)

MDBT50Q Dongleと、Googleアカウント、Chromeブラウザーを使用して、U2F Register／Authenticateの動作確認をする場合の手順を掲載しています。

### [PIN番号を使用したmacOSログイン確認手順](../../CCID/PIV/PIVPINLOGIN.md)

MDBT50Q Dongleに設定したPIV機能を使用し、PIN番号によるmacOSログインを行うための確認手順を掲載しています。
