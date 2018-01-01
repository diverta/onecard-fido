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

    var registerRequests = registerRequest['registerRequests'];
    var appId = registerRequest['appId'];
    var encodedEnrollChallenges =
        encodeEnrollChallenges_(registerRequests, appId);
    var request = {
        type: 'enroll_helper_request',
        enrollChallenges: encodedEnrollChallenges
    };
    console.log("u2fBleHelper.sendRegisterRequest", request);

    var enrollMessage = createEnrollCommand(request);
    sendMessageToAuthenticator(enrollMessage, -1);

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


//
// 下位の関数
//
//
// from u2f-ble-helper/main.js
//
var FIDO_U2F_SERVICE_UUID = "0000fffd-0000-1000-8000-00805f9b34fb";
var U2F_CONTROL_POINT_ID  = "f1d0fff1-deaa-ecee-b42f-c9ba7ed623bb";
var U2F_STATUS_ID         = "f1d0fff2-deaa-ecee-b42f-c9ba7ed623bb";
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
//
// BluetoothDeviceを使用し、
// nRF52へ、メッセージを送信
function sendInitMessageToAuthenticator(
    message, message_length, segment_max_length) {
    if (message_length > segment_max_length) {
        // 分割１回目の場合で、データ長が64バイトをこえる場合は
        // 64バイトだけを送信し、残りのデータを継続送信
        return new Uint8Array(message.slice(0, segment_max_length));
    } else {
        // 分割１回目の場合で、データ長が64バイト以下の場合は
        // そのまま送信して終了
        return new Uint8Array(message);
    }
}
function sendContMessageToAuthenticator(
    message, message_length, segment_max_length, sequence) {
    var message_temp = undefined;
    if (message_length > segment_max_length) {
        // 分割２回目以降の場合で、データ長が63バイトを超える場合、
        // 63バイトだけを送信し、残りのデータを継続送信
        message_temp = new Uint8Array(message.slice(0, segment_max_length));
    } else {
        // 分割２回目以降の場合で、データ長が63バイト以下の場合は
        // そのまま送信して終了
        message_temp = new Uint8Array(message);
    }

    // 先頭にシーケンスを付加
    var seq = new Uint8Array([sequence]);
    var len = seq.length + message_temp.length;
    var u8  = new Uint8Array(len);
    u8.set(seq);
    u8.set(message_temp, seq.length);

    return u8.buffer;
}
function sendMessageToAuthenticator(message, sequence) {
    if (!message || message.byteLength === 0) {
        return;
    }

    var data_view =  new Uint8Array(message);
    var message_length = data_view.length;
    var message_segment = undefined;
    var segment_max_length = undefined;
    console.log("u2fBleHelper.sendMessageToAuthenticator", data_view);

    if (sequence == -1) {
        // 分割送信の先頭メッセージを作成
        segment_max_length = MAX_CHARACTERISTIC_LENGTH;
        message_segment = sendInitMessageToAuthenticator(
            message, message_length, segment_max_length);

    } else {
        // 分割送信の２番目以降のメッセージを作成
        segment_max_length = MAX_CHARACTERISTIC_LENGTH - 1
        message_segment = sendContMessageToAuthenticator(
            message, message_length, segment_max_length, sequence);
    }
    console.log("Writing message to authenticator", 
        unPackBinaryToHex(message_segment));

    // U2F Control Pointに書き込む
    if (message_length > segment_max_length) {
//        chrome.bluetoothLowEnergy.writeCharacteristicValue(
//            u2fControl.instanceId, message_segment, 
//            function() {
//                if (chrome.runtime.lastError) {
//                    console.log('Failed to write message: ' + chrome.runtime.lastError.message);
//                    return;
//                }
                sendMessageToAuthenticator(message.slice(segment_max_length), ++sequence);
//            }
//        );
    } else {
//        chrome.bluetoothLowEnergy.writeCharacteristicValue(
//            u2fControl.instanceId, message_segment, 
//            function() {
                console.log('Complete message to authenticator has been sent!');
//            }
//        );
    }
}

//
// from u2fbackground.js
//
/** @const */
var BROWSER_SUPPORTS_TLS_CHANNEL_ID = true;

