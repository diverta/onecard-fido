# One Card FIDO対応

### コード格納場所

[nRF5_SDK_v13.0.0](nRF5_SDK_v13.0.0)

### 概要

One CardのBLEアプリケーションに、FIDO U2Fに対応するためのコードを追加いたしました。<br>
FIDO U2Fに関する情報 : https://fidoalliance.org/download/

FIDO U2F対応デバイスは、USBポートに挿して使用する「YubiKey」が有名ですが、One Card FIDO U2F対応は、USBポートではなく、One CardのBLEを使用しています。

本GitHub上では、One Card上で稼働するFIDO U2F機能を「BLE U2Fサービス」と称します。

### 機能
* 初期設定機能 - FIDO U2F対応のために必要な初期設定を行います。
* FIDO U2F機能 - FIDO U2F対応のメイン機能（認証情報登録／認証）です。

### 動作環境

BLE (Bluetooth Low Energy) 4.2以上が推奨されますが、4.0でも動作することを確認しています。

### サポート状況

2017/12/11現在で、BLE U2Fサービスで認証情報登録／認証ができることを確認しているサイトは以下になります。

U2F Demo :<br> https://crxjs-dot-u2fdemo.appspot.com/

<img src="assets/0003.png" width="600">

他のPC環境（Windows、macOS等）では、ChromeブラウザーがFIDO U2FのBLEエクステンションをサポートしていないので、2017/12/11時点では動作確認できておりません。

### TODO

#### Android環境でのサポート状況確認

Android向けGoogle Playで既にサポートずみ（とのこと）である、BLE U2Fサービスが利用できるかどうかを、実機で確認する予定です。

[ご参考] Googleサポート状況に関する議論：<br>
https://groups.google.com/a/fidoalliance.org/forum/#!topic/fido-dev/-hT1UF0FKTo

#### ブラウザーエクステンションの調査・開発

PC環境で動作するFIDO U2F BLEエクステンションの調査・開発を予定しております。


# U2F管理ツール

### コード格納場所
[U2FMaintenanceTool](U2FMaintenanceTool)

### 概要

PC環境から、BLE U2Fサービスの動作に必要な鍵・証明書の導入などを行うツールです。<br>
GUIをもつmacOS版と、簡易CUIをもつWindows版を用意しました。

### macOS版

<img src="assets/0001.png" width="600">

#### 機能
* ペアリング情報削除
* 鍵・証明書削除
* 鍵・証明書インストール
* ヘルスチェック実行

#### 動作環境
macOS Sierra (Version 10.12.6)

### Windows版

<img src="assets/0002.png" width="600">

#### 機能
* ペアリング情報削除
* 鍵・証明書削除
* 鍵・証明書インストール

#### 動作環境
Windows 10 (32bit版)
