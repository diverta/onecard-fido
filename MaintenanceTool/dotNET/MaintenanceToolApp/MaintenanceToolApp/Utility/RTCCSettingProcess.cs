using System;
using System.Text;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;
using static MaintenanceToolApp.FIDODefine;

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

        // HIDインターフェースからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        public RTCCSettingProcess(RTCCSettingParameter parameter)
        {
            // パラメーターの参照を保持
            Parameter = parameter;

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
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

            // 現在時刻を参照
            switch(Parameter.Command) {
            case Command.COMMAND_RTCC_SET_TIMESTAMP:
                DoRequestSetTimestamp();
                break;
            case Command.COMMAND_RTCC_GET_TIMESTAMP:
                DoRequestGetTimestamp();
                break;
            default:
                NotifyProcessTerminated(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                break;
            }
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
            // BLE接続を破棄
            if (Parameter.Transport == Transport.TRANSPORT_BLE) {
                BLEProcess.DisconnctBLE();
            }

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

        //
        // 現在時刻の設定
        //
        private void DoRequestSetTimestamp()
        {
            // HID経由で現在時刻を取得
            if (Parameter.Transport == Transport.TRANSPORT_HID) {
                // CTAPHID_INITから実行
                DoRequestCtapHidInit();
            }

            // BLE経由で現在時刻を設定
            if (Parameter.Transport == Transport.TRANSPORT_BLE) {
                // BLE接続試行
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoConnectBleCommand();
            }
        }

        private void DoRequestCtapHidInit()
        {
            // INITコマンドを実行し、nonce を送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidInit();
        }

        private void DoResponseCtapHidInit()
        {
            // CTAPHID_INIT応答後の処理を実行
            DoRequestHidSetTimestamp();
        }

        private void DoRequestHidSetTimestamp()
        {
            // HID経由で現在時刻を設定
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidCommand(0x80 | MNT_COMMAND_BASE, CommandDataForSetTimestamp());
        }

        private void DoRequestBleSetTimestamp()
        {
            // BLE経由で現在時刻を設定
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestBleCommand(0x80 | FIDO_CMD_MSG, CommandDataForSetTimestamp());
        }

        private byte[] CommandDataForSetTimestamp()
        {
            // 現在のUNIX時刻を取得
            TimeSpan t = DateTime.UtcNow - new DateTime(1970, 1, 1);
            UInt32 nowEpochSeconds = (UInt32)t.TotalSeconds;

            // 現在時刻設定用のリクエストデータを生成
            byte[] data = new byte[] { MNT_COMMAND_SET_TIMESTAMP, 0x00, 0x00, 0x00, 0x00 };
            AppUtil.ConvertUint32ToBEBytes(nowEpochSeconds, data, 1);
            return data;
        }

        //
        // 現在時刻の参照
        //
        private void DoRequestGetTimestamp()
        {
            // HID経由で現在時刻を取得
            if (Parameter.Transport == Transport.TRANSPORT_HID) {
                // コマンドバイトだけを送信する
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestCommand(0x80 | MNT_COMMAND_BASE, CommandDataForGetTimestamp());
            }
            // BLE経由で現在時刻を取得
            if (Parameter.Transport == Transport.TRANSPORT_BLE) {
                // コマンドバイトだけを送信する
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestBleCommand(0x80 | FIDO_CMD_MSG, CommandDataForGetTimestamp());
            }
        }

        private void DoResponseGetTimestamp(byte[] response)
        {
            // レスポンスの現在時刻を保持
            SaveTimestampParameter(response);

            // 現在時刻取得処理が正常終了
            NotifyProcessTerminated(true, AppCommon.MSG_NONE);
        }

        private void SaveTimestampParameter(byte[] response)
        {
            // 現在時刻文字列はレスポンスの２バイト目から19文字
            byte[] data = AppUtil.ExtractCBORBytesFromResponse(response, response.Length);

            // 認証器の現在時刻文字列を保持
            Parameter.DeviceTimestamp = Encoding.UTF8.GetString(data);

            // 管理ツールの現在時刻を取得して保持
            Parameter.ToolTimestamp = DateTime.Now.ToString("yyyy/MM/dd HH:mm:ss");
        }

        private byte[] CommandDataForGetTimestamp()
        {
            return new byte[] { MNT_COMMAND_GET_TIMESTAMP };
        }

        //
        // トランスポートからのレスポンス振分け処理
        //
        private void OnCommandResponse(byte CMD, byte[] responseData, bool success, string errorMessage)
        {
            // イベントを解除
            CommandProcess.UnregisterHandlerOnCommandResponse(OnCommandResponseRef);

            // 即時でアプリケーションに制御を戻す
            if (success == false) {
                NotifyProcessTerminated(false, errorMessage);
                return;
            }

            // BLE接続試行からの戻りの場合
            if (Parameter.Transport == Transport.TRANSPORT_BLE && CMD == 0x00) {
                DoRequestBleSetTimestamp();
                return;
            }

            // INITからの戻りの場合
            if (CMD == HIDProcessConst.HID_CMD_CTAPHID_INIT) {
                DoResponseCtapHidInit();
                return;
            }

            // レスポンスメッセージの１バイト目（ステータスコード）を確認
            if (responseData[0] != 0x00) {
                // エラーの場合は処理異常終了
                NotifyProcessTerminated(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            // 処理正常終了
            DoResponseGetTimestamp(responseData);
        }
    }
}
