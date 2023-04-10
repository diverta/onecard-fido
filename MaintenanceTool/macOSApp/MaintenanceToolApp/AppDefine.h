//
//  AppDefine.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/06.
//
#ifndef AppDefine_h
#define AppDefine_h

// コマンド種別
typedef enum : NSInteger {
    COMMAND_NONE = 1,
    COMMAND_TEST_REGISTER,
    COMMAND_TEST_AUTH_CHECK,
    COMMAND_TEST_AUTH_NO_USER_PRESENCE,
    COMMAND_TEST_AUTH_USER_PRESENCE,
    COMMAND_PAIRING,
    COMMAND_UNPAIRING_REQUEST,
    COMMAND_UNPAIRING_CANCEL_REQUEST,
    COMMAND_TEST_CTAPHID_PING,
    COMMAND_TEST_BLE_PING,
    COMMAND_BLE_GET_VERSION_INFO,
    COMMAND_HID_GET_FLASH_STAT,
    COMMAND_HID_GET_VERSION_INFO,
    COMMAND_HID_GET_VERSION_FOR_DFU,
    COMMAND_HID_BOOTLOADER_MODE,
    COMMAND_HID_FIRMWARE_RESET,
    COMMAND_CLIENT_PIN_SET,
    COMMAND_CLIENT_PIN_CHANGE,
    COMMAND_TEST_MAKE_CREDENTIAL,
    COMMAND_TEST_GET_ASSERTION,
    COMMAND_AUTH_RESET,
    COMMAND_TOOL_PREF_PARAM,
    COMMAND_TOOL_PREF_PARAM_INQUIRY,
    COMMAND_USB_DFU,
    COMMAND_BLE_DFU,
    COMMAND_BLE_DFU_GET_SLOT_INFO,
    COMMAND_BLE_DFU_UPLOAD_IMAGE,
    COMMAND_BLE_DFU_CHANGE_IMAGE_UPDATE_MODE,
    COMMAND_BLE_DFU_RESET_APPLICATION,
    COMMAND_ERASE_BONDS,
    COMMAND_CCID_PIV_CHANGE_PIN,
    COMMAND_CCID_PIV_CHANGE_PUK,
    COMMAND_CCID_PIV_UNBLOCK_PIN,
    COMMAND_CCID_PIV_RESET,
    COMMAND_CCID_PIV_IMPORT_KEY,
    COMMAND_CCID_PIV_SET_CHUID,
    COMMAND_CCID_PIV_STATUS,
    COMMAND_OPENPGP_INSTALL_KEYS,
    COMMAND_OPENPGP_STATUS,
    COMMAND_OPENPGP_RESET,
    COMMAND_OPENPGP_CHANGE_PIN,
    COMMAND_OPENPGP_CHANGE_ADMIN_PIN,
    COMMAND_OPENPGP_UNBLOCK_PIN,
    COMMAND_OPENPGP_SET_RESET_CODE,
    COMMAND_OPENPGP_UNBLOCK,
    COMMAND_OATH_SCAN_QRCODE,
    COMMAND_OATH_SHOW_PASSWORD,
    COMMAND_OATH_DELETE_ACCOUNT,
    COMMAND_OPEN_WINDOW_PINPARAM,
    COMMAND_OPEN_WINDOW_PIVPARAM,
    COMMAND_OPEN_WINDOW_PGPPARAM,
    COMMAND_VIEW_APP_VERSION,
    COMMAND_VIEW_LOG_FILE,
    COMMAND_BLE_CTAP2_HCHECK,
    COMMAND_BLE_U2F_HCHECK,
    COMMAND_HID_CTAP2_INIT,
    COMMAND_HID_CTAP2_HCHECK,
    COMMAND_HID_U2F_HCHECK,
    COMMAND_CTAP2_GET_KEY_AGREEMENT,
    COMMAND_CTAP2_GET_PIN_TOKEN,
    COMMAND_RTCC_SETTING,
    COMMAND_RTCC_GET_TIMESTAMP,
    COMMAND_RTCC_SET_TIMESTAMP,
    COMMAND_BLE_CONNECT_ONLY,
    COMMAND_INSTALL_ATTESTATION,
    COMMAND_REMOVE_ATTESTATION,
} Command;

// トランスポート種別
typedef enum : NSInteger {
    TRANSPORT_NONE = 1,
    TRANSPORT_BLE,
    TRANSPORT_HID
} TransportType;

#endif /* AppDefine_h */
