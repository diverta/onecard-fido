# One Card FIDO対応

One CardにFIDO U2F／WebAuthn認証機能を実装するプロジェクトです。

## プログラム

- <b>[FIDO2アプリケーション](nRF5_SDK_v15.3.0)</b><br>
Nordic社のSoC「nRF52840」を使用した、FIDO U2F／WebAuthn認証器のファームウェアです。<br>
[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)と、nRF52840 DK（開発ボード）に対応しています。

- <b>[FIDO認証器管理ツール](MaintenanceTool)</b><br>
FIDO2認証器に、鍵・証明書・PINを導入するために使用する、デスクトップ・ツールです。<br>
[Windows版](MaintenanceTool/WindowsExe)、[macOS版](MaintenanceTool/macOSApp)の両方を用意しております。

## ハードウェア

- <b>[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)</b><br>
Nordic社のSoC「nRF52840」を使用した、FIDO U2F／WebAuthn認証器です。<br>
日本国内の技適取得済みであるnRF52840搭載モジュール「MDBT50Q」を使用し、nRF52840 Dongleとほぼ同じ仕様で製作しております。<br>
PCのUSBポート装着時はUSB HIDデバイスとして稼働し、ボタン乾電池装着時はBLEペリフェラルデバイスとして稼働します。

## What's new

#### 2020/04/15

[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェア導入・更新が、[FIDO認証器管理ツール(Windows版)](MaintenanceTool/WindowsExe)により実行できるようになりました（[#300](https://github.com/diverta/onecard-fido/pull/300) ご参照）。<br>
下記のバージョンをご使用願います。

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.27）](https://github.com/diverta/onecard-fido/blob/research-FIDO2MT-Windows-update-firmware-01/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b><br>
MDBT50Q_Dongleの最新ファームウェア（Version 0.2.8）は、Windows版 FIDO認証器管理ツールに同梱されております。

#### 2020/03/31

[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェア導入・更新が、[FIDO認証器管理ツール(macOS版)](MaintenanceTool/macOSApp)により実行できるようになりました（[#319](https://github.com/diverta/onecard-fido/pull/319) ご参照）。<br>
下記のバージョンをご使用願います。

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.27）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-macOS-cmd-BLmode/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b><br>
MDBT50Q_Dongleの最新ファームウェア（Version 0.2.8）は、macOS版 FIDO認証器管理ツールに同梱されております。

#### ご注意

管理ツールを使用して、MDBT50Q Dongleにファームウェア導入・更新を実行するためには、MDBT50Q Dongleに、新規制作した[「署名機能付きUSBブートローダー」](https://github.com/diverta/onecard-fido/tree/research-FIDO2MT-Windows-update-firmware-01/nRF5_SDK_v15.3.0/firmwares/secure_bootloader)を導入する必要がございます。<br>
手順につきましては<b>[「署名機能付きUSBブートローダー移行手順書」](https://github.com/diverta/onecard-fido/blob/research-FIDO2MT-Windows-update-firmware-01/nRF5_SDK_v15.3.0/firmwares/secure_bootloader/MIGRATION.md)</b>をご参照願います。


#### 2020/03/30

[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェアを修正しました。<br>

- <b>[nRF52840ファームウェア（Version 0.2.8）](https://github.com/diverta/onecard-fido/blob/improve-nRF52840-jumping-to-BLmode/nRF5_SDK_v15.3.0/firmwares)</b>

修正点は以下になります。
- ブートローダーモード遷移コマンドを追加実装<br>（[#318](https://github.com/diverta/onecard-fido/pull/318) ご参照）<br>
MDBT50Q Dongle基板上の物理的な操作無しで、MDBT50Q Dongleをブートローダーモードに遷移させるためのUSB HIDコマンド「ブートローダーモード遷移コマンド」を新設しています。<br>
「ブートローダーモード遷移コマンド」は、別途新規制作した「[署名機能付きUSBブートローダー](https://github.com/diverta/onecard-fido/tree/improve-nRF52840-jumping-to-BLmode/nRF5_SDK_v15.3.0/firmwares/secure_bootloader)」を導入したMDBT50Q Dongleで、実行可能です。<br>
その他の業務処理につきましては、前回（[Version 0.2.7](https://github.com/diverta/onecard-fido/tree/bug-nRF52840-BLE-auth-scanparam/nRF5_SDK_v15.3.0/firmwares)）から変更は一切ありません。

#### [過去の更新履歴はこちら](HISTORY.md)

## FIDO2について

#### イメージ図

<img src="Documents/summary/sys_summary.png" width="600">

<img src="Documents/summary/uc_summary.png" width="600">

#### CTAP2とは

FIDOの新世代パスワードレス認証（<b>WebAuthn</b>）に対応するために用意された、FIDO 2.0の技術仕様です。

[FIDO2アプリケーション](nRF5_SDK_v15.3.0)では、既にUSB HIDトランスポート、BLEトランスポートに対応しています。<br>
NFCトランスポートは、後日対応予定です。

また、Windows環境（Edgeブラウザー）でのWebAuthnは、PINコード（暗証番号）入力が必須となるのですが、こちらの方もすでに対応済みとなっております。

Windows環境による具体的なテスト方法は、別途手順書[「Edgeブラウザーを使用したWebAuthnテスト手順」](FIDO2Device/MDBT50Q_Dongle/WEBAUTHNTEST.md)をご参照ください。

## 以前の仕様

以下の項目は、FIDOの旧世代２要素認証（U2F）に関する開発物件になります。<br>
[FIDO2アプリケーション](nRF5_SDK_v15.3.0)においては、U2FはCTAP2と同居していますので、現在も稼働させることができます。

ただし、U2FはChromeブラウザーのみのサポートであり、かつ将来的にサポートが拡張される予定もないため、現在はメンテナンスをストップさせております。<br>
何卒ご容赦ください。

#### BLE U2Fサービス

One CardのBLE上で稼働するFIDO U2F機能です。<br>
U2F v1.2の仕様に準拠しております。

FIDO U2Fに関する情報 : https://fidoalliance.org/download/

FIDO U2F対応デバイスは、USBポートに挿して使用する「YubiKey」が有名ですが、BLE U2Fサービスは、USBポートではなく、One CardのBLEを使用しています。

コード格納場所--->[nRF5_SDK_v13.0.0](nRF5_SDK_v13.0.0)

#### U2F管理ツール

PC環境から、BLE U2Fサービスの動作に必要な鍵・証明書の導入などを行うツールです。<br>
macOS版と、Windows版を用意しました。

コード格納場所--->[U2FMaintenanceTool](U2FMaintenanceTool)

## 新しい試み

- <b>[mbedOS版 FIDO2アプリケーション](STM32F411RE)</b>（対応無期限延期中）<br>
ST社のマイコン「STM32F411RE」を使用した、FIDO U2F／WebAuthn認証器です。<br>
mbed OSへのアプリケーション移植のほか、NFCタグIC（自己発電機能あり）、セキュアICを導入する新しい試みになります。
