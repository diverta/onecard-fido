/**
 * @fileoverview Implements a helper using BLE Authenticator.
 */
'use strict';

// BLE U2F Helperホスト(U2F管理ツール)の情報を保持
var hostName = "jp.co.diverta.chrome.helper.ble.u2f";
var port = null;

// レスポンス返却時に呼び出されるコールバックを保持
var helperResponse = null;

//
// U2F管理ツールとの通信用関数群
//
function sendNativeMessage(messageJson) {
    if (port) {
        port.postMessage(messageJson);
        console.log("Sent native message:", JSON.stringify(messageJson));
    }
}

function onNativeMessage(messageJson) {
    console.log("Received native message:", JSON.stringify(messageJson));

    if (helperResponse) {
        helperResponse(messageJson);
        helperResponse = null;
    }
}

function onDisconnected() {
    console.log("Failed to connect:", chrome.runtime.lastError.message);
    port = null;
}

function initNativeHelper() {
    port = chrome.runtime.connectNative(hostName);
    port.onMessage.addListener(onNativeMessage);
    port.onDisconnect.addListener(onDisconnected);

    console.log('BleHelper initialized:' + new Date());
}

/**
 * @constructor
 * @extends {GenericRequestHelper}
 */
function BleHelper() {
  GenericRequestHelper.apply(this, arguments);
  initNativeHelper();

  var self = this;
  this.registerHandlerFactory('enroll_helper_request', function(request) {
    return new BleEnrollHandler(/** @type {EnrollHelperRequest} */ (request));
  });
  this.registerHandlerFactory('sign_helper_request', function(request) {
    return new BleSignHandler(/** @type {SignHelperRequest} */ (request));
  });
}

inherits(BleHelper, GenericRequestHelper);

function BleEnrollHandler(request) {
  this.request_  = request;
  this.callback_ = null;
}

BleEnrollHandler.prototype.run = function(callback) {
  this.callback_ = callback;
  console.log('BleEnrollHandler: request=', this.request_);

  // FIDO U2Fエクステンションからの
  // メッセージをU2F管理ツールに転送
  helperResponse = this.onEnrollHelperReplyed.bind(this);
  console.log("sending enroll request");
  sendNativeMessage(this.request_, -1);
  return true;
};

BleEnrollHandler.prototype.onEnrollHelperReplyed = function(response) {
  console.log('BleEnrollHandler: response=', response);

  // U2F管理ツールからのメッセージを
  // FIDO U2Fエクステンションへ転送
  if (response === undefined) {
    console.warn('Ble helper extension not replyed');
    this.callback_({
      'type': 'enroll_helper_reply',
      'code': DeviceStatusCodes.INVALID_DATA_STATUS,
      'version': 'U2F_V2',
      'enrollData': B64_encode("")
    });
  } else {
    this.callback_(response);
  }
};

BleEnrollHandler.prototype.close = function() {
};


function BleSignHandler(request) {
  this.request_  = request;
  this.callback_ = null;
}

BleSignHandler.prototype.run = function(callback) {
  this.callback_ = callback;
  console.log('BleSignHandler: request=', this.request_);

  // signDataが複数の場合はスキップ
  if (this.request_.signData.length > 1) {
      console.log('Batch authentication request not implemented yet');
      return true;
  }

  // FIDO U2Fエクステンションからの
  // メッセージをU2F管理ツールに転送
  helperResponse = this.onSignHelperReplyed.bind(this);
  console.log("sending sign request");
  sendNativeMessage(this.request_, -1);
  return true;
};

BleSignHandler.prototype.onSignHelperReplyed = function(response) {
  console.log('BleSignHandler: response=', response);

  // U2F管理ツールからのメッセージを
  // FIDO U2Fエクステンションへ転送
  if (response === undefined) {
    console.warn('Ble helper extension not replyed');
    this.callback_({
      'type': 'sign_helper_reply',
      'code': DeviceStatusCodes.INVALID_DATA_STATUS,
      'version': 'U2F_V2',
      'signData': B64_encode("")
    });
  } else {
    this.callback_(response);
  }
};

BleSignHandler.prototype.close = function() {
};