function encodeEnrollChallenges_(enrollChallenges, opt_appId) {
  var challenges = [];
  for (var i = 0; i < enrollChallenges.length; i++) {
    var enrollChallenge = enrollChallenges[i];
    var version = enrollChallenge.version;
    if (!version) {
      // Version is implicitly V1 if not specified.
      version = 'U2F_V1';
    }

    if (version == 'U2F_V2') {
      var modifiedChallenge = {};
      for (var k in enrollChallenge) {
        modifiedChallenge[k] = enrollChallenge[k];
      }
      // V2 enroll responses contain signatures over a browser data object,
      // which we're constructing here. The browser data object contains, among
      // other things, the server challenge.
      var serverChallenge = enrollChallenge['challenge'];
      var browserData = makeEnrollBrowserData(
          //serverChallenge, this.sender_.origin, this.sender_.tlsChannelId);
          serverChallenge, opt_appId, '');
      // Replace the challenge with the hash of the browser data.
      modifiedChallenge['challenge'] =
          B64_encode(sha256HashOfString(browserData));
      /*
      this.browserData_[version] =
          B64_encode(UTIL_StringToBytes(browserData));
      */
      challenges.push(encodeEnrollChallenge_(
          /** @type {EnrollChallenge} */ (modifiedChallenge), opt_appId));
    } else {
      challenges.push(
          encodeEnrollChallenge_(enrollChallenge, opt_appId));
    }
  }
  return challenges;
}

function encodeEnrollChallenge_(enrollChallenge, opt_appId) {
  var encodedChallenge = {};
  var version;
  if (enrollChallenge['version']) {
    version = enrollChallenge['version'];
  } else {
    // Version is implicitly V1 if not specified.
    version = 'U2F_V1';
  }
  encodedChallenge['version'] = version;
  encodedChallenge['challengeHash'] = enrollChallenge['challenge'];
  var appId;
  if (enrollChallenge['appId']) {
    appId = enrollChallenge['appId'];
  } else {
    appId = opt_appId;
  }
  /*
  if (!appId) {
    // Sanity check. (Other code should fail if it's not set.)
    console.warn(UTIL_fmt('No appId?'));
  }
  */
  encodedChallenge['appIdHash'] = B64_encode(sha256HashOfString(appId));
  return /** @type {EnrollHelperChallenge} */ (encodedChallenge);
}


//
// from b64.js
//
// Copyright 2014 Google Inc. All rights reserved
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// WebSafeBase64Escape and Unescape.
function B64_encode(bytes, opt_length) {
  if (!opt_length) opt_length = bytes.length;
  var b64out =
      'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_';
  var result = '';
  var shift = 0;
  var accu = 0;
  var inputIndex = 0;
  while (opt_length--) {
    accu <<= 8;
    accu |= bytes[inputIndex++];
    shift += 8;
    while (shift >= 6) {
      var i = (accu >> (shift - 6)) & 63;
      result += b64out.charAt(i);
      shift -= 6;
    }
  }
  if (shift) {
    accu <<= 8;
    shift += 8;
    var i = (accu >> (shift - 6)) & 63;
    result += b64out.charAt(i);
  }
  return result;
}

// Normal base64 encode; not websafe, including padding.
function base64_encode(bytes, opt_length) {
  if (!opt_length) opt_length = bytes.length;
  var b64out =
      'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
  var result = '';
  var shift = 0;
  var accu = 0;
  var inputIndex = 0;
  while (opt_length--) {
    accu <<= 8;
    accu |= bytes[inputIndex++];
    shift += 8;
    while (shift >= 6) {
      var i = (accu >> (shift - 6)) & 63;
      result += b64out.charAt(i);
      shift -= 6;
    }
  }
  if (shift) {
    accu <<= 8;
    shift += 8;
    var i = (accu >> (shift - 6)) & 63;
    result += b64out.charAt(i);
  }
  while (result.length % 4) result += '=';
  return result;
}

var B64_inmap =
[
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 63, 0, 0,
 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 0, 0, 0, 0, 0, 0,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 0, 0, 0, 0, 64,
  0, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 0, 0, 0, 0, 0
];

