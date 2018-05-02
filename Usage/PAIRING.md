# One Cardペアリング手順

One CardおよびBLE U2Fサービスを使用するためには、ペアリングが必要となります。

ペアリングを行うためには、One Cardをペアリングモードに変更します。

## 非ペアリングモード--->ペアリングモードへの変更

One Cardに電源を投入すると、非ペアリングモードで起動しますが、これではペアリングを行うことができません。

ペアリングモードに変更させるためには、One CardのMAIN SWを５秒間押し続けます。

<img src="../assets/0012.png" width="500">

One Card上の２番目のLEDが<font color=ff0000><b>点灯</b></font>したら、指をMAIN SWから離します。

<img src="../assets/0013.png" width="500">

これでペアリングモードに変更されました。

## ペアリングの実施

### Windowsの場合

U2F管理ツールの「ペアリング実行」ボタンをクリックします。

<img src="assets/0054.png" width="500">

約30秒ほど、ペアリング先の探索処理のため画面がクリックできない状態となります。

<img src="assets/0055.png" width="500">

ペアリングが成功すると、ポップアップ画面が表示されます。

<img src="assets/0056.png" width="500">

Windowsの設定画面からBluetooth設定を参照すると、下図のように「OneCard_Peripheral」がペアリング済みであることが確認できます。

<img src="assets/0057.png" width="500">

これで、Windowsとのペアリングは完了です。

### macOSの場合

U2F管理ツールの「ペアリング実行」ボタンをクリックします。


約30秒ほど、ペアリング先の探索処理のため画面がクリックできない状態となります。


ペアリングが成功すると、ポップアップ画面が表示されます。


macOSの設定画面からBluetooth設定を参照すると、下図のように「OneCard_Peripheral」がペアリング済みであることが確認できます。

<img src="../assets/0019.png" width="500">

これで、macOSとのペアリングは完了です。


### ご参考
ペアリングが正常完了すると、One Cardは自動的に非ペアリングモードに移行します。<br>
非ペアリングモードに移行すると、One Card上の２番目のLEDが消灯します。

<img src="../assets/0076.png" width="500">


## ペアリングモード--->非ペアリングモードへの変更

ペアリングモードから、非ペアリングモードに変更させるためには、One CardのMAIN SWを５秒間押し続けます。

<img src="../assets/0075.png" width="500">

One Card上の２番目のLEDが<font color=ff0000><b>消灯</b></font>したら、指をMAIN SWから離します。

<img src="../assets/0076.png" width="500">

これで非ペアリングモードに変更されました。

### ご参考
One Cardは、電源Off状態から電源投入時は、常に非ペアリングモードで起動します。
