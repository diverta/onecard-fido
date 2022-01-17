using MaintenanceToolCommon;
using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class PGPPreferenceForm : Form
    {
        // 処理クラスの参照を保持
        private ToolPGP ToolPGPRef;

        // 設定パラメーターを保持
        private ToolPGPParameter parameter = new ToolPGPParameter();

        public PGPPreferenceForm(ToolPGP toolPGP)
        {
            // 画面項目の初期化
            InitializeComponent();
            InitFieldValue();

            // 処理クラスの参照を保持
            ToolPGPRef = toolPGP;
        }

        private void buttonPubkeyFolderPath_Click(object sender, EventArgs e)
        {
            // フォルダーを選択
            FormUtil.SelectFolderPath(folderBrowserDialog1, textPubkeyFolderPath);
        }

        private void buttonBackupFolderPath_Click(object sender, EventArgs e)
        {
            // フォルダーを選択
            FormUtil.SelectFolderPath(folderBrowserDialog1, textBackupFolderPath);
        }

        private void buttonPGPStatus_Click(object sender, EventArgs e)
        {
            // PGPステータス照会処理を実行
            EnableButtons(false);
            DoCommandPGPStatus();
        }

        private void buttonPGPReset_Click(object sender, EventArgs e)
        {
            // プロンプトで表示されるタイトル
            string title = string.Format(
                ToolGUICommon.MSG_FORMAT_OPENPGP_WILL_PROCESS,
                ToolGUICommon.MSG_LABEL_COMMAND_OPENPGP_RESET);

            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (FormUtil.DisplayPromptPopup(this, title, ToolGUICommon.MSG_PROMPT_OPENPGP_RESET) == false) {
                return;
            }

            // PGPリセット処理を実行
            EnableButtons(false);
            DoCommandPGPReset();
        }

        private void buttonFirmwareReset_Click(object sender, EventArgs e)
        {
            // 認証器のファームウェアを再起動
            EnableButtons(false);
            DoCommandResetFirmware();
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.Cancel);
        }

        private void TerminateWindow(DialogResult dialogResult)
        {
            // 画面項目を初期化し、この画面を閉じる
            InitFieldValue();
            DialogResult = dialogResult;
            Close();
        }

        //
        // PGP鍵管理タブ関連の処理
        //
        private void InitFieldValue()
        {
            // PGP鍵管理タブ内の入力項目を初期化（このタブが選択状態になります）
            tabPreference.SelectedTab = tabPagePGPKeyManagement;
            InitTabPGPKeyManagement();
        }

        private void InitTabPGPKeyManagement()
        {
            // テキストボックスの初期化
            InitTabPGPKeyPathFields();
            InitTabPGPKeyEntryFields();
        }

        private void InitTabPGPKeyPathFields()
        {
            // ファイルパスのテキストボックスを初期化
            textPubkeyFolderPath.Text = "";
            textBackupFolderPath.Text = "";
        }

        private void InitTabPGPKeyEntryFields()
        {
            // テキストボックスを初期化
            textRealName.Text = "";
            textMailAddress.Text = "";
            textComment.Text = "";
            textPin.Text = "";
            textPinConfirm.Text = "";

            // テキストボックスのカーソルを先頭の項目に配置
            textRealName.Focus();
        }

        void EnableButtons(bool enabled)
        {
            // ボタンや入力欄の使用可能／不可制御
            buttonClose.Enabled = enabled;
            buttonPGPStatus.Enabled = enabled;
            buttonPGPReset.Enabled = enabled;
            buttonFirmwareReset.Enabled = enabled;

            // 現在選択中のタブ内も同様に制御を行う
            TabPage page = tabPreference.SelectedTab;
            if (page == tabPagePGPKeyManagement) {
                EnableButtonsInTabPGPKeyManagement(enabled);
            }
        }

        void EnableButtonsInTabPGPKeyManagement(bool enabled)
        {
            // ボタンや入力欄の使用可能／不可制御
            textRealName.Enabled = enabled;
            textMailAddress.Enabled = enabled;
            textComment.Enabled = enabled;
            textPin.Enabled = enabled;
            textPinConfirm.Enabled = enabled;

            buttonPubkeyFolderPath.Enabled = enabled;
            buttonBackupFolderPath.Enabled = enabled;
            buttonInstallPGPKey.Enabled = enabled;
        }

        //
        // OpenPGP設定機能の各処理
        //
        void DoCommandPGPStatus()
        {
            ToolPGPRef.DoOpenPGPCommand(AppCommon.RequestType.OpenPGPStatus, null);
        }

        void DoCommandPGPReset()
        {
            ToolPGPRef.DoOpenPGPCommand(AppCommon.RequestType.OpenPGPReset, null);
        }

        void DoCommandResetFirmware()
        {
            ToolPGPRef.DoCommandResetFirmware();
        }

        public void OnCommandProcessTerminated(AppCommon.RequestType requestType, bool success, string errMessage)
        {
            // 処理終了メッセージをポップアップ表示後、画面項目を使用可とする
            DisplayResultMessage(requestType, success, errMessage);
            EnableButtons(true);
        }

        private void DisplayResultMessage(AppCommon.RequestType requestType, bool success, string errMessage)
        {
            // 処理名称を設定
            string name = "";
            switch (requestType) {
                case AppCommon.RequestType.OpenPGPStatus:
                    if (success) {
                        // メッセージの代わりに、OpenPGP設定情報を、情報表示画面に表示
                        MessageBox.Show(this, ToolPGPRef.GetPGPStatusInfoString(), MainForm.MaintenanceToolTitle, MessageBoxButtons.OK, MessageBoxIcon.Information);
                        return;
                    }
                    name = ToolGUICommon.MSG_LABEL_COMMAND_OPENPGP_STATUS;
                    break;
            case AppCommon.RequestType.OpenPGPReset:
                    name = ToolGUICommon.MSG_LABEL_COMMAND_OPENPGP_RESET;
                    break;
                case AppCommon.RequestType.HidFirmwareReset:
                    name = ToolGUICommon.PROCESS_NAME_FIRMWARE_RESET;
                    break;
                default:
                    break;
            }
            // メッセージをポップアップ表示
            string formatted = string.Format(ToolGUICommon.MSG_FORMAT_END_MESSAGE,
                name,
                success ? ToolGUICommon.MSG_SUCCESS : ToolGUICommon.MSG_FAILURE);
            if (success) {
                MessageBox.Show(this, formatted, MainForm.MaintenanceToolTitle, MessageBoxButtons.OK, MessageBoxIcon.Information);
            } else {
                MessageBox.Show(this, errMessage, formatted, MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }
    }
}
