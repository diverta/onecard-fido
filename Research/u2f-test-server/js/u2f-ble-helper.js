'use strict';

var u2fBleHelper = u2fBleHelper || {};

//
// Web Bluetooth APIで取得したBLEデバイス(BluetoothDevice)
//
//   BluetoothDeviceオブジェクトの参照を、変数bleU2fDeviceに保持。
//   このオブジェクト参照はChrome Extensionに渡すことが出来ない。
//   そのため、U2F JavaScript APIから呼び出されるExtensionの機能を、
//   すべてこのヘルパー・クラスに移設している。
//
var bleU2fDevice;
u2fBleHelper.setBleU2fDevice = function(device) {
    bleU2fDevice = device;
};

//
// U2F JavaScript APIから直接呼び出される関数群
//
u2fBleHelper.sendRegisterRequest = function(registerRequest) {
    console.log("u2fBleHelper.sendRegisterRequest", registerRequest);

    // TODO:
    // ここでBLE U2Fサービス(nRF52)とやり取りを行い、
    // U2F Register処理を実行する
    console.log("u2fBleHelper.sendRegisterRequest", bleU2fDevice);

    // U2F JavaScript APIに返却するレスポンスを格納
    //   リクエストID以外はDummy
    var responseData = {
        errorCode: 0,
        clientData: "",
        registrationData: ""
    };

    var reqId = registerRequest.requestId;
    var messageData = {
        requestId: reqId,
        responseData: responseData
    };

    var message = {
        data: messageData
    };

    // レスポンスをU2F JavaScript APIに返却
    return message;
}

u2fBleHelper.sendSignRequest = function(signRequest) {
    console.log("u2fBleHelper.sendSignRequest", signRequest);

    // TODO:
    // ここでBLE U2Fサービス(nRF52)とやり取りを行い、
    // U2F Authentication処理を実行する
    console.log("u2fBleHelper.sendSignRequest", bleU2fDevice);

    // U2F JavaScript APIに返却するレスポンスを格納
    //   リクエストID以外はDummy
    var responseData = {
        errorCode: 0,
        keyHandle: "",
        clientData: "",
        signatureData: ""
    };

    var reqId = signRequest.requestId;
    var messageData = {
        requestId: reqId,
        responseData: responseData
    };

    var message = {
        data: messageData
    };

    // レスポンスをU2F JavaScript APIに返却
    return message;
}