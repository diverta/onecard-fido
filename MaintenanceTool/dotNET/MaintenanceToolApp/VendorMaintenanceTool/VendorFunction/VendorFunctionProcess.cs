using MaintenanceToolApp;
using System.Threading;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace VendorMaintenanceTool.VendorFunction
{
    internal class VendorFunctionParameter
    {
        public Command Command { get; set; }
        public string CommandTitle { get; set; }
        public bool CommandSuccess { get; set; }
        public string ResultMessage { get; set; }
        public string ResultInformativeMessage { get; set; }

        public VendorFunctionParameter()
        {
            Command = Command.COMMAND_NONE;
            CommandTitle = string.Empty;
            ResultMessage = string.Empty;
            ResultInformativeMessage = string.Empty;
        }
    }

    internal class VendorFunctionProcess
    {
        // 処理実行のためのプロパティー
        private VendorFunctionParameter Parameter = null!;

        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnNotifyProcessTerminated();
        private HandlerOnNotifyProcessTerminated OnNotifyProcessTerminated = null!;

        public void DoProcess(VendorFunctionParameter parameter, HandlerOnNotifyProcessTerminated handlerRef)
        {
            // 画面から引き渡されたパラメーターを退避
            Parameter = parameter;

            // コールバックを保持
            OnNotifyProcessTerminated = handlerRef;

            // 処理開始を通知
            NotifyProcessStarted();

            // TODO: 仮の実装です。
            Thread.Sleep(2000);
            NotifyProcessTerminated(true, AppCommon.MSG_NONE);
        }

        // 
        // 共通処理
        //
        private void NotifyProcessStarted()
        {
            // コマンド開始メッセージをログファイルに出力
            string startMsg = string.Format(AppCommon.MSG_FORMAT_START_MESSAGE, Parameter.CommandTitle);
            AppLogUtil.OutputLogInfo(startMsg);
        }

        private void NotifyProcessTerminated(bool success, string errorMessage)
        {
            // エラーメッセージを画面＆ログ出力
            if (success == false && errorMessage.Length > 0) {
                // ログ出力する文言からは、改行文字を除去
                AppLogUtil.OutputLogError(AppUtil.ReplaceCRLF(errorMessage));
                Parameter.ResultInformativeMessage = errorMessage;
            }

            // コマンドの実行結果をログ出力
            string formatted = string.Format(AppCommon.MSG_FORMAT_END_MESSAGE,
                Parameter.CommandTitle,
                success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);
            if (success) {
                AppLogUtil.OutputLogInfo(formatted);
            } else {
                AppLogUtil.OutputLogError(formatted);
            }

            // パラメーターにコマンド成否を設定
            Parameter.CommandSuccess = success;
            Parameter.ResultMessage = formatted;

            // 画面に制御を戻す
            OnNotifyProcessTerminated();
        }
    }
}
