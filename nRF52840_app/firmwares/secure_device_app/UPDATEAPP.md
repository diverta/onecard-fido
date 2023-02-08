# nRF52840アプリケーション更新手順書（開発時運用）

最終更新日：2023/2/8

## 概要

[MDBT50Q Dongle](../../../FIDO2Device/MDBT50Q_Dongle/README.md)に、最新バージョンの[nRF52840アプリケーション](../../../nRF52840_app/firmwares/secure_device_app)を<b>手動で上書き更新する</b>手順について掲載しています。

## MDBT50Q Dongleの準備

<b>version 0.3.0</b>以降の[nRF52840アプリケーション](../../../nRF52840_app/firmwares/secure_device_app)が書き込まれたMDBT50Q Dongleを、あらかじめ準備します。

次に、MDBT50Q Dongleをブートローダーモードに遷移させ、nRF52840アプリケーションを書き込むことが可能な状態にします。<br>
手順につきましては下記ドキュメントをご参照ください。<br>
　・[ブートローダーモード遷移手順書（Windows版）](../../../MaintenanceTool/dotNET/BOOTLOADERMODE.md)<br>
　・[ブートローダーモード遷移手順書（macOS版）](../../../MaintenanceTool/macOSApp/BOOTLOADERMODE.md)

ブートローダーモードに遷移すると、MDBT50Q Dongleの基板上で、緑色・橙色のLEDが同時点灯します。

<img src="../../../nRF52840_app/firmwares/sample_blehrs/assets02/0002.jpg" width="200">

## ファームウェアの書込み

nRF Utilを使用し、MDBT50Q Dongleに、[nRF52840アプリケーション](../../../nRF52840_app/firmwares/secure_device_app)を書込みます。

#### nRF Utilのインストール

ビルドを実行する際に必要となる、nRF UtilをPCにインストールしておきます。<br>
具体的な手順は、[nRF Utilインストール手順](../../../nRF52840_app/NRFUTILINST.md)をご参照ください。

本手順書を作成したときのnRF Utilは`version 6.1`です。

```
bash-3.2$ nrfutil version
nrfutil version 6.1.0
bash-3.2$
```

#### ファームウェア更新イメージの書込み

`nrfutil dfu usb-serial`コマンドを実行し、仮想COMポート経由で、基板に対応するファームウェア更新イメージファイルを転送します。

| # |ファイル名 |説明 |
|:-:|:-|:-|
|1|`appkg.PCA10059_03.nn.nn.nn.zip`|[MDBT50Q Dongle（rev2.2）](../../../FIDO2Device/MDBT50Q_Dongle/pcb_rev2_2/README.md)専用ファームウェア更新イメージファイル|

具体的には、以下のコマンドを投入します。<br>
下記例ではMDBT50Q Dongle（rev2.2）のファームウェア更新イメージファイル（Version 0.3.7）を使用しています。

```
BOARD_STR=PCA10059_03
VERSION_STR=0.3.7
PACKAGE=appkg.${BOARD_STR}.${VERSION_STR}.zip
FIRMWARES_DIR="${HOME}/GitHub/onecard-fido/nRF52840_app/firmwares/secure_device_app/"
cd ${FIRMWARES_DIR}
PORTNAME=`ls /dev/tty.usbmodem*`
echo command [nrfutil dfu usb-serial -pkg ${PACKAGE} -p ${PORTNAME}]
nrfutil dfu usb-serial -pkg ${PACKAGE} -p ${PORTNAME}
```

下記は実行例になります。

```
bash-3.2$ BOARD_STR=PCA10059_03
bash-3.2$ VERSION_STR=0.3.7
bash-3.2$ PACKAGE=appkg.${BOARD_STR}.${VERSION_STR}.zip
bash-3.2$ FIRMWARES_DIR="${HOME}/GitHub/onecard-fido/nRF52840_app/firmwares/secure_device_app/"
bash-3.2$ cd ${FIRMWARES_DIR}
bash-3.2$ PORTNAME=`ls /dev/tty.usbmodem*`
bash-3.2$ echo command [nrfutil dfu usb-serial -pkg ${PACKAGE} -p ${PORTNAME}]
command [nrfutil dfu usb-serial -pkg appkg.PCA10059_02.0.3.7.zip -p /dev/tty.usbmodemC6863701200B1]
bash-3.2$ nrfutil dfu usb-serial -pkg ${PACKAGE} -p ${PORTNAME}
  [####################################]  100%          
Device programmed.
bash-3.2$
```

#### 書込み完了

書込処理が終了すると、MDBT50Q Dongleが自動的にリセットされ、nRF52840アプリケーションがスタートします。<br>
今度は、アイドル時であることを表示する緑色のLEDが点滅していることを確認します。

<img src="../../../nRF52840_app/firmwares/secure_device_app/assets01/0010.jpg" width="200">

以上で、nRF52840アプリケーションの上書き更新は完了です。
