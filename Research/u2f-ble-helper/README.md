# U2F BLEエクステンション

Chrome OSに導入されたChromeブラウザーで、One CardのBLE U2Fサービスを利用してU2F認証ができるよう対応したChromeエクステンションです。

U2Fエクステンション「FIDO U2F (Universal 2nd Factor) extension」をさらに拡張する、子エクステンションとして使用します。
<br>
（「FIDO U2F (Universal 2nd Factor) extension」は、Googleから公開されているエクステンションです）

U2Fエクステンションについては、こちらのページ（[u2f-chrome-extension](../u2f-ref-code/u2f-chrome-extension/README.md)）をご参照ください。

## このコードについて

有志の方が[こちらのページ](https://github.com/carybran/u2f-ble-helper)で公開している、BLE U2F対応用のBLEエクステンションをベースに、下記の通り若干の修正をしたものです。

* バックグラウンド・スクリプトでHTMLを開かないよう修正
* BLE U2FサービスのUUIDが間違っていたので修正
* Characteristicの最大長を64バイトに修正
* 権限がない場合はスクリプトを途中終了させるよう修正
* init()が２重に呼び出されないよう修正
* console.logを複数箇所に追加
* Leバイト（メッセージ末尾の２バイト分）を追加
* U2F_V2の分割送受信に対応
* 処理対象メッセージを全てUint8Arrayオブジェクトに寄せるよう修正

## テスト用BLEエクステンション

このコードからChromeブラウザーを使用してパッケージ化したバイナリーファイルを用意しました。

こちらからダウンロードできます。<br>
[u2f-ble-helper.crx](../u2f-ble-helper.crx)

## 使用例

こちらのページ（[Chrome OSでのU2F認証テスト](../../Research/CHROMEOSTEST.md)）をご参照ください。
