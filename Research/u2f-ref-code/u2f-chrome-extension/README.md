# U2Fエクステンション

Chrome OSに導入されたChromeブラウザーでU2F認証ができるよう対応した、Chromeのエクステンションです。

正式名称は「FIDO U2F (Universal 2nd Factor) extension」となっております。

## このコードについて

### オリジナルからの修正点

Googleが[GitHub](https://github.com/google/u2f-ref-code/tree/master/u2f-chrome-extension)で公開している、U2Fエクステンション（u2f-chrome-extension）をベースに、下記の通り若干の修正をしたものです。

* U2F BLEエクステンション（[u2f-ble-helper](../../u2f-ble-helper/README.md)）と連携するためのヘルパーコード（[blehelper.js](blehelper.js)）を新設

* UsbHelperの代わりに、BleHelper（blehelper.js内で定義されたクラス）を扱うよう修正

### オリジナルのREADME

こちらのページ（[README.original.md](README.original.md)）をご参照ください。

## 制限事項

* <b>YubiKeyが使用できなくなります</b>
<br>
本エクステンションは、U2F USBエクステンションの代わりに、U2F BLEエクステンション（[u2f-ble-helper](../../u2f-ble-helper/README.md)）を使用することを前提としたコードです。<br>
したがって、USBを使用したYubiKeyのようなデバイスとは、兼用はできません。

* <b>パッケージ化するとU2Fテストサイトが使用できません</b>
<br>
本エクステンションのコードをパッケージ化したエクステンション（.crx）をインストールすると、エクステンションのIDが変わってしまうため、[U2Fテストサイト](https://crxjs-dot-u2fdemo.appspot.com/)が指定したIDと一致しなくなり、結果認証テストが失敗します。<br>
このため、パッケージ化されていない状態で、Chromeブラウザーにインストールするようにします。


## 使用例

こちらのページ（[Chrome OSでのU2F認証テスト](../../Development/CHROMEOSTEST.md)）をご参照ください。
