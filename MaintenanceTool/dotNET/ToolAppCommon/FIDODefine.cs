namespace MaintenanceToolApp
{
    public class FIDODefine
    {
        // FIDO機能関連コマンドバイト
        public const byte FIDO_CMD_MSG = 0x03;
        public const byte MNT_COMMAND_BASE = 0x40;
        public const byte MNT_COMMAND_GET_FLASH_STAT = 0x42;
        public const byte MNT_COMMAND_GET_APP_VERSION = 0x43;
        public const byte MNT_COMMAND_BOOTLOADER_MODE = 0x45;
        public const byte MNT_COMMAND_ERASE_BONDING_DATA = 0x46;
        public const byte MNT_COMMAND_SYSTEM_RESET = 0x47;
        public const byte MNT_COMMAND_RESET_ATTESTATION = 0x49;
        public const byte MNT_COMMAND_GET_TIMESTAMP = 0x4a;
        public const byte MNT_COMMAND_SET_TIMESTAMP = 0x4b;
        public const byte MNT_COMMAND_UNPAIRING_REQUEST = 0x4d;
        public const byte MNT_COMMAND_UNPAIRING_CANCEL = 0x4e;

        // FIDO機能関連エラーステータス
        public const int CTAP1_ERR_SUCCESS = 0x00;
        public const int CTAP2_ERR_PIN_INVALID = 0x31;
        public const int CTAP2_ERR_PIN_BLOCKED = 0x32;
        public const int CTAP2_ERR_PIN_AUTH_INVALID = 0x33;
        public const int CTAP2_ERR_PIN_AUTH_BLOCKED = 0x34;
        public const int CTAP2_ERR_PIN_NOT_SET = 0x35;
        public const int CTAP2_ERR_VENDOR_KEY_CRT_NOT_EXIST = 0xfe;

        // CBORサブコマンドバイトに関する定義
        public const byte CTAP2_CBORCMD_NONE = 0x00;
        public const byte CTAP2_CBORCMD_MAKE_CREDENTIAL = 0x01;
        public const byte CTAP2_CBORCMD_GET_ASSERTION = 0x02;
        public const byte CTAP2_CBORCMD_CLIENT_PIN = 0x06;
        public const byte CTAP2_CBORCMD_AUTH_RESET = 0x07;
        public const byte CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT = 0x02;
        public const byte CTAP2_SUBCMD_CLIENT_PIN_SET = 0x03;
        public const byte CTAP2_SUBCMD_CLIENT_PIN_CHANGE = 0x04;
        public const byte CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN = 0x05;

        // U2Fに関する定義
        public const int U2F_INS_REGISTER = 0x01;
        public const int U2F_INS_AUTHENTICATE = 0x02;
        public const int U2F_INS_VERSION = 0x03;
        public const int U2F_AUTH_ENFORCE = 0x03;
        public const int U2F_AUTH_CHECK_ONLY = 0x07;
        public const int U2F_APPID_SIZE = 32;
        public const int U2F_NONCE_SIZE = 32;

        // BLE U2Fサービスに関する定義
        public const string U2F_BLE_SERVICE_UUID_STR = "0000FFFD-0000-1000-8000-00805f9b34fb";
        public const string U2F_CONTROL_POINT_CHAR_UUID_STR = "F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB";
        public const string U2F_STATUS_CHAR_UUID_STR = "F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB";
    }
}
