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

        //
        // 時刻設定用関数
        // 
        public void DoRTCCSettingProcess(RTCCSettingParameter parameter, HandlerOnNotifyProcessTerminated handlerRef) 
        {
            // 画面から引き渡されたパラメーターを退避
            Parameter = parameter;

            // コールバックを保持
            OnNotifyProcessTerminated = handlerRef;
        }
    }
}
