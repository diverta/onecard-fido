namespace VendorMaintenanceTool
{
    internal class VendorAppCommon
    {
        // ベンダー向け機能関連
        public const string MSG_MENU_ITEM_NAME_VENDOR_FUNCTION = "ベンダー向け機能";

        // コマンド種別に対応する処理名称
        public const string PROCESS_NAME_INSTALL_ATTESTATION = "鍵・証明書インストール";
        public const string PROCESS_NAME_REMOVE_ATTESTATION = "鍵・証明書の削除";
        public const string PROCESS_NAME_BOOT_LOADER_MODE = "ブートローダーモード遷移";
        public const string PROCESS_NAME_FIRMWARE_RESET = "認証器のファームウェア再起動";

        // 確認メッセージ
        public const string MSG_ERASE_SKEY_CERT = "FIDO認証器から鍵・証明書・ユーザー登録情報をすべて削除します。";
        public const string MSG_PROMPT_ERASE_SKEY_CERT = "削除後はFIDO認証器によるユーザー登録／ログインができなくなります。\n削除処理を実行しますか？";
        public const string MSG_CHANGE_TO_BOOTLOADER_MODE = "FIDO認証器をブートローダーモードに遷移させます。";
        public const string MSG_PROMPT_CHANGE_TO_BOOTLOADER_MODE = "ブートローダーモードに遷移したら、nRFコマンドラインツール等を使用し、ファームウェア更新イメージをFIDO認証器に転送できます。\n遷移処理を実行しますか？";
        public const string MSG_FIRMWARE_RESET = "FIDO認証器のファームウェアを再起動します。";
        public const string MSG_PROMPT_FIRMWARE_RESET = "処理を実行しますか？";
    }
}
