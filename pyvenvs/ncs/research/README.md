# nRF Connect SDK（TF-M）に関する調査

新世代のnRF5開発処理系「[nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/index.html)」に関する調査を行います。<br>
ここでは主に、[Trusted firmware-M（TF-M）](https://github.com/zephyrproject-rtos/trusted-firmware-m)に関する調査を実施します。

## 前提条件・制約など

#### 使用環境
使用するボード: nRF5340 DK

PC: iMac<br>
OS: macOS Catalina（Version 10.15.5）<br>
このシステムに、現在使用中のnRF Connect SDKによる開発環境が整備されていることを前提としております。<br>
（下記手順書をご参照）

#### [nRF Connect SDKインストール手順書](../../../nRF5340_app/INSTALLSDK.md)
「[nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/index.html)」をmacOSにインストールする手順について掲載します。