function B64_decode(string) {
  var bytes = [];
  var accu = 0;
  var shift = 0;
  for (var i = 0; i < string.length; ++i) {
    var c = string.charCodeAt(i);
    if (c < 32 || c > 127 || !B64_inmap[c - 32]) return [];
    accu <<= 6;
    accu |= (B64_inmap[c - 32] - 1);
    shift += 6;
    if (shift >= 8) {
      bytes.push((accu >> (shift - 8)) & 255);
      shift -= 8;
    }
  }
  return bytes;
}

//
// from sha256.js
//
// Copyright 2014 Google Inc. All rights reserved
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// SHA256 in javascript.
//
// SHA256 {
//  SHA256();
//  void reset();
//  void update(byte[] data, opt_length);
//  byte[32] digest();
// }

/** @constructor */
function SHA256() {
  this._buf = new Array(64);
  this._W = new Array(64);
  this._pad = new Array(64);
  this._k = [
   0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
   0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
   0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
   0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
   0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
   0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
   0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
   0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
   0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2];

  this._pad[0] = 0x80;
  for (var i = 1; i < 64; ++i) this._pad[i] = 0;

  this.reset();
}

/** Reset the hasher */
SHA256.prototype.reset = function() {
  this._chain = [
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19];

  this._inbuf = 0;
  this._total = 0;
};

/** Hash the next block of 64 bytes
 * @param {Array<number>} buf A 64 byte buffer
 */
SHA256.prototype._compress = function(buf) {
  var W = this._W;
  var k = this._k;

  function _rotr(w, r) { return ((w << (32 - r)) | (w >>> r)); };

  // get 16 big endian words
  for (var i = 0; i < 64; i += 4) {
    var w = (buf[i] << 24) |
            (buf[i + 1] << 16) |
            (buf[i + 2] << 8) |
            (buf[i + 3]);
    W[i / 4] = w;
  }

  // expand to 64 words
  for (var i = 16; i < 64; ++i) {
    var s0 = _rotr(W[i - 15], 7) ^ _rotr(W[i - 15], 18) ^ (W[i - 15] >>> 3);
    var s1 = _rotr(W[i - 2], 17) ^ _rotr(W[i - 2], 19) ^ (W[i - 2] >>> 10);
    W[i] = (W[i - 16] + s0 + W[i - 7] + s1) & 0xffffffff;
  }

  var A = this._chain[0];
  var B = this._chain[1];
  var C = this._chain[2];
  var D = this._chain[3];
  var E = this._chain[4];
  var F = this._chain[5];
  var G = this._chain[6];
  var H = this._chain[7];

  for (var i = 0; i < 64; ++i) {
    var S0 = _rotr(A, 2) ^ _rotr(A, 13) ^ _rotr(A, 22);
    var maj = (A & B) ^ (A & C) ^ (B & C);
    var t2 = (S0 + maj) & 0xffffffff;
    var S1 = _rotr(E, 6) ^ _rotr(E, 11) ^ _rotr(E, 25);
    var ch = (E & F) ^ ((~E) & G);
    var t1 = (H + S1 + ch + k[i] + W[i]) & 0xffffffff;

    H = G;
    G = F;
    F = E;
    E = (D + t1) & 0xffffffff;
    D = C;
    C = B;
    B = A;
    A = (t1 + t2) & 0xffffffff;
  }

  this._chain[0] += A;
  this._chain[1] += B;
  this._chain[2] += C;
  this._chain[3] += D;
  this._chain[4] += E;
  this._chain[5] += F;
  this._chain[6] += G;
  this._chain[7] += H;
};

/** Update the hash with additional data
 * @param {Array<number>|Uint8Array} bytes The data
 * @param {number=} opt_length How many bytes to hash, if not all */
SHA256.prototype.update = function(bytes, opt_length) {
  if (!opt_length) opt_length = bytes.length;

  this._total += opt_length;
  for (var n = 0; n < opt_length; ++n) {
    this._buf[this._inbuf++] = bytes[n];
    if (this._inbuf == 64) {
      this._compress(this._buf);
      this._inbuf = 0;
    }
  }
};

/** Update the hash with a specified range from a data buffer
 * @param {Array<number>} bytes The data buffer
 * @param {number} start Starting index of the range in bytes
 * @param {number} end End index, will not be included in range
 */
