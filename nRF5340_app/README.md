# nRF5340アプリケーション

最終更新日：2021/10/14

## 概要

現在開発計画中のMDBT53 Dongle（nRF5340）上で稼働するアプリケーションです。<br>
[nRF Connect SDK](https://www.nordicsemi.com/Software-and-tools/Software/nRF-Connect-SDK)を使用し、開発しています。

## 搭載機能

後報

## ファームウェア

nRF5340アプリケーションのファームウェアは、フォルダー[`firmwares`](../nRF5340_app/firmwares) に格納予定です。

#### [ファームウェア更新機能](../nRF5340_app/DFUFUNC.md)
nRF5340アプリケーションでは、BLE経由、USB経由のファームウェア更新機能（DFU）を用意しています。<br>
開発中／製品化の各局面に応じ、使い分ける事が可能です。

## 開発環境構築手順

以下の手順書をご参照願います。

- <b>[ARM GCCインストール手順](../nRF52840_app/ARMGCCINST.md)</b>

- <b>[NetBeansインストール手順](../nRF52840_app/NETBEANSINST.md)</b>

- <b>[nRF Connect SDKインストール手順書](../nRF5340_app/INSTALLSDK.md)</b>

## 技術情報

- <b>[データの永続化について](../nRF5340_app/TECH_ZEP_SETTINGS.md)</b><br>
Zephyrプラットフォームにおけるデータの永続化に関する技術情報を、補足的に掲載しています。
