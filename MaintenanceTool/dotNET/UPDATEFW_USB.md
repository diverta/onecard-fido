# ファームウェア更新手順

## 概要

[FIDO認証器管理ツール](README.md)を使用し、[MDBT50Q Dongle](../../FIDO2Device/MDBT50Q_Dongle/README.md)のファームウェアを更新する手順を掲載します。

## 操作方法

まず最初に、MDBT50Q Dongleの背面にあるボタン電池ケースに、電池が入っていないことを必ず確認します。

<img src="../../MaintenanceTool/macOSApp/assets02/0000.jpg" width="400">

次に、MDBT50Q DongleをPCのUSBポートに装着します。<br>
この時に、MDBT50Q Dongleの緑色のLEDが点滅していることを確認します。

<img src="../../MaintenanceTool/macOSApp/assets02/0010.jpg" width="400">

管理ツールを起動し、画面下部のメッセージ欄に「USB HIDデバイスに接続されました。」と表示されることを確認したら「ファームウェア更新」ボタンをクリックします。

<img src="assets03/0001.jpg" width="400">

ファームウェア更新画面が表示されます。<br>
画面上部のボックス「MDBT50Q用」の中にあるボタン「ファームウェアを更新（USB）」をクリックします。

<img src="assets03/0002.jpg" width="400">

処理開始画面が表示されます。<br>
MDBT50Q Dongleに導入されているファームウェアのバージョンと、更新するバージョンが表示されます。

<img src="assets03/0003.jpg" width="400">

上図、処理開始画面の「OK」ボタンをクリックすると、MDBT50Q Dongleが自動的に、ブートローダーモードに遷移します。<br>
MDBT50Q Dongleの緑色・黄色のLEDが同時点灯していることを確認します。

<img src="../../MaintenanceTool/macOSApp/assets02/0011.jpg" width="300">

ほどなく、現在の進捗を示すダイアログが表示されます。<br>
まずはファームウェア更新イメージが転送中であることを知らせています。

<img src="assets03/0004.jpg" width="400">

次に、MDBT50Q Dongle内で、転送されたファームウェア更新イメージが反映中であることを知らせています。

<img src="assets03/0005.jpg" width="400">

ファームウェア更新処理が正常終了すると、下図のようなポップアップが表示され、処理が成功したことを知らせます。

<img src="assets03/0006.jpg" width="400">

再び、MDBT50Q Dongleの緑色のLEDが点滅していることを確認します。

<img src="../../MaintenanceTool/macOSApp/assets02/0010.jpg" width="400">

以上で、ファームウェア更新処理は完了となります。
