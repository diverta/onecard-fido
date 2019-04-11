# Edgeブラウザーを使用したWebAuthnテスト手順

現在開発中の[FIDO2認証器](../../nRF5_SDK_v15.2.0)と、最新バージョンのEdgeブラウザーを使用し、WebAuthnユーザー登録／ログインをPINコードにより行う手順を掲載しています。

## ソフトウェアの導入

### 最新Edgeブラウザーの導入

WebAuthnをサポートしているEdgeブラウザーは、単体で導入できません。<br>
したがって、Windows 10のシステムを最新バージョンに移行させる必要があります。

具体的には、Windows PCに「Windows 10 October 2018 Update」を導入します。<br>
導入したら、Windows 10のバージョン情報を参照し、バージョンが「1809」になっていることを確認してください。

<img src="assets01/0001.png" width="700">

### ファームウェアの書込み

[FIDO2認証器](../../nRF5_SDK_v15.2.0)のファームウェアを、nRF52840 Dongleに書込みます。<br>
書込み手順につきましては、<b>[nRF52840 Dongleプログラミング手順](../../Development/nRF52840/NRFCONNECTINST.md)</b>をご参照ください。

ファームウェアは、GitHubリポジトリーの以下の場所に格納されています。
- ディレクトリー: onecard-fido/nRF5_SDK_v15.2.0/firmwares/
- アプリケーション: [nrf52840_xxaa.hex](../../nRF5_SDK_v15.2.0/firmwares/nrf52840_xxaa.hex)
- ソフトデバイス: [s140_nrf52_6.1.0_softdevice.hex](../../nRF5_SDK_v15.2.0/firmwares/s140_nrf52_6.1.0_softdevice.hex)

## 鍵・証明書の導入

nRF52840 Dongleにファームウェアを書き込んだら、[FIDO2認証器](../../nRF5_SDK_v15.2.0)に鍵・証明書を導入します。

#### 管理ツールを導入

まずは[FIDO認証器管理ツール](../../MaintenanceTool/README.md)を、PC環境（Windows 10）に導入します。<br>
以下の手順書をご参照願います。

* <b>[インストール手順（Windows版）](../../MaintenanceTool/WindowsExe/INSTALLPRG.md) </b><br>

#### 鍵・証明書を導入

PC環境に導入した管理ツールを使用し、鍵・証明書をインストールします。<br>
以下の手順書をご参照願います。

* <b>[鍵・証明書の導入手順（Windows版）](../../MaintenanceTool/WindowsExe/INSTALLKEYCRT.md) </b><br>

## PINコードの設定

今回はPINコード（暗証番号）として、`012345`という６けたの数字を使用するものとします。

### 管理ツールによるPINコード設定

Windows 10環境に導入した管理ツールを起動し、ファームウェアを書き込んだnRF52840 DongleをPCに装着後「PINコード設定」ボタンをクリックします。

<img src="assets01/0011.png" width="400">

「PINコード設定」画面がポップアップ表示されます。

「新しいPINコード」欄に、今回使用するPINコード（暗証番号）の`012345`を入力します。<br>
その下の「新しいPINコード(確認)」欄にも、同じく`012345`を入力します。<br>
その後「新規設定」ボタンをクリックします。

<img src="assets01/0012.png" width="400">

画面上に入力されたPINコードが、認証器に登録されます。<br>
程なく、下図のようなポップアップ画面が表示され、処理が完了します。

<img src="assets01/0013.png" width="400">

これで、PINコードの作成作業は完了です。

### PINコード変更

いったん設定されたPINコードを変更したい場合は、新規設定時と同様「PINコード設定」ボタンをクリックます。

<img src="assets01/0014.png" width="400">

「PINコード設定」画面がポップアップ表示されます。

「新しいPINコード」欄に、変更後のPINコード（暗証番号）を入力します。<br>
その下の「新しいPINコード(確認)」欄にも、同じく変更後のPINコードを入力します。<br>
最後に「変更前のPINコード」欄に、変更前のPINコードを入力します。<br>
その後「変更」ボタンをクリックします。

<img src="assets01/0015.png" width="400">

画面上に入力された変更後のPINコードが、認証器に登録されます。<br>
程なく、下図のようなポップアップ画面が表示され、処理が完了します。

<img src="assets01/0016.png" width="400">

## WebAuthn機能テストの実行

nRF52840 DongleとEdgeブラウザーを使用し、WebAuthn機能（ユーザー登録／ログイン）のテストを実施します。

### ユーザー登録

Edgeブラウザーを起動し、URL「[https://webauthn.org](https://webauthn.org)」を実行すると、WebAuthn機能のデモページが表示されます。

nRF52840 Dongleは、PCのUSBポートに装着しておきます。

ページ内の「Register」タブをクリックし「Create a New Account」画面を表示させます。<br>
テキストボックスに、任意のユーザー名を入力して、Registerボタンをクリックします。

<img src="assets01/0002.png" width="640">

程なく、Edge上にPINを入力する画面が表示されます。<br>
テキストボックスに、先ほど作成したPIN（暗証番号）`012345`を入力し、OKをクリックします。

<img src="assets01/0003.png" width="640">

下図のような、ユーザー所在確認を求める画面が表示されます。

<img src="assets01/0004.png" width="640">

nRF52840 Dongle上の緑色LEDが点滅し始めますので、基板上のボタンを１回プッシュします。

<img src="assets01/0005.png" width="400">

nRF52840 Dongleからレスポンスが戻り、しばらくすると、下図のようにユーザー登録処理が成功したことを通知する画面が表示されます。

<img src="assets01/0006.png" width="640">

これで、ユーザー登録のテストは完了です。

### ログイン

前述のWebAuthn機能のデモページを表示し、nRF52840 DongleをPCのUSBポートに装着しておきます。

ページ内の「Login」タブをクリックし「Log in Account Using WebAuthn」画面を表示させます。<br>
テキストボックスに、ユーザー登録時に入力したユーザー名を入力して、Loginボタンをクリックします。

<img src="assets01/0007.png" width="640">

程なく、Edge上にPINを入力する画面が表示されます。<br>
テキストボックスに、先ほど作成したPIN（暗証番号）`012345`を入力し、OKをクリックします。

<img src="assets01/0008.png" width="640">

下図のような、ユーザー所在確認を求める画面が表示されます。

<img src="assets01/0009.png" width="640">

nRF52840 Dongle上の緑色LEDが点滅し始めますので、基板上のボタンを１回プッシュします。

<img src="assets01/0005.png" width="400">

nRF52840 Dongleからレスポンスが戻り、しばらくすると、下図のようにログイン処理が成功したことを通知する画面が表示されます。

<img src="assets01/0010.png" width="640">

これで、ログインのテストは完了です。
