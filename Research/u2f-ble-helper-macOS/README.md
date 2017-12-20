# [WIP] Chrome(macOS版)でのBLE U2F対応調査

## 概要

macOS版Chromeブラウザーで、BLE U2F対応が可能かどうか調査します。

具体的にはmacOSでサポートされているChromeの「Web Bluetooth API」を使用し、BLE通信が可能なU2Fクライアントを作成します。<br>
後述の２案をもとに、BLE通信が可能なU2Fクライアントの実装調査をすすめます。

## 参考文献
下記のプロジェクトを参考にします。

* Heart Rate Sensor Demo<br>
https://github.com/WebBluetoothCG/demos/tree/gh-pages/heart-rate-sensor

* Web Bluetooth Samples (Chrome) <br>
https://github.com/googlechrome/samples/tree/gh-pages/web-bluetooth

## 実装案調査（その１：エクステンション実装）

Chromeブラウザーの既存U2Fエクステンションを流用して対応する方法になります。

* メリット - すでに実装／テストが完了しているプログラムがベースである
* デメリット - 制御方法が複雑になってしまう

下記に検討内容を掲載します。

### エクステンション実装の制約

エクステンションから、Web Bluetooth APIを呼び出そうとすると、権限エラーで処理が先に進まなくなることが確認されています。<br>
下記コードを使用して確認しました。

```
function init() {

  console.log("sending notification registration to FIDO U2F extension");
  chrome.runtime.sendMessage('pfboblefjcgdjicmnffhdgionmgcdmne', chrome.runtime.id);

  console.log('Hello, World! It is ' + new Date());

};

chrome.runtime.onMessageExternal.addListener(
  function(request, sender, sendResponse) {
    console.log("got a message from the extenstion", JSON.stringify(request));

    if (request.type == HELPER_ENROLL_MSG_TYPE) {
      sendEnrollRequest(request, sendResponse);

    } else if (request.type == HELPER_SIGN_MSG_TYPE) {
      sendSignRequest(request, sendResponse);

    } else {
      console.log("unknown request type sent by FIDO extension");
    }

    return true;
});

function sendEnrollRequest(request, sendResponse){
    console.log("sending enroll request");
    U2F_STATE = U2F_STATE_ENROLL;

    if (bleU2fDevice == undefined) {
        bleU2fDevice =
        navigator.bluetooth.requestDevice({acceptAllDevices:true})
        .then(device => console.log(device))
        .catch(error => console.log(error));
    }
    helperResponse = sendResponse;
}
```

macOS版Chromeのデベロッパー・ツールに出力されたログは以下の通りです。

```
background.js:7 background.js: loaded
main.js:59 sending notification registration to FIDO U2F extension
main.js:62 Hello, World! It is Wed Dec 20 2017 12:32:18 GMT+0900 (JST)
background.js:9 background.js: init() done
main.js:68 got a message from the extenstion {"type":"enroll_helper_request","enrollChallenges":[{"version":"U2F_V2","challengeHash":"Ou5tb4WzlPwvfN2tztDPH-4ZkZu-aSdVKhvQ_tEtQIY","appIdHash":"MencZvKI5fpF6xulpivSP99k-9avciQy0M3BmUv1ea0"}],"signData":[],"timeout":599,"timeoutSeconds":599}
main.js:84 sending enroll request
main.js:91 DOMException: Must be handling a user gesture to show a permission request.
```
navigator.bluetooth.requestDevice呼び出し時、メッセージ `DOMException: Must be handling a user gesture to show a permission request.` が出力されエラーとなることが確認できます。


### エクステンション実装の工夫

そこで、既存のU2Fエクステンションで生成されたU2F RAWリクエスト／レスポンスデータを、いったん呼び出し元のJavaScript（＝ボタンスクリプト）に戻して、ボタンスクリプトからBLE通信させる方法を取るよう、実装を工夫します。

#### 今までの実装

chrome.bluetoothLowEnergyを使用したChrome OS版での実装は、概ね下記のようになっています。

* リクエスト<br>
Chromeのボタンスクリプト--->[U2Fリクエストデータ]--->U2F BLEエクステンション--->[U2F RAWリクエストデータ]--->nRF52

* レスポンス<br>
nRF52--->[U2F RAWレスポンスデータ]--->U2F BLEエクステンション--->[U2Fレスポンスデータ]--->Chromeのボタンスクリプト

#### 対応後の実装

macOS版での実装は、navigator.bluetoothを使用するため、概ね下記のような実装にする必要があります。

* リクエスト<br>
Chromeのボタンスクリプト--->[U2Fリクエストデータ]--->U2F BLEエクステンション--->[U2F RAWリクエストデータ]--->Chromeのボタンスクリプト--->[U2F RAWリクエストデータ]--->nRF52

* レスポンス<br>
nRF52--->[U2F RAWレスポンスデータ]--->Chromeのボタンスクリプト--->[U2F RAWリクエストデータ]--->U2F BLEエクステンション--->[U2Fレスポンスデータ]--->Chromeのボタンスクリプト

#### 既存のU2Fエクステンションを流用する理由

既存のU2Fエクステンションは、U2Fクライアントに相当する部分です。<br>
したがって、U2F RAWリクエスト／レスポンスデータを扱うための複雑な処理が、すでに実装されています。

U2Fクライアントを１から作成する必要がないのと、処理の精度を担保させるため、既存のU2Fエクステンションを再利用する方向とします。

## 実装案調査（その２：ボタンスクリプト実装）

Chromeブラウザー上で動作するボタンスクリプト（＝サーバー側に用意したJavaScript）に、U2Fクライアント処理を新規追加して対応する方法になります。

* メリット - U2Fクライアント機能が集約されるので、処理の制御関係がわかりやすい（エクステンションが不要）
* デメリット - U2FクライアントのJavaScriptを新規開発して、サーバー側に用意する必要がある

下記に検討内容を掲載します。

### サーバーアプリケーションを作成

調査用／開発用に使用するための、サーバーアプリケーションを作成します。

以下の実装が参考になります。<br>
U2F-GAE-Demo：https://github.com/google/u2f-ref-code#u2f-gae-demo

詳細は後報します。

### テスト用ボタンスクリプトを作成

まずは、navigator.bluetoothを使用して、nRF52をディスカバーするボタンスクリプトを作成し、動作確認をします。

詳細は後報します。

### 実装済みU2Fクライアント処理の移植

エクステンションに実装されているU2Fクライアント処理を、ボタンスクリプトから呼び出せるJavaScriptに移設します。

詳細は後報します。
