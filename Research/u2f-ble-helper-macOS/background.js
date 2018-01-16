//
// BLE U2F Helper
//
// このエクステンションの呼出元(FIDO U2Fエクステンション)のID
var chromeU2FExtensionID = "pfboblefjcgdjicmnffhdgionmgcdmne";

// BLE U2F Helperホスト(U2F管理ツール)の情報を保持
var hostName = "jp.co.diverta.chrome.helper.ble.u2f";
var port = null;

// レスポンス返却時に呼び出されるコールバックを保持
var helperResponse = null;

// FIDO U2Fエクステンションから転送されるメッセージ種別
var HELPER_ENROLL_MSG_TYPE = "enroll_helper_request";
var HELPER_SIGN_MSG_TYPE   = "sign_helper_request";

//
// U2F管理ツールとの通信用関数群
//
function sendNativeMessage(messageJson) {
    if (port) {
        // FIDO U2Fエクステンションからの
        // メッセージをU2F管理ツールに転送
        port.postMessage(messageJson);
        console.log("Sent native message:", JSON.stringify(messageJson));
    }
}

function onNativeMessage(messageJson) {
    console.log("Received native message:", JSON.stringify(messageJson));

    if (helperResponse) {
        // U2F管理ツールからのメッセージを
        // FIDO U2Fエクステンションへ転送
        helperResponse(messageJson);
        helperResponse = null;
    }
}

function onDisconnected() {
    console.log("Failed to connect:", chrome.runtime.lastError.message);
    port = null;
}

function init() {
    console.log("sending notification registration to FIDO U2F extension");
    chrome.runtime.sendMessage(chromeU2FExtensionID, chrome.runtime.id);

    console.log('Hello, World! It is ' + new Date());

    port = chrome.runtime.connectNative(hostName);
    port.onMessage.addListener(onNativeMessage);
    port.onDisconnect.addListener(onDisconnected);
}

//
// FIDO U2Fエクステンションからメッセージを受信した時の処理
//
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
    }
);

function sendEnrollRequest(request, sendResponse){
    console.log("sending enroll request");
    sendNativeMessage(request, -1);
    helperResponse = sendResponse;
}

function sendSignRequest(request, sendResponse){
    if (request.signData.length > 1) {
        console.log('Batch authentication request not implemented yet');
        return;
    }
    console.log("sending sign request");
    sendNativeMessage(request, -1);
    helperResponse = sendResponse;
}
//
// Initialize Helper
//
console.log('background.js: loaded');
init();
console.log('background.js: init() done');
