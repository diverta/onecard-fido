# nRF Connect SDKに関する調査

新世代のnRF5開発処理系「[nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/index.html)」に関する調査を行います。

## 前提条件

まずは以下の手順書を参照し、現在使用中のSDK（nRF5 SDK v15.3.0）による開発環境が整備されていることを前提とします。

#### [NetBeansインストール手順](../../nRF5_SDK_v15.3.0/NETBEANSINST.md)
[nRF52840アプリケーション](../../nRF5_SDK_v15.3.0/README.md)のビルドに必要なツールを準備します。<br>
具体的には、nRF Connect SDKを新規導入するために最低限必要な、以下のツール群をインストールします。
- nRFコマンドラインツール（nrfjprogコマンドが含まれる）
- ARM GCCツールチェイン
- SEGGER J-Link

#### [nRF Connect for Desktop導入手順](../../nRF5_SDK_v15.3.0/NRFCONNECTINST.md)
nRF Connect SDKインストール時に必要なGUIツールを準備します。

#### [CMakeインストール手順](../../Research/nRFCnctSDK_v1.4.99/INSTALLCMAKE.md)
メイクファイル生成コマンド「cmake」を含むツール「CMake」を準備します。

## nRF Connect SDKの導入

nRF Connect for Desktopを使用し、nRF Connect SDKをインストールします。

## nRF Connect SDKサンプルの動作確認

Nordic社が用意しているnRF Connect SDK使用サンプルアプリの動作確認を行います。<br>

まずは現在使用中の開発ボード「nRF52840 DK」を使用して実施します。<br>
動作確認OKであれば、別途用意した「nRF5340 DK」を使用して動作確認を実施します。
