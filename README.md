# One Card FIDO対応

One CardにFIDO U2F認証機能を実装するプロジェクトです。

## What's new

#### 2019/05/27（Version 0.1.13）

以下のプログラムを修正しました。<br>

- [nRF52840版 FIDO2認証器](nRF5_SDK_v15.2.0)
- [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp/MaintenanceTool.pkg)
- [Windows版 FIDO認証器管理ツール](MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)

主な修正点は以下になります。
- CTAP2ヘルスチェック機能を追加（`hmac-secret`検証機能付き）[注1]
- PIN認証で使用されるPINコードをメンテナンスする機能を追加
- CTAP2のBLEトランスポート対応[注2]
- U2F／CTAP2で使用する鍵・証明書管理を、USB HID経由で実行できるよう修正
- USB HIDサービスとBLEペリフェラルサービスが同居できないよう修正[注3]

[注1] `hmac-secret`検証機能＝ログイン実行ごとに、ブラウザーと認証器でやり取りされる暗号（Salt）の整合性をチェックすることにより、ユーザーや認証器の成り替わり・成りすましを抑止する機能<br>
[注2] Android Chromeは標準でWebAuthn対応していますが、現時点ではU2F認証が実行されます。<br>
[注3] USB HIDサービスと、BLE<b>セントラル</b>サービスは同居できるように実装しています。

#### CTAP2とは

FIDOの新世代パスワードレス認証（<b>WebAuthn</b>）に対応するために用意された、FIDO 2.0の技術仕様です。

[nRF52840版 FIDO2認証器](nRF5_SDK_v15.2.0)では、既にUSB HIDトランスポート、BLEトランスポートに対応しています。<br>
NFCトランスポートは、後日対応予定です。

また、Windows環境（Edgeブラウザー）でのWebAuthnは、PINコード（暗証番号）入力が必須となるのですが、こちらの方もすでに対応済みとなっております。

Windows環境による具体的なテスト方法は、別途手順書[「Edgeブラウザーを使用したWebAuthnテスト手順」](Research/FIDO_2_0/EDGETEST.md)をご参照ください。

#### [過去の更新履歴はこちら](HISTORY.md)

## 以前の仕様

以下の項目は、FIDOの旧世代２要素認証（U2F）に関する開発物件になります。<br>
[nRF52840版 FIDO2認証器](nRF5_SDK_v15.2.0)においては、U2FはCTAP2と同居していますので、現在も稼働させることができます。

ただし、U2FはChromeブラウザーのみのサポートであり、かつ将来的にサポートが拡張される予定もないため、現在はメンテナンスをストップさせております。<br>
何卒ご容赦ください。

### BLE U2Fサービス

One CardのBLE上で稼働するFIDO U2F機能です。<br>
U2F v1.2の仕様に準拠しております。

FIDO U2Fに関する情報 : https://fidoalliance.org/download/

FIDO U2F対応デバイスは、USBポートに挿して使用する「YubiKey」が有名ですが、BLE U2Fサービスは、USBポートではなく、One CardのBLEを使用しています。

コード格納場所--->[nRF5_SDK_v13.0.0](nRF5_SDK_v13.0.0)

### U2F管理ツール

PC環境から、BLE U2Fサービスの動作に必要な鍵・証明書の導入などを行うツールです。
macOS版と、Windows版を用意しました。

コード格納場所--->[U2FMaintenanceTool](U2FMaintenanceTool)

## 運用に関する情報

[こちらのページ](Usage/README.md)からご参照いただけます。

## 開発に関する情報

[こちらのページ](Development/README.md)からご参照いただけます。

## 調査に関する情報

[こちらのページ](Research/README.md)からご参照いただけます。

## 障害に関する情報

[Issues](https://github.com/diverta/onecard-fido/issues)からご参照いただけます。
