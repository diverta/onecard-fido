# MDBT50Q Dongle Mini（rev1）

## 概要

日本国内の技適取得済みであるnRF52840搭載モジュール「MDBT50Q」を使用し、nRF52840 Dongleとほぼ同じ仕様で製作した、USBドングル基板です。

<img src="../../FIDO2Device/MDBT50Q_Dongle_mini/pcb_rev1/assets01/0003.jpg" width="500">

#### 特色
- 外形寸法＝3.9cm x 1.8cm
- FIDO2機能（WebAuthn）をサポート
- FIDO2機能はUSB HIDトランスポートをサポート
- USB給電方式（Type-Cプラグ）
- 署名検証機能付きUSBブートローダーを採用し、不正ファームウェアの書込みを抑止
- CCIDインターフェースを装備し、PIV／OpenPGPカードエミュレーションが可能
- BLEセントラルデバイスとしても動作可能

### [MDBT50Q Dongle Miniの概要](../../FIDO2Device/MDBT50Q_Dongle_mini/pcb_rev1/README.md)

基板、動作についての概要を説明しています。

### [MDBT50Q Dongle Mini回路図](../../FIDO2Device/MDBT50Q_Dongle_mini/pcb_rev1/SECDONGL_001.pdf)

nRF52840 Dongleをベースとし、LEDの増設、若干の配線変更を行っております。

### [nRF52840アプリケーション](../../nRF52840_app/README.md)

MDBT50Q Dongle Miniで使用するファームウェアです。
