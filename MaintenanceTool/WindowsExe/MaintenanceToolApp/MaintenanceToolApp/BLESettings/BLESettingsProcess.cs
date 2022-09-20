using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.BLESettings
{
    public class BLESettingsParameter
    {
        public string CommandTitle { get; set; }
        public Command Command { get; set; }
        public ulong BluetoothAddress { get; set; }
        public string Passcode { get; set; }
        public string ErrorMessage { get; set; }

        public BLESettingsParameter()
        {
            CommandTitle = string.Empty;
            Command = Command.COMMAND_NONE;
            BluetoothAddress = 0;
            Passcode = string.Empty;
            ErrorMessage = string.Empty;
        }

        public override string ToString()
        {
            return string.Format("Command:{0} CommandTitle:{1} Passcode:{2}", Command, CommandTitle, Passcode);
        }
    }

    public class BLESettingsProcess
    {
        // 処理実行のためのプロパティー
        private readonly BLESettingsParameter Parameter;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow = App.Current.MainWindow;

        public BLESettingsProcess(BLESettingsParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
        }

        public void DoProcess()
        {
            // 実行コマンドにより処理分岐
            switch (Parameter.Command) {
            case Command.COMMAND_PAIRING:
                Parameter.CommandTitle = AppCommon.PROCESS_NAME_PAIRING;
                CommandProcess.NotifyCommandStarted(Parameter.CommandTitle);
                DoRequestPairing();
                break;

            case Command.COMMAND_ERASE_BONDS:
                Parameter.CommandTitle = AppCommon.PROCESS_NAME_ERASE_BONDS;
                CommandProcess.NotifyCommandStarted(Parameter.CommandTitle);
                DoRequestEraseBonds();
                break;

            default:
                // エラーメッセージをポップアップ表示
                DialogUtil.ShowErrorMessage(ParentWindow, AppCommon.MSG_TOOL_TITLE, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
                break;
            }
        }

        private void DoRequestPairing()
        {
            new BLEPairingProcess(Parameter).DoRequestPairing(DoResponseFromSubProcess);
        }

        private void DoRequestEraseBonds()
        {
            new UnpairingProcess(Parameter).DoRequestEraseBonds(DoResponseFromSubProcess);
        }

        //
        // 下位クラスからのコールバック
        //
        private void DoResponseFromSubProcess(string commandTitle, string errorMessage, bool success)
        {
            // 失敗時はログ出力
            if (success == false && errorMessage.Length > 0) {
                AppLogUtil.OutputLogError(errorMessage);
            }

            // メイン画面に制御を戻す
            CommandProcess.NotifyCommandTerminated(commandTitle, errorMessage, success, ParentWindow);
        }
    }
}
