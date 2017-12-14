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

function init() {

  console.log("sending notification registration to FIDO U2F extension");
  chrome.runtime.sendMessage('pfboblefjcgdjicmnffhdgionmgcdmne', chrome.runtime.id);

  console.log('Hello, World! It is ' + new Date());

  //if there are any connected authenticators find one
  chrome.bluetooth.getDevices(function(devices){
    for(var i = 0; i < devices.length; i++){
      var device = devices[i];
      //if check for connected U2F Authenticator
      if(!device.uuids || device.uuids.indexOf(FIDO_U2F_SERVICE_UUID) < 0 || !device.connected){
        continue;
      }
      //got one
      chrome.bluetoothLowEnergy.connect(device.address, function () {
        if (chrome.runtime.lastError) {
          console.log('Failed to connect: ' + chrome.runtime.lastError.message);
          return;
        }
        //connection established
        console.log("connected to FIDO U2F authenticator");
        authenticator = device;
        chrome.bluetoothLowEnergy.getServices(authenticator.address, function(services){
          for(var i = 0; i < services.length; i++){
            console.log("BLE Service", services[i].uuid);
            if (services[i].uuid == FIDO_U2F_SERVICE_UUID){
                initializeService(services[i]);
                console.log("FIDO U2F service initialize start");
                break;
            }
          }
        });
      });
    }
  });
};

chrome.bluetooth.onDeviceChanged.addListener(
  function(device){
    if(!authenticator){
      return;
    }
    if((device.address == authenticator.address) && (device.connected === false)){
      console.log("authenticator has disconnected");
      authenticator = null;
      MESSAGE_STATE = MESSAGE_STATE_IDLE;
      U2F_STATE = U2F_STATE_IDLE;
      
    }
  });

chrome.bluetooth.onDeviceAdded.addListener(
  function(device){
    if (!device.uuids || device.uuids.indexOf(FIDO_U2F_SERVICE_UUID) < 0){
      return;
    }
    if(authenticator !== null){
      //return for now
      return;
    }
    console.log('found a FIDO U2F BLE authenticator - connecting!');

    chrome.bluetoothLowEnergy.connect(device.address, function () {
      if (chrome.runtime.lastError) {
        console.log('Failed to connect: ' + chrome.runtime.lastError.message);
        return;
      }
      //connection established
      console.log("connected to FIDO U2F authenticator");
      authenticator = device;
    });
});

// メッセージ受信時の処理
chrome.bluetoothLowEnergy.onCharacteristicValueChanged.addListener(function(characteristic){
    if (characteristic.uuid != U2F_STATUS_ID || !characteristic.value) {
      return;
    }
    var characteristicValue = new Uint8Array(characteristic.value);
    console.log('Received message from authenticator', unPackBinaryToHex(characteristic.value));

    var msg_view;
    if (MESSAGE_STATE == MESSAGE_STATE_IDLE && characteristicValue[0] == U2F_MESSAGE_TYPE) {
      // 分割受信１回目の場合
      //   受信予定データ長分、バッファ領域を確保し、
      //   今回受信データ(BLEヘッダー=先頭3バイトを削除)を
      //   バッファに格納
      var responseDataLength = characteristicValue[1] << 8;
      responseDataLength += (characteristicValue[2] & 0xFF);
      messageFromDevice = new ArrayBuffer(responseDataLength);
      msg_view = new Uint8Array(messageFromDevice);
      msg_view.set(characteristicValue.subarray(3));

      if (responseDataLength == 2) {
        // ステータスワード(2バイト)のみ受信した場合は、
        // エラーで戻ってきたと判断し、
        // ヘルパーにレスポンスを送信
        console.log("Error message received", unPackBinaryToHex(messageFromDevice));
        MESSAGE_STATE = MESSAGE_STATE_IDLE;
        sendErrorResponseToHelper();

      } else {
        // 分割受信が継続すると判断し、インデックスを更新
        byteIndex = characteristicValue.length - 3;
        MESSAGE_STATE = MESSAGE_STATE_WAITING_FOR_BITS;
      }

    } else if (MESSAGE_STATE == MESSAGE_STATE_WAITING_FOR_BITS) {
      // 分割受信２回目以降の場合
      //   今回受信データ(BLEヘッダー=先頭1バイトを削除)を
      //   バッファに格納
      msg_view = new Uint8Array(messageFromDevice);
      msg_view.set(characteristicValue.subarray(1), byteIndex);

      var dataLength = characteristicValue.length - 1;
      if (byteIndex + dataLength < msg_view.length) {
        // 取得済みバイト数+今回受信データ長が
        // 全体バイト数に達しないときは、
        // 分割受信が継続すると判断し、インデックスを更新
        byteIndex += dataLength;

      } else {
        // 分割受信の最後と判断し、ヘルパーにレスポンスを送信
        console.log("U2F message received", unPackBinaryToHex(messageFromDevice));
        MESSAGE_STATE = MESSAGE_STATE_IDLE;
        sendResponseToHelper();
      }
    }
});

