/**
 * @fileoverview Implements a helper using BLE Authenticator.
 */
'use strict';

/**
 * @constructor
 * @extends {GenericRequestHelper}
 */
function BleHelper() {
  GenericRequestHelper.apply(this, arguments);

  var self = this;
  this.registerHandlerFactory('enroll_helper_request', function(request) {
    return new BleEnrollHandler(/** @type {EnrollHelperRequest} */ (request));
  });
  this.registerHandlerFactory('sign_helper_request', function(request) {
    return new BleSignHandler(/** @type {SignHelperRequest} */ (request));
  });
}

inherits(BleHelper, GenericRequestHelper);


//var BLE_HELPER_EXTENSION_ID = 'naodkhmgbblamoijhmonofommoajlide';
// for test
var BLE_HELPER_EXTENSION_ID = 'mmafjllbfijjcejkmnaoioihhfnelodd';

function BleEnrollHandler(request) {
  this.request_  = request;
  this.callback_ = null;
}

BleEnrollHandler.prototype.run = function(callback) {
  this.callback_ = callback;
  console.log('BleEnrollHandler: request=', this.request_);
  console.log('BleEnrollHandler: extensionID=', BLE_HELPER_EXTENSION_ID);

  var handler = this.onEnrollHelperReplyed.bind(this);
  chrome.runtime.sendMessage(BLE_HELPER_EXTENSION_ID, this.request_, handler);
  return true;
};

BleEnrollHandler.prototype.onEnrollHelperReplyed = function(response) {
  console.log('BleEnrollHandler: response=', response);

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
  console.log('BleSignHandler: extensionID=', BLE_HELPER_EXTENSION_ID);

  var handler = this.onSignHelperReplyed.bind(this);
  chrome.runtime.sendMessage(BLE_HELPER_EXTENSION_ID, this.request_, handler);
  return true;
};

BleSignHandler.prototype.onSignHelperReplyed = function(response) {
  console.log('BleSignHandler: response=', response);

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
