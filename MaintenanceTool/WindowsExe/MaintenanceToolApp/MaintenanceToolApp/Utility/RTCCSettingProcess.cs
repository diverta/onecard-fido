using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.Utility
{
    public class RTCCSettingParameter
    {
        public string CommandTitle { get; set; }
        public Command Command { get; set; }
        public bool CommandSuccess { get; set; }
        public string ResultMessage { get; set; }
        public string ResultInformativeMessage { get; set; }
        public Transport Transport { get; set; }
        //
        // 以下は処理生成中に設定
        //
        public string ToolTimestamp { get; set; }
        public string DeviceTimestamp { get; set; }

        public RTCCSettingParameter()
        {
            CommandTitle = string.Empty;
            Command = Command.COMMAND_NONE;
            ResultMessage = string.Empty;
            ResultInformativeMessage = string.Empty;
            Transport = Transport.TRANSPORT_NONE;
            ToolTimestamp = string.Empty;
            DeviceTimestamp = string.Empty;
        }
    }

    internal class RTCCSettingProcess
    {
        // 処理実行のためのプロパティー
        private RTCCSettingParameter Parameter = null!;

        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnNotifyProcessTerminated(RTCCSettingParameter parameter);
        private HandlerOnNotifyProcessTerminated OnNotifyProcessTerminated = null!;

        public RTCCSettingProcess(RTCCSettingParameter parameter)
        {
            // パラメーターの参照を保持
            Parameter = parameter;
        }

        //
        // 時刻設定用関数
        // 
        public void DoRTCCSettingProcess(HandlerOnNotifyProcessTerminated handlerRef) 
        {
            // タイムスタンプをクリア
            Parameter.ToolTimestamp = string.Empty;
            Parameter.DeviceTimestamp = string.Empty;

            // コールバックを保持
            OnNotifyProcessTerminated = handlerRef;

            // 処理開始を通知
            NotifyProcessStarted();

            // TODO: 仮の実装です。
            System.Threading.Thread.Sleep(2000);
            NotifyProcessTerminated(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
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
            OnNotifyProcessTerminated(Parameter);
        }
    }
}
