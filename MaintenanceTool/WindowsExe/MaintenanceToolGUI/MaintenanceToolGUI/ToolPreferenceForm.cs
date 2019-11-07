using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class ToolPreferenceForm : Form
    {
        // メイン画面の参照を保持
        private MainForm mainForm;

        // 実行した機能名を保持
        private string funcName;

        public ToolPreferenceForm()
        {
            InitializeComponent();
            InitFieldValue();
        }

        public void SetMainForm(MainForm f)
        {
            mainForm = f;
        }

        public void SetTitleAndVersionText(String toolName, String toolVersion)
        {
            // ツールタイトル表示
            labelToolName.Text = toolName;

            // バージョン表示
            labelVersion.Text = toolVersion;
        }

        private void buttonCancel_Click(object sender, EventArgs e)
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
        // 自動認証設定タブ関連の処理
        //
        private void InitFieldValue()
        {
            // 画面項目を初期値に設定
            textScanUUID.Text = "";
            textScanSec.Text = "";
            checkScanEnable.Checked = false;

            // 画面項目を使用不可とする
            textScanUUID.Enabled = false;
            textScanSec.Enabled = false;
            checkScanEnable.Enabled = false;

            // 設定書込・解除ボタンを押下不可とする
            buttonWrite.Enabled = false;
            buttonReset.Enabled = false;
        }

        private void SetupFieldValue()
        {
            // 画面項目に設定（スキャン対象サービスUUID）
            textScanUUID.Text = "422E0000-E141-11E5-A837-0800200C9A66";

            // 画面項目に設定（スキャン秒数）
            textScanSec.Text = "3";

            // 有効化チェックボックスを設定
            checkScanEnable.Checked = false;

            // 画面項目を使用可とする
            textScanUUID.Enabled = true;
            textScanSec.Enabled = true;
            checkScanEnable.Enabled = true;

            // 設定書込・解除ボタンを押下可とする
            buttonWrite.Enabled = true;
            buttonReset.Enabled = true;

            // 最初の項目（スキャン対象サービスUUID）にフォーカスを移動
            textScanUUID.Focus();
        }

        private void buttonRead_Click(object sender, EventArgs e)
        {
            // 自動認証用パラメーター照会コマンドを実行し、
            // スキャン対象サービスUUID、スキャン秒数を読込
            funcName = ToolGUICommon.MSG_LABEL_AUTH_PARAM_GET;
            mainForm.DoCommandToolPreference(sender, e);
        }

        private void buttonWrite_Click(object sender, EventArgs e)
        {
            // 入力チェックがNGの場合は終了
            if (CheckEntries() == false) {
                return;
            }

            // 処理続行確認ダイアログを開く
            string message = string.Format("{0}\n\n{1}",
                ToolGUICommon.MSG_WRITE_UUID_SCAN_PARAM,
                checkScanEnable.Checked ?
                    ToolGUICommon.MSG_PROMPT_WRITE_UUID_SCAN_PARAM_1 :
                    ToolGUICommon.MSG_PROMPT_WRITE_UUID_SCAN_PARAM_0
                );
            if (FormUtil.DisplayPromptPopup(message) == false) {
                return;
            }

            // スキャン対象サービスUUID、スキャン秒数を設定し、
            // 自動認証用パラメーター設定コマンドを実行
            funcName = ToolGUICommon.MSG_LABEL_AUTH_PARAM_SET;
            mainForm.DoCommandToolPreference(sender, e);
        }

        private void buttonReset_Click(object sender, EventArgs e)
        {
            // 処理続行確認ダイアログを開く
            string message = string.Format("{0}\n\n{1}",
                ToolGUICommon.MSG_CLEAR_UUID_SCAN_PARAM,
                ToolGUICommon.MSG_PROMPT_CLEAR_UUID_SCAN_PARAM);
            if (FormUtil.DisplayPromptPopup(message) == false) {
                return;
            }

            // 自動認証用パラメーター解除コマンドを実行
            funcName = ToolGUICommon.MSG_LABEL_AUTH_PARAM_RESET;
            mainForm.DoCommandToolPreference(sender, e);
        }

        private bool CheckEntries()
        {
            // スキャン対象サービスUUIDのチェック
            if (checkScanUUID() == false) {
                return false;
            }

            //
            // スキャン秒数のチェック
            //
            // 長さチェック
            if (FormUtil.checkEntrySize(textScanSec, 
                ToolGUICommon.AUTH_PARAM_UUID_SCAN_SEC_SIZE, ToolGUICommon.AUTH_PARAM_UUID_SCAN_SEC_SIZE, 
                ToolGUICommon.MSG_PROMPT_INPUT_UUID_SCAN_SEC_LEN) == false) {
                return false;
            }
            // 数字チェック
            if (FormUtil.checkIsNumeric(textScanSec, ToolGUICommon.MSG_PROMPT_INPUT_UUID_SCAN_SEC_NUM) == false) {
                return false;
            }
            // 範囲チェック
            if (FormUtil.checkValueInRange(textScanSec, 1, 9, 
                ToolGUICommon.MSG_PROMPT_INPUT_UUID_SCAN_SEC_RANGE) == false) {
                return false;
            }

            return true;
        }

        private bool checkScanUUID()
        {
            // チェックが付いていない場合で、UUIDがブランクであればチェック不要
            if (checkScanEnable.Checked == false) {
                if (textScanUUID.Text.Length == 0) {
                    return true;
                }
            }
            // 長さチェック
            if (FormUtil.checkEntrySize(textScanUUID,
                ToolGUICommon.AUTH_PARAM_UUID_STRING_SIZE, ToolGUICommon.AUTH_PARAM_UUID_STRING_SIZE,
                ToolGUICommon.MSG_PROMPT_INPUT_UUID_STRING_LEN) == false) {
                return false;
            }
            // 入力形式チェック（正規表現チェック）
            string pattern = "([0-9a-fA-F]{8}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{12})";
            if (FormUtil.checkValueWithPattern(textScanUUID, pattern, 
                ToolGUICommon.MSG_PROMPT_INPUT_UUID_STRING_PATTERN) == false) {
                return false;
            }

            return true;
        }

        // 
        // 自動認証設定コマンド実行完了時の処理
        //
        public void OnToolPreferenceCommandExecuted(bool success, string errMessage)
        {
            // コマンドの実行結果を表示
            string formatted = string.Format(ToolGUICommon.MSG_FORMAT_END_MESSAGE,
                funcName, 
                success ? ToolGUICommon.MSG_SUCCESS : ToolGUICommon.MSG_FAILURE);

            // 引数に格納されたエラーメッセージをポップアップ表示
            if (success) {
                // 取得したパラメーターを画面項目に設定し、設定書込・解除ボタンを押下可とする
                SetupFieldValue();
                // 読込成功時はポップアップ表示を省略
                if (funcName != ToolGUICommon.MSG_LABEL_AUTH_PARAM_GET) {
                    MessageBox.Show(this, formatted, MainForm.MaintenanceToolTitle);
                }

            } else {
                // 処理失敗時はメッセージをポップアップ表示
                MessageBox.Show(this, formatted, MainForm.MaintenanceToolTitle);
            }
        }
    }
}