SHA256.prototype.updateRange = function(bytes, start, end) {
  this._total += (end - start);
  for (var n = start; n < end; ++n) {
    this._buf[this._inbuf++] = bytes[n];
    if (this._inbuf == 64) {
      this._compress(this._buf);
      this._inbuf = 0;
    }
  }
};

/**
 * Optionally update the hash with additional arguments, and return the
 * resulting hash value.
 * @param {...*} var_args Data buffers to hash
 * @return {Array<number>} the SHA256 hash value.
 */
SHA256.prototype.digest = function(var_args) {
  for (var i = 0; i < arguments.length; ++i)
    this.update(arguments[i]);

  var digest = new Array(32);
  var totalBits = this._total * 8;

  // add pad 0x80 0x00*
  if (this._inbuf < 56)
    this.update(this._pad, 56 - this._inbuf);
  else
    this.update(this._pad, 64 - (this._inbuf - 56));

  // add # bits, big endian
  for (var i = 63; i >= 56; --i) {
    this._buf[i] = totalBits & 255;
    totalBits >>>= 8;
  }

  this._compress(this._buf);

  var n = 0;
  for (var i = 0; i < 8; ++i)
    for (var j = 24; j >= 0; j -= 8)
      digest[n++] = (this._chain[i] >> j) & 255;

  return digest;
};


//
// from util.js
//
/**
 * Converts a string to an array of bytes.
 * @param {string} s The string to convert.
 * @param {(Array|Uint8Array)=} bytes The Array-like object into which to store
 *     the bytes. A new Array will be created if not provided.
 * @return {(Array|Uint8Array)} An array of bytes representing the string.
 */
function UTIL_StringToBytes(s, bytes) {
  bytes = bytes || new Array(s.length);
  for (var i = 0; i < s.length; ++i)
    bytes[i] = s.charCodeAt(i);
  return bytes;
}


//
// from webrequest.js
//
// Copyright 2014 Google Inc. All rights reserved
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

/**
 * @fileoverview Does common handling for requests coming from web pages and
 * routes them to the provided handler.
 */

/**
 * FIDO U2F Javascript API Version
 * @const
 * @type {number}
 */
var JS_API_VERSION = 1.1;

/**
 * Gets the scheme + origin from a web url.
 * @param {string} url Input url
 * @return {?string} Scheme and origin part if url parses
 */
function getOriginFromUrl(url) {
  var re = new RegExp('^(https?://)[^/]*/?');
  var originarray = re.exec(url);
  if (originarray == null) return originarray;
  var origin = originarray[0];
  while (origin.charAt(origin.length - 1) == '/') {
    origin = origin.substring(0, origin.length - 1);
  }
  if (origin == 'http:' || origin == 'https:')
    return null;
  return origin;
}

/**
 * Returns whether the registered key appears to be valid.
 * @param {Object} registeredKey The registered key object.
 * @param {boolean} appIdRequired Whether the appId property is required on
 *     each challenge.
 * @return {boolean} Whether the object appears valid.
 */
function isValidRegisteredKey(registeredKey, appIdRequired) {
  if (appIdRequired && !registeredKey.hasOwnProperty('appId')) {
    return false;
  }
  if (!registeredKey.hasOwnProperty('keyHandle'))
    return false;
  if (registeredKey['version']) {
    if (registeredKey['version'] != 'U2F_V1' &&
        registeredKey['version'] != 'U2F_V2') {
      return false;
    }
  }
  return true;
}

/**
 * Returns whether the array of registered keys appears to be valid.
 * @param {Array<Object>} registeredKeys The array of registered keys.
 * @param {boolean} appIdRequired Whether the appId property is required on
 *     each challenge.
 * @return {boolean} Whether the array appears valid.
 */
function isValidRegisteredKeyArray(registeredKeys, appIdRequired) {
  return registeredKeys.every(function(key) {
    return isValidRegisteredKey(key, appIdRequired);
  });
}

/**
 * Gets the sign challenges from the request. The sign challenges may be the
 * U2F 1.0 variant, signRequests, or the U2F 1.1 version, registeredKeys.
 * @param {Object} request The request.
 * @return {!Array<SignChallenge>|undefined} The sign challenges, if found.
 */
