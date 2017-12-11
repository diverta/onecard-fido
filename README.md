# One Card FIDO対応

## 概要
One CardのBLEアプリケーションに、FIDO U2Fに対応するためのコードを追加いたしました。<br>
FIDO U2Fに関する情報 : https://fidoalliance.org/download/

FIDO U2F対応デバイスは、USBポートに挿して使用する「YubiKey」が有名ですが、One Card FIDO U2F対応は、USBポートではなく、One CardのBLEを使用しています。

## コード格納場所
[nRF5_SDK_v13.0.0](nRF5_SDK_v13.0.0)

# U2F管理ツール

## 概要
PC環境から、BLE U2Fサービスの動作に必要な鍵・証明書の導入などを行うツールです。<br>
（本GitHub上では、One Card上で稼働するFIDO U2F機能を「BLE U2Fサービス」と称します。）

GUIをもつmacOS版と、簡易CUIをもつWindows版を用意しました。

## コード格納場所
[U2FMaintenanceTool](U2FMaintenanceTool)
