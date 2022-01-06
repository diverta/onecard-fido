namespace MaintenanceToolGUI
{
    public class ToolPGPParameter
    {
        // 鍵作成用パラメーター
        public string RealName { get; set; }
        public string MailAddress { get; set; }
        public string Comment { get; set; }
        public string Passphrase { get; set; }
        public string PubkeyFolderPath { get; set; }
        public string BackupFolderPath { get; set; }
    }

    public class ToolPGP
    {
        // 画面の参照を保持
        private MainForm mainForm;

        // 処理クラスの参照を保持
        private HIDMain hidMain;

        // 実行する自動認証設定コマンドの種別
        public enum GPGCommand
        {
            COMMAND_GPG_NONE = 1,
            COMMAND_GPG_VERSION,
            COMMAND_GPG_MAKE_TEMP_FOLDER,
            COMMAND_GPG_GENERATE_MAIN_KEY,
            COMMAND_GPG_ADD_SUB_KEY,
            COMMAND_GPG_EXPORT_PUBKEY_AND_BACKUP,
            COMMAND_GPG_TRANSFER_SUBKEY_TO_CARD,
            COMMAND_GPG_REMOVE_TEMP_FOLDER,
            COMMAND_GPG_CARD_STATUS,
            COMMAND_GPG_CARD_RESET,
        };

        // リクエストパラメーターを保持
        private ToolPGPParameter toolPGPParameter = null;

        public ToolPGP(MainForm f, HIDMain h)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // HID処理クラスの参照を保持
            hidMain = h;
        }

        public void ShowDialog()
        {
            // TODO: 仮の実装です。
            ToolPGPParameter parameter = new ToolPGPParameter();
            parameter.RealName = "";
            parameter.MailAddress = "";
            parameter.Comment = "";
            parameter.Passphrase = "";
            parameter.PubkeyFolderPath = "";
            parameter.BackupFolderPath = "";
        }
    }
}
