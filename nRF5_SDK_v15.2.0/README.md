# Diverta FIDO Authenticator

## 概要
FIDO U2Fの仕様に準拠したUSB HID／BLEサービスです。

- PCにおけるU2F Register／Authenticate実行時：ドングルをUSBポートに挿すと、nRF52840のUSB HIDサービスを経由して処理が行われます。

- AndroidにおけるU2F Authenticate実行時：ドングルに外部電源を供給すると、nRF52840のBLEサービスを経由して処理が行われます。

FIDO U2Fに関する情報 : https://fidoalliance.org/download/

### ハードウェア

nRF52840 Dongleでの動作を想定しています。

ただし、nRF52840 DK（開発用ボード）でも動作させることが可能です。<br>
この場合は、USB経由のUARTにより、デバッグログを表示させることができます。

### ファームウェア

「Diverta FIDO Authenticator」で使用するファームウェアは下記の２本になります。<br>
フォルダー `firmwares` に格納しています。

- s140_nrf52_6.1.0_softdevice.hex - ソフトデバイス

- nrf52840_xxaa.hex - アプリケーション

これらのファームウェアは、Nordic社から提供されているアプリ「nRF Connect」を使い、nRF52840 Dongleにダウンロードするようにします。
