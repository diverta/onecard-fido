# One Card FIDO対応

One CardにFIDO U2F／WebAuthn認証機能を実装するプロジェクトです。

## [開発TODO](TODO.md)
現在進行中、もしくは将来予定している開発案件についての概要を掲載しています。

## プログラム

- <b>[FIDO2アプリケーション](nRF5_SDK_v15.3.0)</b><br>
Nordic社のSoC「nRF52840」を使用した、FIDO U2F／WebAuthn認証器のファームウェアです。<br>
[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)と、nRF52840 DK（開発ボード）に対応しています。

- <b>[FIDO認証器管理ツール](MaintenanceTool)</b><br>
FIDO2認証器に、鍵・証明書・PINを導入するために使用する、デスクトップ・ツールです。<br>
[Windows版](MaintenanceTool/WindowsExe)、[macOS版](MaintenanceTool/macOSApp)の両方を用意しております。

- <b>[オープンソースコードライセンスについて](LICENSES.md)</b><br>
上記プログラム内で使用されているオープンソースコード（ライブラリー）についての概要を掲載しています。

## ハードウェア

- <b>[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)</b><br>
Nordic社のSoC「nRF52840」を使用した、FIDO U2F／WebAuthn認証器です。<br>
日本国内の技適取得済みであるnRF52840搭載モジュール「MDBT50Q」を使用し、nRF52840 Dongleとほぼ同じ仕様で製作しております。<br>
PCのUSBポート装着時はUSB HIDデバイスとして稼働し、ボタン乾電池装着時はBLEペリフェラルデバイスとして稼働します。

## What's new

#### 2020/11/04

FIDO認証器管理ツール、ファームウェアを修正しました。<br>
（ファームウェア[Version 0.2.12](https://github.com/diverta/onecard-fido/tree/improve-SDApp-Android-20201103/nRF5_SDK_v15.3.0/firmwares)は、管理ツールに同梱しています。<br>
　管理ツールの「[ファームウェア更新](https://github.com/diverta/onecard-fido/tree/improve-SDApp-Android-20201103/MaintenanceTool/macOSApp/UPDATEFIRMWARE.md)」機能を使用し、ファームウェアをアップデート願います）

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.33）](https://github.com/diverta/onecard-fido/blob/improve-SDApp-Android-20201103/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.33）](https://github.com/diverta/onecard-fido/blob/improve-SDApp-Android-20201103/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b>

修正点は以下になります。
- [BLEデバイス近接認証機能](https://github.com/diverta/onecard-fido/blob/improve-SDApp-Android-20201103/FIDO2Device/MDBT50Q_Dongle/BLEDAUTH.md)を、スマートフォンで利用できるようにする（[#371](https://github.com/diverta/onecard-fido/issues/371) ご参照）
- 開発時運用メニュー（ペアリング情報削除機能／ブートローダーモード遷移機能）を追加（[#381](https://github.com/diverta/onecard-fido/pull/381)、[#383](https://github.com/diverta/onecard-fido/pull/383) ご参照）

また、[BLEデバイス近接認証機能](https://github.com/diverta/onecard-fido/blob/improve-SDApp-Android-20201103/FIDO2Device/MDBT50Q_Dongle/BLEDAUTH.md)で使用することができる[Androidアプリ](https://github.com/diverta/onecard-fido/blob/improve-SDApp-Android-20201103/FIDO2Device/SecureDongleApp/Android/README.md)も合わせてリリースしています。<br>
（手順につきましては、[こちらの手順書](https://github.com/diverta/onecard-fido/blob/improve-SDApp-Android-20201103/FIDO2Device/SecureDongleApp/Android/BLEAUTH.md)をご参照）


#### [過去の更新履歴はこちら](HISTORY.md)

## FIDO2について

最新バージョンのプログラムにより使用可能となった新機能「BLEデバイス近接認証機能（パスワードレス・ボタンレス）」のイメージです。

<img src="FIDO2Device/MDBT50Q_Dongle/assets01/0000.jpg" width="720">
<img src="FIDO2Device/MDBT50Q_Dongle/assets01/0001.jpg" width="720">
<img src="FIDO2Device/MDBT50Q_Dongle/assets01/0002.jpg" width="720">

FIDO認証（WebAuthn）実行時、MDBT50Q Dongle上のボタンを押す代わりに、スマートフォンなどのBLEデバイスを近づけることにより、認証処理を自動的に続行させる機能です。

詳細につきましてはドキュメント<b>「[BLEデバイス近接認証機能](FIDO2Device/MDBT50Q_Dongle/BLEDAUTH.md)」</b>をご参照願います。

#### CTAP2とは

FIDOの新世代パスワードレス認証（<b>WebAuthn</b>）に対応するために用意された、FIDO 2.0の技術仕様です。

[FIDO2アプリケーション](nRF5_SDK_v15.3.0)では、既にUSB HIDトランスポート、BLEトランスポートに対応しています。<br>
NFCトランスポートは、後日対応予定です。

また、Windows環境（Edgeブラウザー）でのWebAuthnは、PINコード（暗証番号）入力が必須となるのですが、こちらの方もすでに対応済みとなっております。

Windows環境による具体的なテスト方法は、別途手順書[「Edgeブラウザーを使用したWebAuthnテスト手順」](FIDO2Device/MDBT50Q_Dongle/WEBAUTHNTEST.md)をご参照ください。

#### [以前の仕様](FORMER.md)
FIDOの旧世代２要素認証（U2F）に関する開発物件になります。
