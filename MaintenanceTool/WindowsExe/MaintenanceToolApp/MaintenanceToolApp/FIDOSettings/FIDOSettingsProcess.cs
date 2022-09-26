using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.FIDOSettings
{
    public class FIDOSettingsParameter
    {
        public string CommandTitle { get; set; }
        public Command Command { get; set; }
        public string PinNew { get; set; }
        public string PinOld { get; set; }

        public FIDOSettingsParameter()
        {
            CommandTitle = string.Empty;
            Command = Command.COMMAND_NONE;
            PinNew = string.Empty;
            PinOld = string.Empty;
        }

        public override string ToString()
        {
            return string.Format("Command:{0} CommandTitle:{1} PinNew:{2} PinOld:{3}", Command, CommandTitle, PinNew, PinOld);
        }
    }

    public class FIDOSettingsProcess
    {
        // 処理実行のためのプロパティー
        private readonly FIDOSettingsParameter Parameter;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow = App.Current.MainWindow;

        public FIDOSettingsProcess(FIDOSettingsParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
        }

        public void DoProcess()
        {
            // TODO: 仮の実装です。
            AppLogUtil.OutputLogDebug(Parameter.ToString());

            // 実行コマンドにより処理分岐
            switch (Parameter.Command) {
            default:
                // エラーメッセージをポップアップ表示
                DialogUtil.ShowErrorMessage(ParentWindow, AppCommon.MSG_TOOL_TITLE, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
                break;
            }
        }
    }
}
