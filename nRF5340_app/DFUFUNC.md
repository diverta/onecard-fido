# ファームウェア更新機能

最終更新日：2021/10/14

## 概要

[nRF5340アプリケーション](../nRF5340_app/README.md)では、BLE経由、USB経由のファームウェア更新機能（DFU）を用意いたします。

## DFU方式について

nRF5340アプリケーションをビルドの際、BLE経由、USB経由の両方をサポートすると、生成されるファームウェア更新イメージのサイズが肥大化し、DFU自体の所要時間が長くなる等の影響があります。<br>
この問題を回避するため、本プロジェクトでは、BLE、USBのいずれを経由するかを、ビルド時に指定するものとしております。

最終製品イメージとしては、セキュリティー性向上の観点から、BLE経由のDFU機能を搭載する想定です。<br>
BLE経由のDFUは、ファームウェア更新イメージの署名、および通信データの暗号化が行われます。

一方、開発イメージとしては、速度向上の観点から、USB経由のDFUを搭載する想定です。<br>
USB経由のDFUは、ファームウェア更新イメージの署名は行いますが、通信データは暗号化されません。

いずれも内部処理的には、[Zephyr](https://docs.zephyrproject.org/latest/index.html)の[MCUmgr](https://docs.zephyrproject.org/latest/guides/device_mgmt/mcumgr.html)というパッケージを使用しております。

#### ビルド方法

nRF5340アプリケーションをビルドする際、DFU機能がBLE／USBのいずれを経由するかを、下記手順により指定します。

<b>【BLE DFU搭載版】</b><br>
最終製品イメージには、セキュリティー性向上の観点から、BLE経由のDFUを搭載します。<br>
nRF5340アプリケーションのビルド用ファイル[`overlay-smp.conf`](../nRF5340_app/secure_device_app/overlay-smp.conf)を使用し、ビルドを行います。

ビルド時はスクリプト[`westbuild.sh`](../nRF5340_app/secure_device_app/westbuild.sh)に下記のような記述を行います。

```
    ${NCS_HOME}/bin/west build -c -b ${BUILD_TARGET} -d build_signed -- -DOVERLAY_CONFIG=overlay-smp.conf
```

<b>【USB DFU搭載版】</b><br>
開発中のイメージには、速度向上の観点から、USB経由のDFUを搭載します。<br>
nRF5340アプリケーションのビルド用ファイル[`overlay-usb-dfu.conf`](../nRF5340_app/secure_device_app/overlay-usb-dfu.conf)を使用し、ビルドを行います。

ビルド時はスクリプト[`westbuild.sh`](../nRF5340_app/secure_device_app/westbuild.sh)に下記のような記述を行います。

```
    ${NCS_HOME}/bin/west build -c -b ${BUILD_TARGET} -d build_signed -- -DOVERLAY_CONFIG=overlay-usb-dfu.conf
```

## 各種手順書

- <b>[ファームウェア更新手順（USB）](INSTALLFW_USB.md)</b><br>
nRF5340アプリケーションを、USB経由でインストールする手順について掲載しています。

## 開発情報

現在開発中のBLE DFU機能について、各種調査結果を掲載しています。

- <b>[BLE DFU機能のトランザクション](../nRF5340_app/BLEDFU_TRANSACTION.md)</b><br>
BLE DFU機能について、処理中に発生するトランザクションの情報を掲載しています。

- <b>[BLE DFU機能のサンプルコード調査](../nRF5340_app/BLEDFU_FUNC_IOS.md)</b><br>
Nordic社から提供されている、iOS版のBLE DFUアプリのサンプルコードについて調査した結果を掲載しています。
