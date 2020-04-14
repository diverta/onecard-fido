# [開発運用] アプリケーション書込み手順

MDBT50Q Dongleにプレインストールされている[USBブートローダー（署名機能付き）](../nRF5_SDK_v15.3.0/firmwares/secure_bootloader/README.md)を使用して、MDBT50Q Dongleに[FIDO2認証器アプリケーション](../nRF5_SDK_v15.3.0/README.md)を書き込む手順を掲載いたします。

## 書込み準備

### ハードウェアの準備

まず最初に、MDBT50Q Dongleの背面にあるボタン電池ケースに、<b>電池が入っていないこと</b>を必ず確認します。

<img src="assets02/0000.png" width="400">

MDBT50Q DongleをPCのUSBポートに装着します。<br>
アイドル時であることを表示する緑色のLEDが点滅していることを確認します。

<img src="assets02/0010.jpg" width="400">

### ファームウェアの準備

ビルド済みのファームウェア更新イメージファイルが、GitHubリポジトリーの以下の場所に格納されています。
- ディレクトリー: [/nRF5_SDK_v15.3.0/firmwares/](../nRF5_SDK_v15.3.0/firmwares)
- ファームウェア更新イメージファイル: `app_dfu_package.nn.nn.nn.zip`

`app_dfu_package.nn.nn.nn.zip`の`nn.nn.nn`は、バージョン番号になります。<br>
例えば、バージョン`0.2.8`のファームウェア更新イメージファイル名は、`app_dfu_package.0.2.8.zip`となります。

### 書込み用ツールの準備

書込み用ツール「nRFコマンドラインツール」を、あらかじめPCに導入しておきます。<br>
詳細につきましては、手順書[「NetBeansインストール手順」](../nRF5_SDK_v15.3.0/NETBEANSINST.md)の該当部分をご参照ください。

## アプリケーションの書込み

### ブートローダーモードに遷移

Pythonスクリプト「[`HookUpHIDBLMode.py`](HookUpHIDBLMode.py)」を実行し、MDBT50Q Dongleをブートローダーモードに遷移させます。<br>
以下のコマンドを実行します。

```
GITHUB_DIR=${HOME}/GitHub/onecard-fido
python ${GITHUB_DIR}/nRF5_SDK_v15.3.0/HookUpHIDBLMode.py
```

下記は実行例になります。

```
MacBookPro-makmorit-jp:~ makmorit$ GITHUB_DIR=${HOME}/GitHub/onecard-fido
MacBookPro-makmorit-jp:~ makmorit$ python ${GITHUB_DIR}/nRF5_SDK_v15.3.0/HookUpHIDBLMode.py
HID device: path=USB_f055_0001_0x7f95e25070f0, usage_page=61904, usage=1
---- sent data ----
ffffffff860008d4e5f607182930410000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
---- received data ----
ffffffff860011d4e5f607182930410100330102050002070000000000000000
0000000000000000000000000000000000000000000000000000000000000000
---- INIT done ----
---- sent data ----
01003301c5000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
---- received data ----
01003301c5000100000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
---- command done ----
MacBookPro-makmorit-jp:~ makmorit$
```

コマンド実行が完了すると、MDBT50Q Dongleが自動的に、ブートローダーモードに遷移します。<br>
MDBT50Q Dongleの緑色・黄色のLEDが同時点灯していることを確認します。

<img src="../MaintenanceTool/macOSApp/assets02/0011.jpg" width="300">


### 書込み実行

nRFコマンドラインツールで`nrfutil dfu usb-serial`コマンドを実行し、仮想COMポート経由で、ファームウェア更新イメージファイルを転送します。<br>
具体的には、以下のコマンドを投入します。

```
FIRMWARES_DIR="${HOME}/GitHub/onecard-fido/nRF5_SDK_v15.3.0/firmwares"
cd ${FIRMWARES_DIR}
PACKAGE=`ls *.zip`
PORTNAME=`ls /dev/tty.usbmodem*`
echo command [nrfutil dfu usb-serial -pkg ${PACKAGE} -p ${PORTNAME}]
nrfutil dfu usb-serial -pkg ${PACKAGE} -p ${PORTNAME}
```

下記は実行例になります。

```
MacBookPro-makmorit-jp:~ makmorit$ FIRMWARES_DIR="${HOME}/GitHub/onecard-fido/nRF5_SDK_v15.3.0/firmwares"
MacBookPro-makmorit-jp:~ makmorit$ cd ${FIRMWARES_DIR}
MacBookPro-makmorit-jp:firmwares makmorit$ PACKAGE=`ls *.zip`
MacBookPro-makmorit-jp:firmwares makmorit$ PORTNAME=`ls /dev/tty.usbmodem*`
MacBookPro-makmorit-jp:firmwares makmorit$ echo command [nrfutil dfu usb-serial -pkg ${PACKAGE} -p ${PORTNAME}]
command [nrfutil dfu usb-serial -pkg app_dfu_package.0.2.8.zip -p /dev/tty.usbmodem1421]
MacBookPro-makmorit-jp:firmwares makmorit$ nrfutil dfu usb-serial -pkg ${PACKAGE} -p ${PORTNAME}
  [####################################]  100%          
Device programmed.
MacBookPro-makmorit-jp:firmwares makmorit$

```

### 書込み完了

書込処理が終了すると、MDBT50Q Dongleが自動的にリセットされ、アプリケーションがスタートします。<br>
アイドル時であることを表示する緑色のLEDが点滅していることを確認します。

<img src="assets02/0010.jpg" width="400">

以上で、MDBT50Q Dongleへのアプリケーション書込みは完了になります。
