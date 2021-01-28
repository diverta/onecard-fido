# nRF52840アプリケーション初回導入手順書

## 概要

[MDBT50Q Dongle](../../../FIDO2Device/MDBT50Q_Dongle/README.md)に、[nRF52840アプリケーション](../../../nRF52840_app/firmwares/secure_device_app)を<b>新規に書き込む</b>手順を、以下に掲載いたします。

## 注意事項

この移行作業を実施すると、MDBT50Q DongleのFlash ROM全領域が消去されるため、MDBT50Q Dongleに導入した秘密鍵・証明書や、PIN番号、WebAuthn認証情報などは、すべて消去されます。

## MDBT50Q Dongleの初期化

[USBブートローダー](../../../nRF52840_app/firmwares/secure_bootloader)（ファームウェアを書き込むためのプログラム）は、nRF52840アプリケーションからのみ起動が可能、という仕様になっております。<br>
（不用意にアプリケーションの書込み／消去ができないようにするための措置）<br>
したがって、ファームウェア更新イメージを新規に書込むためには、いったんMDBT50Q Dongleを初期化する必要があります。

詳細につきましては、別途ドキュメント「[USBブートローダー書込み手順書](../../../nRF52840_app/firmwares/secure_bootloader/WRITESBL.md)」をご参照ください。

前述手順により、MDBT50Q Dongleの初期化が完了すると、基板上で緑色・橙色のLEDが同時点灯している状態となります。

<img src="../../../nRF52840_app/firmwares/sample_blehrs/assets02/0002.jpg" width="200">

## ファームウェアの書込み

nRF Utilを使用し、MDBT50Q Dongleに、[nRF52840アプリケーション](../../../nRF52840_app/firmwares/secure_device_app)を書込みます。

#### nRF Utilのインストール

ビルドを実行する際に必要となる、nRF UtilをPCにインストールしておきます。<br>
具体的な手順は、[nRF Utilインストール手順](../../../nRF52840_app/NRFUTILINST.md)をご参照ください。

本手順書を作成した時点でのnRF Utilは、`version 6.1`となっておりました。

```
bash-3.2$ nrfutil version
nrfutil version 6.1.0
bash-3.2$
```

#### ファームウェア更新イメージの書込み

`nrfutil dfu usb-serial`コマンドを実行し、仮想COMポート経由で、基板に対応するファームウェア更新イメージファイルを転送します。

| # |ファイル名 |説明 |
|:-:|:-|:-|
|1|`appkg.PCA10059_01.nn.nn.nn.zip`|[MDBT50Q Dongle（rev2）](../../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2/README.md)専用ファームウェア更新イメージファイル|
|2|`appkg.PCA10059_02.nn.nn.nn.zip`|[MDBT50Q Dongle（rev2.1.2）](../../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_1_2/README.md)専用ファームウェア更新イメージファイル|

具体的には、以下のコマンドを投入します。<br>
下記例ではMDBT50Q Dongle（rev2.1.2）のファームウェア更新イメージファイルを使用しています。

```
FIRMWARES_DIR="${HOME}/GitHub/onecard-fido/nRF52840_app/firmwares/secure_device_app/"
cd ${FIRMWARES_DIR}
PACKAGE=`ls appkg.PCA10059_02.*.zip`
PORTNAME=`ls /dev/tty.usbmodem*`
echo command [nrfutil dfu usb-serial -pkg ${PACKAGE} -p ${PORTNAME}]
nrfutil dfu usb-serial -pkg ${PACKAGE} -p ${PORTNAME}
```

下記は実行例になります。

```
bash-3.2$ FIRMWARES_DIR="${HOME}/GitHub/onecard-fido/nRF52840_app/firmwares/secure_device_app/"
bash-3.2$ cd ${FIRMWARES_DIR}
bash-3.2$ PACKAGE=`ls appkg.PCA10059_02.*.zip`
bash-3.2$ PORTNAME=`ls /dev/tty.usbmodem*`
bash-3.2$ echo command [nrfutil dfu usb-serial -pkg ${PACKAGE} -p ${PORTNAME}]
command [nrfutil dfu usb-serial -pkg appkg.PCA10059_02.0.3.0.zip -p /dev/tty.usbmodemD6209557A6AE1]
bash-3.2$ nrfutil dfu usb-serial -pkg ${PACKAGE} -p ${PORTNAME}
  [####################################]  100%          
Device programmed.
bash-3.2$
```

#### 書込み完了

書込処理が終了すると、MDBT50Q Dongleが自動的にリセットされ、nRF52840アプリケーションがスタートします。<br>
今度は、アイドル時であることを表示する緑色のLEDが点滅していることを確認します。

<img src="../../../nRF52840_app/firmwares/secure_device_app/assets01/0010.jpg" width="200">

## ブートローダーモード遷移機能の確認

この状態のMDBT50Q Dongleは、nRF52840アプリケーションが稼働している状況となっております。<br>
他方、MDBT50Q DongleはすでにUSBブートローダーが書き込まれているため、管理ツールを使用し、後日、nRF52840アプリケーションの更新ができるようになります。

ここでは管理ツールを使用し、ブートローダーモードに遷移できるかどうか確認を行います。<br>
手順につきましては別ドキュメント「[ブートローダーモード遷移手順書](../../../nRF52840_app/firmwares/secure_device_app/BLMODE.md)」をご参照ください。

ブートローダーモードに遷移すると、MDBT50Q Dongleの基板上で、緑色・橙色のLEDが同時点灯します。

<img src="../../../nRF52840_app/firmwares/sample_blehrs/assets02/0002.jpg" width="200">

確認が終わったら、いったんMDBT50Q Dongleを取り外し、再度PCのUSBポートに装着します。<br>
今度は、アイドル時であることを表示する緑色のLEDが点滅していることを確認します。

<img src="../../../nRF52840_app/firmwares/secure_device_app/assets01/0010.jpg" width="200">

以上で、nRF52840アプリケーションの初回導入は完了です。
