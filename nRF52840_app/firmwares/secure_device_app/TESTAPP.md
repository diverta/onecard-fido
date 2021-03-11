# nRF52840アプリケーション動作確認手順書

## 概要

[管理ツール](../../../MaintenanceTool/README.md)等を使用し、[MDBT50Q Dongle](../../../FIDO2Device/MDBT50Q_Dongle/README.md)に導入した[nRF52840アプリケーション](../../../nRF52840_app/firmwares/secure_device_app)の動作確認を行う手順について、以下に掲載いたします。

## 動作確認手順

下記の各手順書をご参照願います。

### [FIDO2機能の動作確認（HID）](../../../nRF52840_app/firmwares/secure_device_app/TESTAPPHID.md)

FIDO2機能の動作確認（HID）は、ヘルスチェック機能の実行により行います。<br>
管理ツールの`Test --> USB`メニューで実行できます。

### [FIDO2機能の動作確認（BLE）](../../../nRF52840_app/firmwares/secure_device_app/TESTAPPBLE.md)

FIDO2機能の動作確認（BLE）は、ヘルスチェック機能の実行により行います。<br>
管理ツールの`Test --> BLE`メニューで実行できます。

### [FIDO2機能の動作確認（BLE近接認証）](../../../nRF52840_app/firmwares/secure_device_app/TESTAPPBLEAUTH.md)

[BLE近接認証機能](../../../FIDO2Device/SecureDongleApp/Android/BLEAUTH.md)を使用し、FIDO2機能の動作確認を行います。

### [PIV機能の動作確認（macOS限定）](../../../nRF52840_app/firmwares/secure_device_app/TESTAPPCCID.md)

PIV機能の動作確認は、PIV機能設定の実行により行います。<br>
管理ツールの`Option --> PIV機能設定`メニューで実行できます。

### [OpenPGP機能の動作確認（macOS限定）](../../../nRF52840_app/firmwares/secure_device_app/TESTAPPOPGP.md)

OpenPGP機能の動作確認は、管理ツールではなく、macOSに導入した[GPGツール](https://gpgtools.org)の実行により行います。
