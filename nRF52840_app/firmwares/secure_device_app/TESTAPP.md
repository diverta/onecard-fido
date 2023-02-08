# nRF52840アプリケーション動作確認手順書

最終更新日：2023/2/8

## 概要

[管理ツール](../../../MaintenanceTool/README.md)等を使用し、[MDBT50Q Dongle](../../../FIDO2Device/MDBT50Q_Dongle/README.md)に導入した[nRF52840アプリケーション](../../../nRF52840_app/firmwares/secure_device_app/README.md)の動作確認を行う手順について、以下に掲載いたします。

## 動作確認手順

下記の各手順書をご参照願います。

### [FIDO2機能の動作確認（HID）](../../../nRF52840_app/firmwares/secure_device_app/TESTAPPHID.md)

FIDO2機能の動作確認（HID）は、ヘルスチェック機能の実行により行います。<br>
管理ツールの「ヘルスチェック実行（USB）」メニューで実行できます。

### [FIDO2機能の動作確認（BLE）](../../../nRF52840_app/firmwares/secure_device_app/TESTAPPBLE.md)

FIDO2機能の動作確認（BLE）は、ヘルスチェック機能の実行により行います。<br>
管理ツールの「ヘルスチェック実行（BLE）」メニューで実行できます。

### PIV機能の動作確認（macOS限定）

PIV機能の動作確認は、認証器にPIV機能の設定を行った後、macOSにPIN番号でログインすることにより行います。<br>
手順につきましては、別ドキュメント「[OpenPGPカードエミュレーション対応](../../../CCID/PIV/README.md)」の各手順書をご参照ください。<br>

### OpenPGP機能の動作確認

OpenPGP機能の動作確認は、認証器にOpenPGP機能の設定を行った後、任意のファイルをPGP鍵で暗号／復号化することにより行います。<br>
手順につきましては、別ドキュメント「[OpenPGPカードエミュレーション対応](../../../CCID/OpenPGP/README.md)」の各手順書をご参照ください。<br>
