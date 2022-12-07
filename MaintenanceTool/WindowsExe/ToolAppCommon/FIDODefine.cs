namespace MaintenanceToolApp
{
    internal class FIDODefine
    {
        // FIDO機能関連コマンドバイト
        public const byte FIDO_CMD_MSG = 0x03;
        public const byte MNT_COMMAND_BASE = 0x40;
        public const byte MNT_COMMAND_GET_FLASH_STAT = 0x42;
        public const byte MNT_COMMAND_GET_APP_VERSION = 0x43;
        public const byte MNT_COMMAND_BOOTLOADER_MODE = 0x45;
        public const byte MNT_COMMAND_ERASE_BONDING_DATA = 0x46;
        public const byte MNT_COMMAND_SYSTEM_RESET = 0x47;
        public const byte MNT_COMMAND_GET_TIMESTAMP = 0x4a;
        public const byte MNT_COMMAND_SET_TIMESTAMP = 0x4b;

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
    }
}
