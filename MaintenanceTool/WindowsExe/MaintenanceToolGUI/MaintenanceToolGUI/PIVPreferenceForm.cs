using System;
using System.Windows.Forms;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public partial class PIVPreferenceForm : Form
    {
        // 処理クラスの参照を保持
        private ToolPIV ToolPIVRef;

        public PIVPreferenceForm(ToolPIV toolPIV)
        {
            // 画面項目の初期化
            InitializeComponent();

            // 処理クラスの参照を保持
            ToolPIVRef = toolPIV;
        }

        private void buttonFirmwareReset_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (ToolPIVRef.CheckUSBDeviceDisconnected()) {
                return;
            }

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
            DialogResult = dialogResult;
            Close();
        }

        private void EnableButtons(bool enabled)
        {
            // ボタンや入力欄の使用可能／不可制御
            buttonClose.Enabled = enabled;
            buttonPivStatus.Enabled = enabled;
            buttonInitialSetting.Enabled = enabled;
            buttonClearSetting.Enabled = enabled;
            buttonFirmwareReset.Enabled = enabled;

            // 現在選択中のタブ内も同様に制御を行う
            TabPage page = tabPreference.SelectedTab;
            if (page.Equals(tabPagePkeyCertManagement)) {
                EnableButtonsInTabPagePkeyCertManagement(enabled);
            }
            if (page.Equals(tabPagePinManagement)) {
                EnableButtonsInTabPinManagement(enabled);
            }
        }

        //
        // 鍵・証明書管理タブ関連の処理
        //
        void EnableButtonsInTabPagePkeyCertManagement(bool enabled)
        {
            // ボタンや入力欄の使用可能／不可制御
            textPin.Enabled = enabled;
            textPinConfirm.Enabled = enabled;

            buttonPkeyFolderPath.Enabled = enabled;
            buttonCertFolderPath.Enabled = enabled;
            buttonInstallPkeyCert.Enabled = enabled;
        }

        //
        // PIN番号管理タブ関連の処理
        //
        void EnableButtonsInTabPinManagement(bool enabled)
        {
            // ボタンや入力欄の使用可能／不可制御
            groupBoxPinCommand.Enabled = enabled;
            groupBoxPinText.Enabled = enabled;
            buttonPerformPinCommand.Enabled = enabled;
        }

        //
        // PIV設定機能の各処理
        //
        void DoCommandResetFirmware()
        {
            ToolPIVRef.DoCommandResetFirmware();
        }

        public void OnCommandProcessTerminated(AppCommon.RequestType requestType, bool success, string errMessage)
        {
            // 処理終了メッセージをポップアップ表示後、画面項目を使用可とする
            DisplayResultMessage(requestType, success, errMessage);
            ClearEntry(requestType, success);
            EnableButtons(true);
        }

        private void DisplayResultMessage(AppCommon.RequestType requestType, bool success, string errMessage)
        {
            // 処理名称を設定
            string name = "";
            switch (requestType) {
            case AppCommon.RequestType.HidFirmwareReset:
                name = AppCommon.PROCESS_NAME_FIRMWARE_RESET;
                break;
            default:
                break;
            }

            // メッセージをポップアップ表示
            string formatted = string.Format(AppCommon.MSG_FORMAT_END_MESSAGE,
                name,
                success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);
            if (success) {
                FormUtil.ShowInfoMessage(this, MainForm.MaintenanceToolTitle, formatted);
            } else {
                FormUtil.ShowWarningMessage(this, formatted, errMessage);
            }
        }

        private void ClearEntry(AppCommon.RequestType requestType, bool success)
        {
            // 全ての入力欄をクリア
            switch (requestType) {
            default:
                break;
            }
        }
    }
}
