# One Card FIDO対応

One CardにFIDO U2F認証機能を実装するプロジェクトです。

## BLE U2Fサービス

One CardのBLE上で稼働するFIDO U2F機能です。<br>
U2F v1.2の仕様に準拠しております。

FIDO U2Fに関する情報 : https://fidoalliance.org/download/

FIDO U2F対応デバイスは、USBポートに挿して使用する「YubiKey」が有名ですが、BLE U2Fサービスは、USBポートではなく、One CardのBLEを使用しています。

コード格納場所--->[nRF5_SDK_v13.0.0](nRF5_SDK_v13.0.0)

## U2F管理ツール

PC環境から、BLE U2Fサービスの動作に必要な鍵・証明書の導入などを行うツールです。
GUIをもつmacOS版と、簡易CUIをもつWindows版を用意しました。

コード格納場所--->[U2FMaintenanceTool](U2FMaintenanceTool)

## 運用に関する情報

[こちらのページ](Usage/README.md)からご参照いただけます。

## 開発に関する情報

[こちらのページ](Development/README.md)からご参照いただけます。

## 調査に関する情報

[こちらのページ](Research/README.md)からご参照いただけます。

## 障害に関する情報

[Issues](https://github.com/diverta/onecard-fido/issues)からご参照いただけます。
