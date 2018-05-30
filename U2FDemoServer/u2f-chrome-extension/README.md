# U2Fエクステンション

macOSに導入されたChromeブラウザーで、BLEデバイスを使用したU2F認証ができるよう対応した、Chromeのエクステンションです。

正式名称は「FIDO U2F (Universal 2nd Factor) extension」となっております。

## このコードについて

### オリジナルからの修正点

Googleが[GitHub](https://github.com/google/u2f-ref-code/tree/master/u2f-chrome-extension)で公開している、U2Fエクステンション（u2f-chrome-extension）をベースに、下記の通り若干の修正をしたものです。

* [macOS版U2F管理ツール](../../../U2FMaintenanceTool/README.md)と連携するためのヘルパーコード（[blehelper.js](blehelper.js)）を新設

* YubiKey用のUsbHelperに加え、BleHelper（blehelper.js内で定義されたクラス）を扱うよう修正

### オリジナルのREADME

こちらのページ（[README.original.md](README.original.md)）をご参照ください。

## 制限事項

* <b>パッケージ化するとU2Fデモサイトが使用できません</b><br>
本エクステンションのコードをパッケージ化したエクステンション（.crx）をインストールすると、エクステンションのIDが変わってしまうため、U2Fデモサイト (https://crxjs-dot-u2fdemo.appspot.com/) が指定したIDと一致しなくなり、結果認証テストが失敗します。<br>
このため、パッケージ化されていない状態で、Chromeブラウザーにインストールするようにします。

* <b>macOS版U2F管理ツールの導入が必要です</b><br>
本エクステンションで、One Cardを使用してU2F認証を行う場合は、別途[macOS版U2F管理ツール](../../../U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)をインストールする必要があります。<br>
（YubiKeyを使用してU2F認証を行う場合は、YubiKeyをPCに挿すだけで使用することができます。）

* <b>Windows、Chrome OS環境では稼働しません</b><br>
2018/01/23現在、本エクステンションでサポートする環境はmacOSのみであり、Windows環境、Chrome OS環境はサポートしておりません。<br><br>
Chrome OSについては（macOSサポート調査のため）一時的に外したU2F BLEエクステンション（[u2f-ble-helper](../../u2f-ble-helper/README.md)）と連携できる状態に戻す予定です。<br>
Windowsについては、対応のための調査／実装を、後日実施する予定です。

## 使用例

以下のページをご参照ください。

* [Chrome(macOS版)でのBLE U2F対応調査](../../../Research/CHROMEBLEEXT.md)

* [Chrome OSでのU2F認証テスト](../../../Research/CHROMEOSTEST.md)
