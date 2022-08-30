using MaintenanceToolApp.ToolAppCommon;
using System.Windows;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp
{
    public class HealthCheckParameter
    {
        public string CommandTitle { get; set; }
        public Command Command { get; set; }

        public HealthCheckParameter() 
        {
            CommandTitle = string.Empty;
            Command = Command.COMMAND_NONE;
        }
    }

    public class HealthCheckProcess
    {
        // 処理実行のためのプロパティー
        private readonly HealthCheckParameter Parameter;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow = App.Current.MainWindow;

        // HIDインターフェースからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        public HealthCheckProcess(HealthCheckParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
        }

        public void DoProcess()
        {
            // 実行コマンドにより処理分岐
            switch (Parameter.Command) {
            default:
                // エラーメッセージをポップアップ表示
                DialogUtil.ShowErrorMessage(ParentWindow, AppCommon.MSG_TOOL_TITLE, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
                break;
            }
        }

        //
        // HIDからのレスポンス振分け処理
        //
        private void OnCommandResponse(byte[] responseData, bool success, string errorMessage)
        {
            // イベントを解除
            CommandProcess.UnregisterHandlerOnCommandResponse(OnCommandResponseRef);

            // 即時でアプリケーションに制御を戻す
            if (success == false) {
                CommandProcess.NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, success, ParentWindow);
                return;
            }

            // レスポンスメッセージの１バイト目（ステータスコード）を確認
            if (responseData[0] != FIDODefine.CTAP1_ERR_SUCCESS) {
                // エラーの場合は画面に制御を戻す
                CommandProcess.NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false, ParentWindow);
                return;
            }

            // 実行コマンドにより処理分岐
            switch (Parameter.Command) {
            default:
                // メイン画面に制御を戻す
                CommandProcess.NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false, ParentWindow);
                break;
            }
        }
    }
}