function getSignChallenges(request) {
  if (!request) {
    return undefined;
  }
  var signChallenges;
  if (request.hasOwnProperty('signRequests')) {
    signChallenges = request['signRequests'];
  } else if (request.hasOwnProperty('registeredKeys')) {
    signChallenges = request['registeredKeys'];
  }
  return signChallenges;
}

/**
 * Returns whether the array of SignChallenges appears to be valid.
 * @param {Array<SignChallenge>} signChallenges The array of sign challenges.
 * @param {boolean} challengeValueRequired Whether each challenge object
 *     requires a challenge value.
 * @param {boolean} appIdRequired Whether the appId property is required on
 *     each challenge.
 * @return {boolean} Whether the array appears valid.
 */
function isValidSignChallengeArray(signChallenges, challengeValueRequired,
    appIdRequired) {
  for (var i = 0; i < signChallenges.length; i++) {
    var incomingChallenge = signChallenges[i];
    if (challengeValueRequired &&
        !incomingChallenge.hasOwnProperty('challenge'))
      return false;
    if (!isValidRegisteredKey(incomingChallenge, appIdRequired)) {
      return false;
    }
  }
  return true;
}

/**
 * @param {Object} request Request object
 * @param {MessageSender} sender Sender frame
 * @param {Function} sendResponse Response callback
 * @return {?Closeable} Optional handler object that should be closed when port
 *     closes
 */
function handleWebPageRequest(request, sender, sendResponse) {
  switch (request.type) {
    case MessageTypes.U2F_REGISTER_REQUEST:
      return handleU2fEnrollRequest(sender, request, sendResponse);

    case MessageTypes.U2F_SIGN_REQUEST:
      return handleU2fSignRequest(sender, request, sendResponse);

    case MessageTypes.U2F_GET_API_VERSION_REQUEST:
      sendResponse(
          makeU2fGetApiVersionResponse(request, JS_API_VERSION,
              MessageTypes.U2F_GET_API_VERSION_RESPONSE));
      return null;

    default:
      sendResponse(
          makeU2fErrorResponse(request, ErrorCodes.BAD_REQUEST, undefined,
              MessageTypes.U2F_REGISTER_RESPONSE));
      return null;
  }
}

/**
 * Makes a response to a request.
 * @param {Object} request The request to make a response to.
 * @param {string} responseSuffix How to name the response's type.
 * @param {string=} opt_defaultType The default response type, if none is
 *     present in the request.
 * @return {Object} The response object.
 */
function makeResponseForRequest(request, responseSuffix, opt_defaultType) {
  var type;
  if (request && request.type) {
    type = request.type.replace(/_request$/, responseSuffix);
  } else {
    type = opt_defaultType;
  }
  var reply = { 'type': type };
  if (request && request.requestId) {
    reply.requestId = request.requestId;
  }
  return reply;
}

/**
 * Makes a response to a U2F request with an error code.
 * @param {Object} request The request to make a response to.
 * @param {ErrorCodes} code The error code to return.
 * @param {string=} opt_detail An error detail string.
 * @param {string=} opt_defaultType The default response type, if none is
 *     present in the request.
 * @return {Object} The U2F error.
 */
function makeU2fErrorResponse(request, code, opt_detail, opt_defaultType) {
  var reply = makeResponseForRequest(request, '_response', opt_defaultType);
  var error = {'errorCode': code};
  if (opt_detail) {
    error['errorMessage'] = opt_detail;
  }
  reply['responseData'] = error;
  return reply;
}

/**
 * Makes a success response to a web request with a responseData object.
 * @param {Object} request The request to make a response to.
 * @param {Object} responseData The response data.
 * @return {Object} The web error.
 */
function makeU2fSuccessResponse(request, responseData) {
  var reply = makeResponseForRequest(request, '_response');
  reply['responseData'] = responseData;
  return reply;
}

/**
 * Maps a helper's error code from the DeviceStatusCodes namespace to a
 * U2fError.
 * @param {number} code Error code from DeviceStatusCodes namespace.
 * @return {U2fError} An error.
 */
