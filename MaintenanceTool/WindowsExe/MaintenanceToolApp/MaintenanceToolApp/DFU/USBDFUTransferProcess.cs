using ToolAppCommon;
using static MaintenanceToolApp.DFU.DFUParameter;

namespace MaintenanceToolApp.DFU
{
    internal class USBDFUTransferParameter
    {
        public int MTU;
    }

    internal class USBDFUTransferProcess
    {
        // このクラスのインスタンス
        private static readonly USBDFUTransferProcess Instance = new USBDFUTransferProcess();

        // HIDからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        // USB接続／切断時のコールバック参照
        private readonly HIDProcess.HandlerOnConnectHIDDevice OnConnectHIDDeviceRef;

        // CDC ACMからデータ受信時のコールバック参照
        private readonly USBDFUService.HandlerOnReceivedDFUResponse OnReceivedDFUResponseRef;

        // 処理パラメーターを保持
        private readonly USBDFUTransferParameter TransferParameter = new USBDFUTransferParameter();

        private USBDFUTransferProcess()
        {
            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
            OnConnectHIDDeviceRef = new HIDProcess.HandlerOnConnectHIDDevice(OnConnectHIDDevice);
            OnReceivedDFUResponseRef = new USBDFUService.HandlerOnReceivedDFUResponse(OnReceivedDFUResponse);
        }

        // USB DFUサービスの参照を保持（インスタンス生成は１度だけ行われる）
        private static readonly USBDFUService DFUService = new USBDFUService();

        // 上位クラス／パラメーターの参照を保持
        private USBDFUProcess DFUProcess = null!;
        private DFUParameter Parameter = null!;

        public static void InvokeTransferProcess(USBDFUProcess process, DFUParameter parameter)
        {
            // 上位クラス／パラメーターの参照を保持
            Instance.DFUProcess = process;
            Instance.Parameter = parameter;

            // 転送処理を起動
            Instance.InvokeDFUTransferProcess();
        }

        //
        // 内部処理
        //
        private void InvokeDFUTransferProcess()
        {
            // ステータスを更新
            Parameter.Status = DFUStatus.GetCurrentVersion;

            // DFU処理開始時の画面処理
            int maximum = 100;
            DFUProcess.NotifyDFUProcessStarting(maximum);

            // DFU本処理を開始（処理の終了は、処理進捗画面に通知される）
            // ブートローダーモード遷移コマンドを実行
            DoRequestCtapHidInit();
        }

        private void TerminateDFUTransferProcess(bool success, string message)
        {
            // イベントを解除
            DFUService.UnregisterHandlerOnReceivedResponse(OnReceivedDFUResponseRef);

            // CDC ACM接続を破棄
            DFUService.CloseDFUDevice();

            if (success == false) {
                // エラーメッセージ文言を画面とログに出力
                Parameter.ErrorMessage = message;
                AppLogUtil.OutputLogError(message);
            }
            DFUProcess.OnTerminatedTransferProcess(success);
        }

        //
        // INITコマンド関連処理
        //
        private void DoRequestCtapHidInit()
        {
            // INITコマンドを実行し、nonce を送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidInit();
        }

        private void DoResponseCtapHidInit()
        {
            // CTAPHID_INIT応答後の処理を実行
            DoRequestCommandBootloaderMode();
        }

        //
        // ブートローダーモード遷移処理
        //
        private void DoRequestCommandBootloaderMode()
        {
            // コマンドバイトだけを送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidCommand(HIDProcessConst.HID_CMD_BOOTLOADER_MODE, System.Array.Empty<byte>());
        }

        public void DoResponseCommandBootloaderMode(byte CMD, byte[] responseData)
        {
            if (CMD == HIDProcessConst.HID_CMD_BOOTLOADER_MODE) {
                // ブートローダーモード遷移コマンド成功時
                Parameter.Status = DFUStatus.ToBootloaderMode;
                HIDProcess.RegisterHandlerOnConnectHIDDevice(OnConnectHIDDeviceRef);

            } else {
                // ブートローダーモード遷移コマンド失敗時は、
                // 画面に制御を戻す
                TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_TARGET_NOT_BOOTLOADER_MODE);
            }
        }

