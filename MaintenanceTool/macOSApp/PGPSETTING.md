# OpenPGP機能の基本設定手順

最終更新日：2022/2/24

## 概要

[FIDO認証器管理ツール](README.md)を使用して、[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)に対し、OpenPGP機能に最低限必要な基本設定を行う手順を掲載します。

## ソフトウェアのバージョン確認／インストール

OpenPGP機能は、[CCIDインターフェース](../../CCID/README.md)という仕組みを使用しております。<br>
この仕組みを使用するためには、管理ツール、ファームウェア共に、必要バージョン以降である必要があります。<br>
また、[GPG Suite](https://gpgtools.org)というツールを、PCに別途インストールする必要があります。

#### 管理ツールのバージョン確認
まずは[インストール手順](INSTALLPRG.md)を参照し、管理ツールをmacOSにインストールします。<br>
次に、下記手順で管理ツールのバージョン確認を行い、<b>Version 0.1.39以降</b>であるかどうか確認します。

管理ツールのメニュー「Preferences」を選択し、ツール設定画面を開きます。

<img src="assets03/0001.jpg" width="360">

ツール設定画面のタブ「バージョン」を選択し、バージョンを確認してください。<br>
（下記例では「Version 0.1.39」となっております）

<img src="assets07/0001.jpg" width="360">

#### ファームウェアのバージョン確認
続いて、下記手順でファームウェアのバージョン確認を行い、<b>0.3.4以降</b>であるかどうか確認します。

[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)をPCのUSBポートに装着した後、管理ツールのメニュー「Test-->USB-->バージョン情報取得」を選択します。

<img src="assets05/0031.jpg" width="360">

管理ツール下部のメッセージ欄に表示される、ファームウェアのバージョンを確認してください。<br>
（下記例では「0.3.5」となっております）

<img src="assets07/0002.jpg" width="360">

#### GPG Suiteのインストール

管理ツールのOpenPGP機能設定においては、[GPG Suite](https://gpgtools.org)というツールに同梱の「MacGPG2」を管理ツールで内部利用しているため、あらかじめ<b>GPG SuiteがPCにインストールされている</b>必要があります。

<img src="../../CCID/OpenPGP/assets01/0001.jpg" width="400">

GPG Suiteのインストール手順につきましては、別ドキュメント「<b>[GPG Suiteインストール手順書](../../CCID/OpenPGP/GPGINSTMAC.md)</b>」をご参照願います。

## OpenPGP機能設定画面の表示

OpenPGP機能の設定は「OpenPGP機能設定画面」上で行います。

まずは管理ツールを起動し、USBポートに[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)を装着します。<br>

<img src="assets/0028.jpg" width="360">

管理ツール画面下部のメッセージ欄に「USB HIDデバイスに接続されました。」と表示されることを確認したら、管理ツール画面の「OpenPGP機能設定」ボタンをクリックします。

<img src="assets07/0003.jpg" width="360">

ホーム画面の上に、OpenPGP機能設定画面がポップアップ表示されます。

<img src="assets07/0004.jpg" width="360">

以後の設定作業は、すべてこの「OpenPGP機能設定画面」で実行します。

## 基本設定の実行

OpenPGP機能に最低限必要な基本設定、すなわちPGP鍵のインストールを実行します。

### PGP秘密鍵のインストール

OpenPGP機能では、PGP秘密鍵を[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)に導入する必要があります。

導入が必要な秘密鍵は、以下の３セットになります。<br>
いずれも、本機能で自動生成されます。

- 電子署名用（Signature key）
- 暗号／復号化用（Encryption key）
- PGP認証用（Authentication key）

以下の手順により、３セットの秘密鍵をすべてインストールします。

#### インストール手順

まずはインストールする鍵の個人情報を入力します。<br>
全項目入力必須になります。

- 名前（５文字以上で入力します）
- メールアドレス
- コメント

<img src="assets07/0005.jpg" width="350">

次に、各種ファイルの出力先フォルダーを選択します。<br>
「PGP公開鍵」欄右側の「参照」ボタンをクリックします。

<img src="assets07/0006.jpg" width="350">

フォルダー参照ダイアログから、該当の出力先フォルダーを選択し「選択」ボタンをクリックします。

<img src="assets07/0007.jpg" width="350">

フォルダー欄に、選択された出力先フォルダーのパスが表示されます。<br>
（長いフォルダー名の場合は、マウスカーソルを上から当てると、フルパス名称が小さくToolTip表示されます）

<img src="assets07/0008.jpg" width="350">

同様に、バックアップファイルの出力先フォルダーも選択します。

<img src="assets07/0009.jpg" width="350">

PGP公開鍵、バックアップ両方の出力先フォルダーを選択したら、下部の認証情報欄に、OpenPGP機能で使用する管理用PIN番号を入力します。[注1]

PIN番号を入力したら、下部の「PGP秘密鍵のインストール」ボタンをクリックします。

<img src="assets07/0010.jpg" width="350">

下記のような確認ダイアログが表示されますので、Yesボタンをクリックします。

<img src="assets07/0011.jpg" width="350">

PGP秘密鍵のインストール処理が実行されます。

<img src="assets07/0012.jpg" width="350">

程なく、下図のようなメッセージがポップアップ表示され、処理が完了します。

<img src="assets07/0013.jpg" width="350">

[注1] 管理用PIN番号は初期状態では「`12345678`」となっております。変更したい場合は、別ドキュメント「[OpenPGP機能の各種設定手順](../../MaintenanceTool/macOSApp/PGPSETTING_OPT.md)」をご参照願います。

### 確認手順

インストールされた証明書は「OpenPGP設定情報取得画面」で確認できます。<br>
OpenPGP機能設定画面の左下部のボタン「設定情報を参照」をクリックします。

<img src="assets07/0014.jpg" width="350">

下図のようなOpenPGP設定情報取得画面がポップアップ表示されます。<br>
以下の３点が設定されていることが確認できます。

- 電子署名用（Signature key）の設定情報
- 暗号／復号化用（Encryption key）の設定情報
- PGP認証用（Authentication key）の設定情報

<img src="assets07/0015.jpg" width="350">

以上で、PGP秘密鍵のインストールは完了です。

## OpenPGP機能設定情報の消去

万が一、管理用PIN番号を３回連続で間違えて指定した場合は、認証がブロックされます。<br>
他方、管理用PIN番号のリセット機能は存在しません。

したがって、[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)において、再び管理用PIN番号が利用できるようにするためには、いったんOpenPGP機能設定情報を全て消去する必要があります。<br>
この場合、OpenPGP機能に関連する以下の設定情報が全て消去されてしまいますので、ご注意ください。

- 電子署名用（Signature key）の設定情報
- 暗号／復号化用（Encryption key）の設定情報
- PGP認証用（Authentication key）の設定情報

OpenPGP機能設定情報の消去を実行するためには、OpenPGP機能設定画面の右下部「設定情報を消去」ボタンをクリックします。

<img src="assets07/0016.jpg" width="350">

下記のような確認ダイアログが表示されますので、Yesボタンをクリックします。

<img src="assets07/0017.jpg" width="350">

設定情報消去処理が実行されます。

<img src="assets07/0018.jpg" width="350">

程なく、下図のようなメッセージがポップアップ表示され、処理が完了します。

<img src="assets07/0019.jpg" width="350">

OpenPGP設定情報取得画面で確認すると、下記３点の設定情報が消去されたことを示しています。

- 電子署名用（Signature key）
- 暗号／復号化用（Encryption key）
- PGP認証用（Authentication key）

<img src="assets07/0020.jpg" width="350">

これでOpenPGP機能設定の消去は完了です。

## 認証器のリセット

前述の各機能を実行時、下図のようなエラーメッセージが表示されることがあります。

<img src="assets07/0021.jpg" width="350">

この場合は、認証器のリセットをお試しいただくことにより、認証器の再装着が不要となる場合があります。<br>
画面下部のボタン「認証器のリセット」をクリックします。

<img src="assets07/0022.jpg" width="350">

認証器がUSBに装着されたまま、ファームウェアが再始動されます。

<img src="assets07/0018.jpg" width="350">

程なく、下図のようなメッセージがポップアップ表示され、処理が完了します。

<img src="assets07/0023.jpg" width="350">

万が一、上記手順でも解消しない場合は、一旦MDBT50Q DongleをPCのUSBポートから取り出し、再度装着した後、処理を再試行していただけますようお願いします。

## その他の各種設定

前項までの手順により、OpenPGP機能に最低限必要な基本設定が完了しますが、その他にもPIN番号変更等のオプション機能を用意しております。<br>
詳細につきましては、別ドキュメント「[OpenPGP機能の各種設定手順](../../MaintenanceTool/macOSApp/PGPSETTING_OPT.md)」をご参照願います。
