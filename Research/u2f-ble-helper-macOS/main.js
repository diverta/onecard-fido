//FIDO Bluetooth UUIDs
var FIDO_U2F_SERVICE_UUID = "0000fffd-0000-1000-8000-00805f9b34fb";
var U2F_CONTROL_POINT_ID  = "f1d0fff1-deaa-ecee-b42f-c9ba7ed623bb";
var U2F_STATUS_ID  = "f1d0fff2-deaa-ecee-b42f-c9ba7ed623bb";
var CHARACTERISTIC_UPDATE_NOTIFICATION_DESCRIPTOR_UUID = "00002902-0000-1000-8000-00805f9b34fb";

var MAX_CHARACTERISTIC_LENGTH = 64;
var U2F_MESSAGE_TYPE = 0x83;

var ENABLE_NOTIFICATIONS = new ArrayBuffer(2);
var en_view = new Uint8Array(ENABLE_NOTIFICATIONS);
en_view[0] = 1;
en_view[1] = 0;

var HELPER_ENROLL_MSG_TYPE = "enroll_helper_request";
var HELPER_SIGN_MSG_TYPE = "sign_helper_request";
var authenticator;
var u2fService;
var u2fStatus;
var u2fControl;


var U2F_STATE_IDLE = 0;
var U2F_STATE_ENROLL = 1;
var U2F_STATE_SIGN = 2;
var U2F_STATE = U2F_STATE_IDLE;

var MESSAGE_STATE_WAITING_FOR_BITS = 0;
var MESSAGE_STATE_IDLE = 1;
var MESSAGE_STATE = MESSAGE_STATE_IDLE;
var byteIndex = 0;
var messageFromDevice;

var helperResponse;

var enroll_helper_reply = {
 "type":"enroll_helper_reply",
 "code": null,
 "version":"U2F_V2",
 "enrollData": null
};
     
var sign_helper_reply = {
 "type": "sign_helper_reply",
 "code": 0,  
 "responseData": {
   "version": "U2F_V2",
   "appIdHash": null,
   "challengeHash": null,
   "keyHandle": null,
   "signatureData": null
  }
};

var hostName = "com.google.chrome.example.echo";
var port = null;

function sendNativeMessage(textValue) {
    if (port) {
        message = {"text": textValue};
        port.postMessage(message);
        console.log("Sent native message:", JSON.stringify(message));
    }
}

function onNativeMessage(message) {
    console.log("Received native message:", JSON.stringify(message));
}

function onDisconnected() {
    console.log("Failed to connect:", chrome.runtime.lastError.message);
    port = null;
}

function init() {

    console.log("sending notification registration to FIDO U2F extension");
    chrome.runtime.sendMessage('pfboblefjcgdjicmnffhdgionmgcdmne', chrome.runtime.id);

    console.log('Hello, World! It is ' + new Date());

    port = chrome.runtime.connectNative(hostName);
    port.onMessage.addListener(onNativeMessage);
    port.onDisconnect.addListener(onDisconnected);
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

/*
    var enrollMessage = createEnrollCommand(request);
    sendMessageToAuthenticator(enrollMessage, -1);
*/
    // for research
    sendNativeMessage("sending enroll request to native");

    helperResponse = sendResponse;
}

function sendSignRequest(request, sendResponse){
    if (request.signData.length > 1) {
        console.log('Batch authentication request not implemented yet');
        return;
    }
    console.log("sending sign request");
    U2F_STATE = U2F_STATE_SIGN;

    sign_helper_reply.responseData.appIdHash = request.signData[0].appIdHash;
    sign_helper_reply.responseData.challengeHash = request.signData[0].challengeHash;
    sign_helper_reply.responseData.keyHandle = request.signData[0].keyHandle;
/*
    var signMessage = createSignCommand(request);
    sendMessageToAuthenticator(signMessage, -1);
*/
    helperResponse = sendResponse;
}