        //
        // HIDからのレスポンス振分け処理
        //
        private void OnCommandResponse(byte CMD, byte[] responseData, bool success, string errorMessage)
        {
            // イベントを解除
            CommandProcess.UnregisterHandlerOnCommandResponse(OnCommandResponseRef);

            // 即時でアプリケーションに制御を戻す
            if (success == false) {
                TerminateDFUTransferProcess(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            // INITからの戻りの場合
            if (CMD == HIDProcessConst.HID_CMD_CTAPHID_INIT) {
                DoResponseCtapHidInit();
                return;
            }

            // ブートローダーモード遷移コマンド実行後の処理
            DoResponseCommandBootloaderMode(CMD, responseData);
        }

        //
        // USB接続／切断時の処理
        //
        private void OnConnectHIDDevice(bool connected)
        {
            // イベントを解除
            HIDProcess.UnregisterHandlerOnConnectHIDDevice(OnConnectHIDDeviceRef);

            if (Parameter.Status == DFUStatus.ToBootloaderMode) {
                if (connected) {
                    // 画面に制御を戻す
                    TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_TARGET_NOT_BOOTLOADER_MODE);

                } else {
                    // ブートローダーモード遷移完了 --> CDC ACM接続処理に移行
                    AppLogUtil.OutputLogDebug(AppCommon.MSG_DFU_TARGET_BOOTLOADER_MODE);
                    EstablishDFUConnection();
                }

            } else {
                // 画面に制御を戻す
                TerminateDFUTransferProcess(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
            }
        }

        //
        // 認証器へのCDC ACM接続処理
        //
        public void EstablishDFUConnection()
        {
            // DFU対象デバイスに接続（USB CDC ACM接続）
            USBDFUService.HandlerOnConnectedToDFUService handler = new USBDFUService.HandlerOnConnectedToDFUService(OnConnectedToDFUService);
            DFUService.ConnectUSBDFUService(handler);
        }

        private void OnConnectedToDFUService(bool success)
        {
            if (success == false) {
                // 接続失敗時は転送処理を開始しない
                TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_CONNECT_FAILED);
                return;
            }

            // イベントを登録
            DFUService.RegisterHandlerOnReceivedResponse(OnReceivedDFUResponseRef);

            // DFU対象デバイスの通知設定
            DoRequestSetReceipt();
        }

        //
        // 転送準備処理
        //
        private void DoRequestSetReceipt()
        {
            // SET RECEIPTコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                USBDFUConst.NRF_DFU_OP_RECEIPT_NOTIF_SET, 0x00, 0x00, USBDFUConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (DFUService.SendDFURequest(b) == false) {
                TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_REQUEST_FAILED);
            }
        }

        private void DoResponseSetReceipt(byte[] response)
        {
            // DFU対象デバイスからMTUを取得
            DoRequestGetMtu();
        }

        private void DoRequestGetMtu()
        {
            // GET MTUコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                USBDFUConst.NRF_DFU_OP_MTU_GET, USBDFUConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (DFUService.SendDFURequest(b) == false) {
                TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_REQUEST_FAILED);
            }
        }

        private void DoResponseGetMtu(byte[] response)
        {
            // レスポンスからMTUを取得（4〜5バイト目、リトルエンディアン）
            TransferParameter.MTU = AppUtil.ToInt16(response, 3, false);

            // TODO: 仮の実装です。
            TerminateDFUTransferProcess(true, AppCommon.MSG_NONE);
        }

        //
        // DFUレスポンス受信時の処理
        //
        public void OnReceivedDFUResponse(bool success, byte[] response)
        {
            // コマンドバイト（レスポンスの２バイト目）を取得
            byte cmd = response[1];

            // 失敗時はメイン画面に制御を戻す
            if (success == false) {
                TerminateDFUTransferProcess(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            // レスポンスがNGの場合は処理終了
            if (AssertDFUResponseSuccess(response) == false) {
                TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_RESPONSE_FAILED);
            }

            // コマンドバイトで処理分岐
            switch (cmd) {
            case USBDFUConst.NRF_DFU_OP_RECEIPT_NOTIF_SET:
                DoResponseSetReceipt(response);
                break;
            case USBDFUConst.NRF_DFU_OP_MTU_GET:
                DoResponseGetMtu(response);
                break;
            default:
                break;
            }
        }

        private bool AssertDFUResponseSuccess(byte[] response)
        {
            // レスポンスを検証
            if (response == null || response.Length == 0) {
                return false;
            }

            // ステータスコードを参照し、処理が成功したかどうかを判定
            return (response[2] == USBDFUConst.NRF_DFU_BYTE_RESP_SUCCESS);
        }
    }
}