function mapDeviceStatusCodeToU2fError(code) {
  switch (code) {
    case DeviceStatusCodes.WRONG_DATA_STATUS:
      return {errorCode: ErrorCodes.DEVICE_INELIGIBLE};

    case DeviceStatusCodes.TIMEOUT_STATUS:
    case DeviceStatusCodes.WAIT_TOUCH_STATUS:
      return {errorCode: ErrorCodes.TIMEOUT};

    default:
      var reportedError = {
        errorCode: ErrorCodes.OTHER_ERROR,
        errorMessage: 'device status code: ' + code.toString(16)
      };
      return reportedError;
  }
}

/**
 * Sends a response, using the given sentinel to ensure at most one response is
 * sent. Also closes the closeable, if it's given.
 * @param {boolean} sentResponse Whether a response has already been sent.
 * @param {?Closeable} closeable A thing to close.
 * @param {*} response The response to send.
 * @param {Function} sendResponse A function to send the response.
 */
function sendResponseOnce(sentResponse, closeable, response, sendResponse) {
  if (closeable) {
    closeable.close();
  }
  if (!sentResponse) {
    sentResponse = true;
    try {
      // If the page has gone away or the connection has otherwise gone,
      // sendResponse fails.
      sendResponse(response);
    } catch (exception) {
      console.warn('sendResponse failed: ' + exception);
    }
  } else {
    console.warn(UTIL_fmt('Tried to reply more than once!'));
  }
}

/**
 * @param {!string} string Input string
 * @return {Array<number>} SHA256 hash value of string.
 */
function sha256HashOfString(string) {
  var s = new SHA256();
  s.update(UTIL_StringToBytes(string));
  return s.digest();
}

var UNUSED_CID_PUBKEY_VALUE = "unused";

/**
 * Normalizes the TLS channel ID value:
 * 1. Converts semantically empty values (undefined, null, 0) to the empty
 *     string.
 * 2. Converts valid JSON strings to a JS object.
 * 3. Otherwise, returns the input value unmodified.
 * @param {Object|string|undefined} opt_tlsChannelId TLS Channel id
 * @return {Object|string} The normalized TLS channel ID value.
 */
function tlsChannelIdValue(opt_tlsChannelId) {
  if (!opt_tlsChannelId) {
    // Case 1: Always set some value for TLS channel ID, even if it's the empty
    // string: this browser definitely supports them.
    return UNUSED_CID_PUBKEY_VALUE;
  }
  if (typeof opt_tlsChannelId === 'string') {
    try {
      var obj = JSON.parse(opt_tlsChannelId);
      if (!obj) {
        // Case 1: The string value 'null' parses as the Javascript object null,
        // so return an empty string: the browser definitely supports TLS
        // channel id.
        return UNUSED_CID_PUBKEY_VALUE;
      }
      // Case 2: return the value as a JS object.
      return /** @type {Object} */ (obj);
    } catch (e) {
      console.warn('Unparseable TLS channel ID value ' + opt_tlsChannelId);
      // Case 3: return the value unmodified.
    }
  }
  return opt_tlsChannelId;
}

/**
 * Creates a browser data object with the given values.
 * @param {!string} type A string representing the "type" of this browser data
 *     object.
 * @param {!string} serverChallenge The server's challenge, as a base64-
 *     encoded string.
 * @param {!string} origin The server's origin, as seen by the browser.
 * @param {Object|string|undefined} opt_tlsChannelId TLS Channel Id
 * @return {string} A string representation of the browser data object.
 */
function makeBrowserData(type, serverChallenge, origin, opt_tlsChannelId) {
  var browserData = {
    'typ' : type,
    'challenge' : serverChallenge,
    'origin' : origin
  };
  if (BROWSER_SUPPORTS_TLS_CHANNEL_ID) {
    browserData['cid_pubkey'] = tlsChannelIdValue(opt_tlsChannelId);
  }
  return JSON.stringify(browserData);
}

/**
 * Creates a browser data object for an enroll request with the given values.
 * @param {!string} serverChallenge The server's challenge, as a base64-
 *     encoded string.
 * @param {!string} origin The server's origin, as seen by the browser.
 * @param {Object|string|undefined} opt_tlsChannelId TLS Channel Id
 * @return {string} A string representation of the browser data object.
 */
function makeEnrollBrowserData(serverChallenge, origin, opt_tlsChannelId) {
  return makeBrowserData(
      'navigator.id.finishEnrollment', serverChallenge, origin,
      opt_tlsChannelId);
}

