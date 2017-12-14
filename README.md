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

## 各種手順

* [One Cardペアリング手順](Usage/PAIRING.md) <br>
ペアリングモード変更／ペアリング実行の手順を掲載しています。

* [鍵・証明書インストール手順](Usage/INSTALL.md) <br>
鍵・証明書インストールなどの手順を掲載しています。

* [FIDO U2F認証テスト手順](Usage/CERTTEST.md) <br>
FIDOアライアンスから提供されている、FIDO U2F認証取得のための事前テストツール（BLECertificationTool.exe）について、実行手順を掲載しています。

## 開発情報

開発に関する情報は、[こちらのページ](Development/README.md)にまとめて記載しております。

## 調査情報

調査に関する情報は、[こちらのページ](Research/README.md)にまとめて記載しております。
