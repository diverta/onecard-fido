using MaintenanceToolApp.Common;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.CommonProcess
{
    public class VersionInfoData
    {
        public string FWRev { get; set; }
        public string HWRev { get; set; }

        public VersionInfoData(string fWRev, string hWRev)
        {
            FWRev = fWRev;
            HWRev = hWRev;
        }
    }

    internal class VersionInfoProcess
    {
        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(bool success, string errorMessage, VersionInfoData versionInfoData);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;

        // 戻り先の関数を保持
        private HandlerOnNotifyCommandTerminated HandlerRef = null!;

        // BLEからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        //
        // 外部公開用
        //
        public VersionInfoProcess()
        {
            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
        }

        public void DoRequestVersionInfo(Transport transport, HandlerOnNotifyCommandTerminated handler)
        {
            // 戻り先の関数を保持
            HandlerRef = handler;
            NotifyCommandTerminated += HandlerRef;

            // バージョン照会コマンドを実行
            if (transport == Transport.TRANSPORT_BLE) {
                DoRequestBLEGetVersionInfo();

            } else {
                DoRequestHIDGetVersionInfo();
            }
        }

        //
        // 内部処理
        //
        private void DoRequestBLEGetVersionInfo()
        {
            // コマンドバイトだけを送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestBleCommand(HIDProcessConst.HID_CMD_GET_VERSION_INFO, new byte[1]);
        }

        private void DoRequestHIDGetVersionInfo()
        {
            // コマンドバイトだけを送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCommand(HIDProcessConst.HID_CMD_GET_VERSION_INFO, new byte[0]);
        }

        private void DoResponseGetVersionInfo(byte[] responseData)
        {
            // BLE接続を破棄
            BLEProcess.DisconnctBLE();
            
            // ステータスバイトをチェック
            string statusMessage;
            if (CTAP2Util.CheckStatusByte(responseData, out statusMessage) == false) {
                // 処理結果が不正の場合は画面に制御を戻す
                DoNotifyCommandTerminated(false, statusMessage, null!);
                return;
            }

            // レスポンスされたCBORを抽出
            byte[] cborBytes = AppUtil.ExtractCBORBytesFromResponse(responseData, responseData.Length);

            // 取得情報CSVを抽出
            string responseCSV = System.Text.Encoding.ASCII.GetString(cborBytes);

            // 情報取得CSVからバージョンに関する情報を抽出
            string[] vars = responseCSV.Split(',');
            string strFWRev = "";
            string strHWRev = "";
            foreach (string v in vars) {
                if (v.StartsWith("FW_REV=")) {
                    strFWRev = v.Split('=')[1].Replace("\"", "");
                } else if (v.StartsWith("HW_REV=")) {
                    strHWRev = v.Split('=')[1].Replace("\"", "");
                }
            }

            // 上位クラスに制御を戻す
            DoNotifyCommandTerminated(true, AppCommon.MSG_NONE, new VersionInfoData(strFWRev, strHWRev));
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
                DoNotifyCommandTerminated(success, errorMessage, null!);
                return;
            }

            // バージョン情報照会結果
            DoResponseGetVersionInfo(responseData);
        }

        //
        // コマンド終了時の処理
        //
        private void DoNotifyCommandTerminated(bool success, string errorMessage, VersionInfoData versionInfoData)
        {
            // バージョン情報、エラーメッセージを戻す
            NotifyCommandTerminated(success, errorMessage, versionInfoData);

            // 呼出元クラスの関数コールバックを解除
            NotifyCommandTerminated -= HandlerRef;
        }
    }
}
