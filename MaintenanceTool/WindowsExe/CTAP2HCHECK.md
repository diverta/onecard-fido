# CTAP2ヘルスチェック実行手順

## 概要

[FIDO認証器管理ツール](README.md)を使用して、[nRF52840版FIDO認証器](https://github.com/diverta/onecard-fido/tree/master/nRF5_SDK_v15.2.0)のヘルスチェックを実行する手順を掲載します。

#### 処理内容

「CTAP2ヘルスチェック実行」は、管理ツールが擬似WebAuthnクライアント（Webブラウザーの代わり）となり、認証器に対して、FIDO 2.0の仕様に準拠したユーザー登録・ログインの各処理を実行する機能です。

CTAP2ヘルスチェックは、USB HID経由で実行されます。

## 認証器の準備

#### 使用できる機材

Nordic社から販売されている開発基板「nRF52840 DK」または「nRF52840 Dongle」が、[nRF52840版FIDO認証器](https://github.com/diverta/onecard-fido/tree/master/nRF5_SDK_v15.2.0)として使用できます。

- nRF52840 DK<br>
https://www.mouser.jp/new/nordicsemiconductor/nordic-nrf52840-dev-kit/

- nRF52840 Dongle<br>
https://www.mouser.jp/new/nordicsemiconductor/nordic-nrf52840-usb-dongle/

本文では、nRF52840 Dongleを使用するものとしております。

#### ファームウェアの書込み

[FIDO2認証器](../../nRF5_SDK_v15.2.0)のファームウェアを、nRF52840 Dongleに書込みます。<br>
書込み手順につきましては、<b>[nRF52840 Dongleプログラミング手順](../../Development/nRF52840/NRFCONNECTINST.md)</b>をご参照ください。

ファームウェアは、GitHubリポジトリーの以下の場所に格納されています。
- ディレクトリー: onecard-fido/nRF5_SDK_v15.2.0/firmwares/
- アプリケーション: [nrf52840_xxaa.hex](../../nRF5_SDK_v15.2.0/firmwares/nrf52840_xxaa.hex)
- ソフトデバイス: [s140_nrf52_6.1.0_softdevice.hex](../../nRF5_SDK_v15.2.0/firmwares/s140_nrf52_6.1.0_softdevice.hex)

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

nRF52840 DongleをPCのUSBポートに装着後、管理ツールを起動します。<br>
「USB HIDデバイスに接続されました。」というメッセージが表示されていることを確認します。

<img src="assets/0021.png" width="400">

メニューから「テスト(T)」-->「USB」-->「CTAP2ヘルスチェック実行」を選択します。

<img src="assets/0022.png" width="400">

PIN入力画面がポップアップ表示されますので、認証器に設定したPINコード（暗証番号）を入力し「OK」をクリックします。

<img src="assets/0023.png" width="400">

ヘルスチェック処理が進み、ほどなく下図のようなメッセージが表示され、ユーザー所在確認が要求されます。

<img src="assets/0024.png" width="400">

nRF52840 Dongle上の緑色LEDが点滅し始めますので、基板上のボタンを１回プッシュします。

<img src="assets/0026.png" width="400">

ヘルスチェック処理が成功すると「CTAP2ヘルスチェックが成功しました。」というメッセージが表示されます。

<img src="assets/0025.png" width="400">

これで、CTAP2ヘルスチェックの実行は完了です。