/**
 * Creates a browser data object for a sign request with the given values.
 * @param {!string} serverChallenge The server's challenge, as a base64-
 *     encoded string.
 * @param {!string} origin The server's origin, as seen by the browser.
 * @param {Object|string|undefined} opt_tlsChannelId TLS Channel Id
 * @return {string} A string representation of the browser data object.
 */
function makeSignBrowserData(serverChallenge, origin, opt_tlsChannelId) {
  return makeBrowserData(
      'navigator.id.getAssertion', serverChallenge, origin, opt_tlsChannelId);
}

/**
 * Makes a response to a U2F request with an error code.
 * @param {Object} request The request to make a response to.
 * @param {number=} version The JS API version to return.
 * @param {string=} opt_defaultType The default response type, if none is
 *     present in the request.
 * @return {Object} The GetJsApiVersionResponse.
 */
function makeU2fGetApiVersionResponse(request, version, opt_defaultType) {
  var reply = makeResponseForRequest(request, '_response', opt_defaultType);
  var data = {'js_api_version': version};
  reply['responseData'] = data;
  return reply;
}

/**
 * Encodes the sign data as an array of sign helper challenges.
 * @param {Array<SignChallenge>} signChallenges The sign challenges to encode.
 * @param {string|undefined} opt_defaultChallenge A default sign challenge
 *     value, if a request does not provide one.
 * @param {string=} opt_defaultAppId The app id to use for each challenge, if
 *     the challenge contains none.
 * @param {function(string, string): string=} opt_challengeHashFunction
 *     A function that produces, from a key handle and a raw challenge, a hash
 *     of the raw challenge. If none is provided, a default hash function is
 *     used.
 * @return {!Array<SignHelperChallenge>} The sign challenges, encoded.
 */
function encodeSignChallenges(signChallenges, opt_defaultChallenge,
    opt_defaultAppId, opt_challengeHashFunction) {
  function encodedSha256(keyHandle, challenge) {
    return B64_encode(sha256HashOfString(challenge));
  }
  var challengeHashFn = opt_challengeHashFunction || encodedSha256;
  var encodedSignChallenges = [];
  if (signChallenges) {
    for (var i = 0; i < signChallenges.length; i++) {
      var challenge = signChallenges[i];
      var keyHandle = challenge['keyHandle'];
      var challengeValue;
      if (challenge.hasOwnProperty('challenge')) {
        challengeValue = challenge['challenge'];
      } else {
        challengeValue = opt_defaultChallenge;
      }
      var challengeHash = challengeHashFn(keyHandle, challengeValue);
      var appId;
      if (challenge.hasOwnProperty('appId')) {
        appId = challenge['appId'];
      } else {
        appId = opt_defaultAppId;
      }
      var encodedChallenge = {
        'challengeHash': challengeHash,
        'appIdHash': B64_encode(sha256HashOfString(appId)),
        'keyHandle': keyHandle,
        'version': (challenge['version'] || 'U2F_V1')
      };
      encodedSignChallenges.push(encodedChallenge);
    }
  }
  return encodedSignChallenges;
}

/**
 * Makes a sign helper request from an array of challenges.
 * @param {Array<SignHelperChallenge>} challenges The sign challenges.
 * @param {number=} opt_timeoutSeconds Timeout value.
 * @param {string=} opt_logMsgUrl URL to log to.
 * @return {SignHelperRequest} The sign helper request.
 */
function makeSignHelperRequest(challenges, opt_timeoutSeconds, opt_logMsgUrl) {
  var request = {
    'type': 'sign_helper_request',
    'signData': challenges,
    'timeout': opt_timeoutSeconds || 0,
    'timeoutSeconds': opt_timeoutSeconds || 0
  };
  if (opt_logMsgUrl !== undefined) {
    request.logMsgUrl = opt_logMsgUrl;
  }
  return request;
}



//
// from u2f-ble-helper/u2f.js
//
var U2F_HEADER_BYTES = 7;
var U2F_FOOTER_BYTES = 2;
var U2F_KEYHANDLE_BYTE_LENGTH = 1;


