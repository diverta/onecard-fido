using System;

namespace MaintenanceToolGUI
{
    public class ToolPreferenceParameter
    {
        // 自動認証設定関連のパラメーター
        public bool BleScanAuthEnabled { get; set; }
        public string ServiceUUIDString { get; set; }
        public string ServiceUUIDScanSec { get; set; }

        // 自動認証設定コマンドの種別
        public ToolPreference.CommandType CommandType { get; set; }
    }

    public class ToolPreference
    {
        // 画面の参照を保持
        private MainForm mainForm;
        private ToolPreferenceForm toolPreferenceForm;

        // 処理クラスの参照を保持
        private HIDMain hidMain;

        // 実行する自動認証設定コマンドの種別
        public enum CommandType
        {
            None = 0,
            COMMAND_AUTH_PARAM_GET,
            COMMAND_AUTH_PARAM_SET,
            COMMAND_AUTH_PARAM_RESET
        };

        public ToolPreference(MainForm f, HIDMain h)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // HID処理クラスの参照を保持
            hidMain = h;

            // ツール設定画面を生成
            toolPreferenceForm = new ToolPreferenceForm();
            toolPreferenceForm.SetToolPreferenceRef(this);
        }

        public void SetTitleAndVersionText(String toolName, String toolVersion)
        {
            // ツール名、バージョンを引き渡し
            toolPreferenceForm.SetTitleAndVersionText(toolName, toolVersion);
        }

        public void ShowDialog()
        {
            // ツール設定画面を表示
            toolPreferenceForm.ShowDialog();
        }

        public void DoCommandToolPreference(ToolPreferenceParameter parameter)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }
            // HID INITコマンドを実行
            hidMain.DoRequestCtapHidInitByToolPreference(this);
        }

        // 
        // INITコマンドの後続処理判定
        //
        public bool DoResponseHidInit(byte[] message, int length)
        {
            // 仮コード：コマンド実行完了時の処理
            toolPreferenceForm.OnToolPreferenceCommandExecuted(true, "");
            return true;
        }
    }
}
