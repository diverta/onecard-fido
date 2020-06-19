# 以前の仕様

以下の項目は、FIDOの旧世代２要素認証（U2F）に関する開発物件になります。<br>
[FIDO2アプリケーション](nRF5_SDK_v15.3.0)においては、U2FはCTAP2と同居していますので、現在も稼働させることができます。

ただし、U2FはChromeブラウザーのみのサポートであり、かつ将来的にサポートが拡張される予定もないため、現在はメンテナンスをストップさせております。<br>
何卒ご容赦ください。

### BLE U2Fサービス

One CardのBLE上で稼働するFIDO U2F機能です。<br>
U2F v1.2の仕様に準拠しております。

FIDO U2Fに関する情報 : https://fidoalliance.org/download/

FIDO U2F対応デバイスは、USBポートに挿して使用する「YubiKey」が有名ですが、BLE U2Fサービスは、USBポートではなく、One CardのBLEを使用しています。

コード格納場所--->[nRF5_SDK_v13.0.0](nRF5_SDK_v13.0.0)

### U2F管理ツール

PC環境から、BLE U2Fサービスの動作に必要な鍵・証明書の導入などを行うツールです。<br>
macOS版と、Windows版を用意しました。

コード格納場所--->[U2FMaintenanceTool](U2FMaintenanceTool)
