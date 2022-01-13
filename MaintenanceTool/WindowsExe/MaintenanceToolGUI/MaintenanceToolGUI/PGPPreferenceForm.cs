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
    }
}
