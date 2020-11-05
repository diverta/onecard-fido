# MDBT50Q Dongle（rev2.1.2）

## 概要

日本国内の技適取得済みであるnRF52840搭載モジュール「MDBT50Q」を使用し、nRF52840 Dongleとほぼ同じ仕様で製作した、USBドングル基板です。

<img src="../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/assets/0001.jpg" width="500">

#### 特色
- FIDO2機能（WebAuthn）をサポート
- FIDO2機能はUSB HID／BLEの両トランスポートをサポート
- USB給電／ボタン電池の２電源方式
- セキュアIC（ATECC608A）を搭載し、一度導入された秘密鍵／AESパスワードはいかなる方法によっても読出不可
- 署名検証機能付きUSBブートローダーを採用し、不正ファームウェアの書込みを抑止
- CCIDインターフェースを装備し、スマートカードエミュレーションが可能（ただし2020/08/31現在、業務処理プログラムは開発中）

### [MDBT50Q Dongleの概要](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/README.md)

基板、動作についての概要を説明しています。

### [MDBT50Q Dongle回路図](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/FIDO2AUTH_00212.pdf)

nRF52840 Dongleをベースとし、電池電源の増設と、LED／セキュアICの増設、若干の配線変更を行っております。

### [FIDO2認証器アプリケーション](../../nRF5_SDK_v15.3.0/README.md)

MDBT50Q Dongleで使用するファームウェアです。

### [ファームウェア更新手順](../../MaintenanceTool/macOSApp/UPDATEFIRMWARE.md)

MDBT50Q Dongleのファームウェアを、[FIDO認証器管理ツール（macOS版）](MaintenanceTool/macOSApp)により更新する手順について説明しています。

## 動作確認手順

### [Edgeブラウザーを使用したWebAuthnテスト手順（Windows10 PC）](WEBAUTHNTEST.md)

MDBT50Q Dongleと、Edgeブラウザー、デモサイトを使用して、WebAuthnのユーザー登録／ログイン（MakeCredential／GetAssertion）の動作確認をする場合の手順を掲載しています。

### [Googleアカウントのログイン確認手順（PC）](PCCHROME.md)

MDBT50Q Dongleと、Googleアカウント、Chromeブラウザーを使用して、U2F Register／Authenticateの動作確認をする場合の手順を掲載しています。

## オプション機能

BLEセントラルサービスを使用したオプション機能です。

### [BLE近接認証](BLEDAUTH.md)

FIDO認証（WebAuthn／U2F）実行時、MDBT50Q Dongle上のボタンを押す代わりに、スマートフォンなどのBLEデバイスを近づけることにより、認証処理を自動的に続行させます。

## ご参考情報

### [アプリケーション書込み手順](../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/APPINSTALL.md)

MDBT50Q Dongleにプレインストールされている[USBブートローダー（署名機能付き）](../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader/README.md)を経由し、MDBT50Q Dongleに[FIDO2認証器アプリケーション](../../nRF5_SDK_v15.3.0/README.md)を書き込む手順を掲載しています。<br>
（ファームウェア単体開発時の運用になります）

### [USBブートローダー（署名機能付き）](../../nRF5_SDK_v15.3.0/firmwares/secure_bootloader/README.md)

PCのUSBポート経由で、MDBT50Q Dongleに[FIDO2認証器アプリケーション](../../nRF5_SDK_v15.3.0/README.md)を書き込むことができるようにするためのファームウェアです。<br>
MDBT50Q Dongleにはすでに導入済みとなっております。