function sendErrorResponseToHelper() {
  if (helperResponse) {
    // ステータスワードを取得
    var temp = new Uint8Array(messageFromDevice);
    var statusWord = (temp[0] << 8) + (temp[1] & 0xFF);

    // エラーレスポンスを送信
    if (U2F_STATE == U2F_STATE_ENROLL) {
      enroll_helper_reply.code = statusWord;
      enroll_helper_reply.enrollData = null;
      helperResponse(enroll_helper_reply);

    } else if(U2F_STATE == U2F_STATE_SIGN){
      sign_helper_reply.code = statusWord;
      sign_helper_reply.responseData = null;
      helperResponse(sign_helper_reply);
    }
    helperResponse = null;
  }

  messageFromDevice = null;
  U2F_STATE = U2F_STATE_IDLE;
}

function sendResponseToHelper(){
  // 受信レスポンスからステータスワード(末尾2バイト)を削除後
  // B64エンコード
  var messageWithoutSW = messageFromDevice.slice(0, messageFromDevice.byteLength - 2);
  var b64 = B64_encode(new Uint8Array(messageWithoutSW));

  var replyData = undefined;
  if (U2F_STATE == U2F_STATE_ENROLL) {
    console.log("base64 websafe enroll data", b64);
    enroll_helper_reply.enrollData = b64;
    enroll_helper_reply.code = 0;
    replyData = enroll_helper_reply;
    console.log("sending enroll response back to chrome extension");

  } else if (U2F_STATE == U2F_STATE_SIGN) {
    console.log("base64 websafe sign data", b64);
    sign_helper_reply.responseData.signatureData = b64;
    sign_helper_reply.code = 0;
    replyData = sign_helper_reply;
    console.log("sending sign response back to chrome extension");
  }

  // レスポンスを送信
  if (helperResponse) {
    if (replyData !== undefined) {
      helperResponse(replyData);
    }
    helperResponse = null;
  }

  messageFromDevice = null;
  U2F_STATE = U2F_STATE_IDLE;
}

function initializeService(service){
  if (service === null) {
    console.log("u2f service disconnect");
    u2fService = null;
    return;
  }

  u2fService = service;
  chrome.bluetoothLowEnergy.getCharacteristics(u2fService.instanceId, function(characteristics){
    if (characteristics === undefined){
      console.log('u2f status characteristic is undefined: ' + chrome.runtime.lastError.message);
      return;
    }

    for (var i = 0; i < characteristics.length; i++) {
      if (characteristics[i].uuid == U2F_STATUS_ID) {
        u2fStatus = characteristics[i];
      } else if(characteristics[i].uuid == U2F_CONTROL_POINT_ID) {
        u2fControl = characteristics[i];
      }
    }

    if (u2fStatus !== null) {
      chrome.bluetoothLowEnergy.startCharacteristicNotifications(u2fStatus.instanceId, function(){
        if (chrome.runtime.lastError) {
          console.log('failed to enable notifications for u2f status characteristic: ' + chrome.runtime.lastError.message);
          console.log("FIDO U2F service initialize abend");
          return;
        }
        console.log("notifications set up on u2f status characteristic");
        console.log("FIDO U2F service initialize end");
      });
    }
  });
}

