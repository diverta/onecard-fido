# CTAP2ヘルスチェック実行手順

## 概要

[FIDO認証器管理ツール](README.md)を使用して、[FIDO2認証器](../../nRF5_SDK_v15.3.0)のヘルスチェックを実行する手順を掲載します。

#### 処理内容

「CTAP2ヘルスチェック実行」は、管理ツールが擬似WebAuthnクライアント（Webブラウザーの代わり）となり、認証器に対して、FIDO 2.0の仕様に準拠したユーザー登録・ログインの各処理を実行する機能です。

CTAP2ヘルスチェックは、USB HID経由またはBLE経由で実行されます。

## 認証器の準備

#### 使用機材

本ドキュメントでは「[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)」を、[FIDO2認証器](../../nRF5_SDK_v15.3.0)として使用します。

<img src="../../FIDO2Device/MDBT50Q_Dongle/assets/0000.jpg" width="300">

#### ファームウェアの書込み

[FIDO2認証器](../../nRF5_SDK_v15.3.0)のファームウェアを、MDBT50Q Dongleに書込みます。<br>
書込み手順につきましては、<b>[アプリケーション書込み手順](../../FIDO2Device/MDBT50Q_Dongle/APPINSTALL.md)</b>をご参照ください。

#### 管理ツールを導入

[FIDO認証器管理ツール](../../MaintenanceTool/README.md)を、PC環境（Windows 10）に導入します。<br>
以下の手順書をご参照願います。

* <b>[インストール手順（Windows版）](../../MaintenanceTool/WindowsExe/INSTALLPRG.md) </b>

#### 鍵・証明書導入／PIN設定

PC環境に導入した管理ツールを使用し、鍵・証明書のインストール、およびPINコード（暗証番号）の設定をします。<br>
以下の手順書をご参照願います。

* <b>[鍵・証明書の導入手順（Windows版）](../../MaintenanceTool/WindowsExe/INSTALLKEYCRT.md) </b>

* <b>[PINコードの設定手順（Windows版）](../../MaintenanceTool/WindowsExe/SETPIN.md) </b>

## CTAP2ヘルスチェックの実行

MDBT50Q DongleをPCのUSBポートに装着後、管理ツールを起動します。<br>
「USB HIDデバイスに接続されました。」というメッセージが表示されていることを確認します。

<img src="assets/0021.png" width="400">

メニューから「テスト(T)」-->「USB」-->「CTAP2ヘルスチェック実行」を選択します。

<img src="assets/0022.png" width="400">

PIN入力画面がポップアップ表示されますので、認証器に設定したPINコード（暗証番号）を入力し「OK」をクリックします。

<img src="assets/0023.png" width="400">

ヘルスチェック処理が進み、ほどなく下図のようなメッセージが表示され、ユーザー所在確認が要求されます。

<img src="assets/0024.png" width="400">

MDBT50Q Dongle上の緑色LEDが点滅し始めますので、基板上のボタンを１回プッシュします。

<img src="../../FIDO2Device/MDBT50Q_Dongle/assets/0014.jpg" width="400">

ヘルスチェック処理が成功すると「CTAP2ヘルスチェックが成功しました。」というメッセージが表示されます。

<img src="assets/0025.png" width="400">

これで、CTAP2ヘルスチェックの実行は完了です。
