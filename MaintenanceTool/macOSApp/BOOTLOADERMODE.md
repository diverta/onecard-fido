# ブートローダーモード遷移手順書

[FIDO認証器管理ツール（ベンダー向け）](../../MaintenanceTool/macOSApp/DEVTOOL.md)を使用して、FIDO2認証器をブートローダーモードに遷移させる手順について記載しています。

## 認証器の準備

#### 使用機材

本ドキュメントでは「[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)」を、FIDO2認証器として使用します。

<img src="../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/assets/0001.jpg" width="300">

#### 管理ツールを導入

[FIDO認証器管理ツール（ベンダー向け）](../../MaintenanceTool/macOSApp/DEVTOOL.md)を、PC環境（macOS）に導入します。<br>
以下の手順書をご参照願います。

* <b>[FIDO認証器管理ツール（ベンダー向け） インストール手順](../../MaintenanceTool/macOSApp/DEVTOOLINST.md) </b>

## ブートローダーモード遷移手順

管理ツールのベンダー向け機能を使用し、MDBT50Q Dongleをブートローダーモードに遷移させます。

#### ブートローダーモード遷移の実行

管理ツール（ベンダー向け）を起動し、USBポートにFIDO2認証器を装着します。

<img src="assets/0028.jpg" width="400">

管理ツール画面下部のメッセージ欄に「USB HIDデバイスに接続されました。」と表示されることを確認したら、管理ツールのメニューから「ベンダー向け機能」を選択します。

<img src="assets/0029.jpg" width="400">

ベンダー向け機能画面が表示されますので「ブートローダーモード遷移」ボタンをクリックします。

<img src="assets/0031.jpg" width="400">

下図のような確認ダイアログが表示されます。<br>
「Yes」ボタンをクリックし、ブートローダーモード遷移処理を実行させます。

<img src="assets/0032.jpg" width="400">

ほどなく遷移処理が完了し、ポップアップが表示されます。

<img src="assets/0033.jpg" width="400">

「OK」をクリックし、ポップアップを閉じます。

#### 遷移結果の確認

ブートローダーモード遷移処理の実行が完了すると、MDBT50Q Dongleの基板上で、緑色・橙色のLEDが同時点灯します。<br>
これはMDBT50Q Dongleが、ブートローダーモードである（＝アプリケーションのファームウェア更新イメージが書き込み可能である）状態を示しております。

<img src="../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/assets/0006.jpg" width="300">

以上で、FIDO2認証器のブートローダーモード遷移は完了となります。