chrome.bluetoothLowEnergy.onServiceAdded.addListener(function(service) {
  if (service.uuid == FIDO_U2F_SERVICE_UUID) {
    initializeService(service);
  }
});

chrome.bluetoothLowEnergy.onServiceRemoved.addListener(function(service) {
  if (service.uuid == FIDO_U2F_SERVICE_UUID) {
    initializeService(null);
  }
});

chrome.runtime.onMessageExternal.addListener(
  function(request, sender, sendResponse) {
    console.log("got a message from the extenstion", JSON.stringify(request));
    if(request.type == HELPER_ENROLL_MSG_TYPE){
      sendEnrollRequest(request, sendResponse);
    }
    else if(request.type == HELPER_SIGN_MSG_TYPE){
      sendSignRequest(request, sendResponse);
    }
    else{
      console.log("unknown request type sent by FIDO extension");
    }
    
    //returning true will allow for the asynchronous calling of the sendResponse function
    return true;
});

// メッセージ送信時の処理
function sendMessageToAuthenticator(message, sequence){
  if(!message || message.byteLength === 0){
    return;
  }

  var data_view =  new Uint8Array(message);
  var data_view_max_length = undefined;
  var messageSegment = undefined;
  var messageTemp = undefined;

  if (sequence == -1) {
    data_view_max_length = MAX_CHARACTERISTIC_LENGTH;
    if (data_view.length > data_view_max_length) {
      // 分割１回目の場合で、データ長が64バイトをこえる場合は
      // 64バイトだけを送信し、残りのデータを継続送信
      messageSegment = new Uint8Array(message.slice(0, data_view_max_length));

    } else {
      // 分割１回目の場合で、データ長が64バイト以下の場合は
      // そのまま送信して終了
      messageSegment = new Uint8Array(message);
    }

  } else {
    data_view_max_length = MAX_CHARACTERISTIC_LENGTH - 1
    if (data_view.length > data_view_max_length) {
      // 分割２回目以降の場合で、データ長が63バイトを超える場合、
      // 63バイトだけを送信し、残りのデータを継続送信
      messageTemp = new Uint8Array(message.slice(0, data_view_max_length));

    } else {
      // 分割２回目以降の場合で、データ長が63バイト以下の場合は
      // そのまま送信して終了
      messageTemp = new Uint8Array(message);
    }

    // 先頭にシーケンスを付加
    var seq = new Uint8Array([sequence]);
    var len = seq.length + messageTemp.length;
    var u8 = new Uint8Array(len);
    u8.set(seq);
    u8.set(messageTemp, seq.length);
    messageSegment = u8.buffer;
  }

  console.log("Writing message to authenticator", unPackBinaryToHex(messageSegment));

  if (data_view.length > data_view_max_length) {
    chrome.bluetoothLowEnergy.writeCharacteristicValue(u2fControl.instanceId, messageSegment, function() {
      if (chrome.runtime.lastError) {
        console.log('Failed to write message: ' + chrome.runtime.lastError.message);
        return;
      }
      sendMessageToAuthenticator(message.slice(data_view_max_length), ++sequence);
    });

  } else {
    chrome.bluetoothLowEnergy.writeCharacteristicValue(u2fControl.instanceId, messageSegment, function() {
      console.log('Complete message to authenticator has been sent!');
    });
  }
}

function sendEnrollRequest(request, sendResponse){
    console.log("sending enroll request");
    U2F_STATE = U2F_STATE_ENROLL;
    var enrollMessage = createEnrollCommand(request);
    sendMessageToAuthenticator(enrollMessage, -1);
    helperResponse = sendResponse;
}

function sendSignRequest(request, sendResponse){
    console.log("sending sign request");
    if(request.signData.length > 1){
      console.log('Batch authentication request not implemented yet');
      return;
    }
    U2F_STATE = U2F_STATE_SIGN;
    sign_helper_reply.responseData.appIdHash = request.signData[0].appIdHash;
    sign_helper_reply.responseData.challengeHash = request.signData[0].challengeHash;
    sign_helper_reply.responseData.keyHandle = request.signData[0].keyHandle;
    var signMessage = createSignCommand(request);
    sendMessageToAuthenticator(signMessage, -1);
    helperResponse = sendResponse;
}