var U2F_BLE_PING = 0x81;
var U2F_BLE_MSG = 0x83;
var U2F_BLE_ERROR = 0xbf;

var U2F_ENROLL_COMMAND = 0x01;
var U2F_AUTHENTICATE_COMMAND = 0x02;

var U2F_REQUIRE_PHYSICAL_PRESENCE = 0x03;

function createEnrollCommand(enrollInfo) {
    var hashChallenge = B64_decode(enrollInfo.enrollChallenges[0].challengeHash); 
    var hashApp = B64_decode(enrollInfo.enrollChallenges[0].appIdHash);

    var lenU2F = hashChallenge.length + hashApp.length;
    var lenData = lenU2F + U2F_HEADER_BYTES + U2F_FOOTER_BYTES;
    var msgLengthHi = (lenData & 0XFF00) >> 8;
    var msgLengthLow = (lenData & 0x00FF);

    var enrollLengthHi = (lenU2F & 0XFF00) >> 8;
    var enrollLengthLow = (lenU2F & 0x00FF);

    var apdu = new Uint8Array([
        U2F_BLE_MSG, msgLengthHi, msgLengthLow, 
        0x00, U2F_ENROLL_COMMAND, U2F_REQUIRE_PHYSICAL_PRESENCE, 0x00, 
        0x00, enrollLengthHi, enrollLengthLow]);
    var Le = new Uint8Array([0x00, 0x00]);
    var u8 = new Uint8Array(apdu.length + hashChallenge.length + hashApp.length + Le.length);
    u8.set(apdu);
    u8.set(hashChallenge, apdu.length);
    u8.set(hashApp,       apdu.length + hashChallenge.length);
    u8.set(Le,            apdu.length + hashChallenge.length + hashApp.length);
    return u8.buffer;
}

function createSignCommand(signInfo){
  var hashChallenge = B64_decode(signInfo.signData[0].challengeHash); 
  var hashApp = B64_decode(signInfo.signData[0].appIdHash);
  var hashHandle = B64_decode(signInfo.signData[0].keyHandle);
  
  var lenU2F = hashChallenge.length + hashApp.length + U2F_KEYHANDLE_BYTE_LENGTH + hashHandle.length;
  var lenData = lenU2F + U2F_HEADER_BYTES + U2F_FOOTER_BYTES;
  var msgLengthHi = (lenData & 0XFF00) >> 8;
  var msgLengthLow = (lenData & 0x00FF);
  
  //calculate the authentication message length
  var authLengthHi = (lenU2F & 0XFF00) >> 8;
  var authLengthLow = (lenU2F & 0x00FF);
  var keyHandleLength = hashHandle.length;
  
  //build up the message to send to the BLE U2F authenticator
  var apdu = new Uint8Array([U2F_BLE_MSG, msgLengthHi, msgLengthLow, 0x00, U2F_AUTHENTICATE_COMMAND, U2F_REQUIRE_PHYSICAL_PRESENCE, 0x00, 0x00, authLengthHi, authLengthLow]);
  var Le = new Uint8Array([0x00, 0x00]);
  var u8 = new Uint8Array(apdu.length +  hashChallenge.length + hashApp.length + U2F_KEYHANDLE_BYTE_LENGTH + hashHandle.length + Le.length);
  u8.set(apdu);
  u8.set(hashChallenge, apdu.length);
  u8.set(hashApp,       apdu.length + hashChallenge.length);
  u8[apdu.length + hashChallenge.length + hashApp.length] = keyHandleLength;
  u8.set(hashHandle,    apdu.length + hashChallenge.length + hashApp.length + U2F_KEYHANDLE_BYTE_LENGTH);
  u8.set(Le,            apdu.length + hashChallenge.length + hashApp.length + U2F_KEYHANDLE_BYTE_LENGTH + hashHandle.length);
  return u8.buffer;
}
//
// for debug information
//
function unPackBinaryToHex(byteArray){
    if (!byteArray) {
        return;
    }
    var data_view = new Uint8Array(byteArray);
    var hexString = "";
    for (var i = 0; i < data_view.length; i++) {
        var hexVal = data_view[i].toString(16);
        if (hexVal.length == 1) {
            hexVal = "0" + hexVal;
        }
        hexString += hexVal;
    }
    return hexString;
}
