# 簡易USBブートローダー

Nordic社から提供されているサンプル「[Open Bootloader with DFU](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v15.2.0/ble_sdk_app_open_bootloader.html)」を小修正し、[MDBT50Q Dongle](../../../../FIDO2Device/MDBT50Q_Dongle/README.md)で利用できるようにしたものです。

Nordic社製の開発基板「nRF52840 Dongle」にあらかじめセットアップされているブートローダーと、同じ仕様になっております。

## 作成手順

NetBeansとARM GCC、nRF5 SDKを使用し、簡易USBブートローダーを作成する手順を記載します。

### NetBeans環境の作成

あらかじめ、NetBeans環境をPCに作成しておきます。<br>
具体的な手順は、[NetBeansインストール手順](../../../../nRF5_SDK_v15.3.0/NETBEANSINST.md)をご参照ください。

### ソースファイルの準備

#### オリジナルソースの取得

nRF5 SDKのサンプルアプリケーション・フォルダー（/nRF5_SDK_15.3.0/examples/dfu）から、必要なソースコードを取得します。

- `open_bootloader`サブフォルダー
- `dfu_public_key.c` - [README](../README.md)の「公開鍵ファイル」の項に記載した通りの手順で、あらかじめ作成しておきます。

今回の作成にあたっては、[`<リポジトリールート>/nRF5_SDK_v15.3.0/examples/dfu`](../../../../nRF5_SDK_v15.3.0/examples/dfu)配下に配置いたしました。

#### メイクファイルの修正

メイクファイル「[Makefile](../../../../nRF5_SDK_v15.3.0/examples/dfu/open_bootloader/pca10059_usb/armgcc/Makefile)」の下記部分を修正します。

<b>修正前</b>
```
SDK_ROOT := ../../../../..
```

<b>修正後</b>
```
SDK_ROOT := $(HOME)/opt/nRF5_SDK_15.3.0
```

### ソースファイルからビルド

上記で取得したソースファイルから、NetBeansプロジェクトを新規作成し、簡易USBブートローダー（`.hex`ファイル）を生成します。

#### プロジェクトの新規作成〜ビルド実行

NetBeansを起動し、ファイル--->新規プロジェクトを実行します。

<img src="../assets01/0001.png" width="300">

新規プロジェクト画面が表示されますので、一覧から「既存のソースを使用するC/C++プロジェクト」を選択し「次 >」をクリックします。

<img src="../assets01/0002.png" width="500">

下図のような画面に遷移しますので、以下のように設定します。

- 既存のソースを含むフォルダを指定 - サンプルアプリが格納されているフォルダー「`examples/dfu/open_bootloader`」を指定します。<br>
下図の例では「`/Users/makmorit/GitHub/onecard-fido/nRF5_SDK_v15.3.0/examples/dfu/open_bootloader`」という文字列が設定されています。

- 構成モードを選択 - 「カスタム(C)」をチェックします。

設定が完了したら「次 >」をクリックします。

<img src="../assets01/0003.png" width="500">

下図のような画面に遷移しますので、以下のように設定します。

- 「事前ビルド・ステップが必要」にチェック

- フォルダで実行(U) - サンプルアプリのサブフォルダー「`pca10059_usb/armgcc`」を指定します。<br>
下図の例では「`/Users/makmorit/GitHub/onecard-fido/nRF5_SDK_v15.3.0/examples/dfu/open_bootloader/pca10059_usb/armgcc`」という文字列が設定されています。

- 「カスタム・コマンド」にチェック

- コマンド(O) - 「make」と入力します。

設定が完了したら「次 >」をクリックします。

<img src="../assets01/0004.png" width="500">

「4. ビルド・アクション」に遷移しますが、以降は「7. プロジェクトの名前と場所」に遷移するまではデフォルト設定のまま「次 >」をクリックします。

<img src="../assets01/0005.png" width="500">

「7. プロジェクトの名前と場所」に遷移したら、プロジェクト名(P)を「open_bootloader」から「open_bootloader_proj」に変更しておきます。<br>
（オリジナルのプロジェクト「open_bootloader」を上書きしたくないための措置です）

設定が完了したら「終了(F)」をクリックします。

<img src="../assets01/0006.png" width="500">

自動的にビルドがスタートしますので、しばらくそのまま待ちます。<br>
しばらくするとビルドが完了し「ビルド SUCCESSFUL」と表示されれば、ビルドは成功です。

<img src="../assets01/0007.png" width="600">

#### ビルド結果の確認

ビルドが完了したら、簡易USBブートローダー`nrf52840_xxaa.hex`が正しく生成されているかどうか確認します。<br>
下図は、Finderで`nrf52840_xxaa.hex`(109KB)が生成されたことを確認したところです。

<img src="../assets01/0008.png" width="500">

以上で、ソースファイルからのビルドは完了です。
