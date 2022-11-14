using MaintenanceToolApp.CommonProcess;
using System;
using ToolAppCommon;
using static MaintenanceToolApp.DFU.DFUParameter;

namespace MaintenanceToolApp.DFU
{
    internal class USBDFUTransferParameter
    {
        public byte ObjectType;
        public int MaxCreateSize;
        public int AlreadySent;
        public int RemainingToSend;
        public int SizeToSend;
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
        private USBDFUTransferParameter TransferParameter = null!;

        // 転送用ユーティリティーの参照を保持
        private USBDFUTransferUtil TransferUtil = null!;

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
        private DFUProcess DFUProcess = null!;
        private DFUParameter Parameter = null!;

        public static void InvokeTransferProcess(DFUProcess process, DFUParameter parameter)
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
            int maximum = 100 + DFUProcessConst.USBDFU_WAITING_SEC_ESTIMATED;
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
                DFUProcess.OnTerminatedTransferProcess(success);
                return;
            }

            // 正常時は以降の処理を続行
            // USB再接続時のコールバックを設定
            HIDProcess.RegisterHandlerOnConnectHIDDevice(OnConnectHIDDeviceRef);

            // ステータスを更新（DFU反映待ち）
            Parameter.Status = DFUStatus.WaitForBoot;

            // DFU反映待ち処理を起動
            PerformDFUUpdateMonitor();
        }

        // 
        // DFU反映待ち処理
        // 
        private void PerformDFUUpdateMonitor()
        {
            // 処理進捗画面に通知
            DFUProcess.NotifyDFUProgress(AppCommon.MSG_DFU_PROCESS_WAITING_UPDATE, 100);

            // 反映待ち（リセットによるファームウェア再始動完了まで待機）
            for (int i = 0; i < DFUProcessConst.USBDFU_WAITING_SEC_ESTIMATED; i++) {
                // USB接続が検知された場合はループを終了
                if (Parameter.Status == DFUStatus.CheckUpdateVersion) {
                    break;
                }

                // 処理進捗画面に通知
                DFUProcess.NotifyDFUProgress(AppCommon.MSG_DFU_PROCESS_WAITING_UPDATE, 100 + i);
                System.Threading.Thread.Sleep(1000);
            }

            // 処理進捗画面に通知
            DFUProcess.NotifyDFUProgress(AppCommon.MSG_DFU_PROCESS_CONFIRM_VERSION, 100 + DFUProcessConst.USBDFU_WAITING_SEC_ESTIMATED);

            // ステータスを更新（バージョン更新判定）
            Parameter.Status = DFUStatus.CheckUpdateVersion;

            // バージョン情報照会処理に遷移
            DFUProcess.CheckUpdateVersionInfo();
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

            } else if (Parameter.Status == DFUStatus.WaitForBoot) {
                if (connected == false) {
                    // 画面に制御を戻す
                    TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_TARGET_NOT_NORMAL_MODE);

                } else {
                    // ファームウェア再始動完了
                    AppLogUtil.OutputLogDebug(AppCommon.MSG_DFU_TARGET_NORMAL_MODE);

                    // ステータスを変更
                    Parameter.Status = DFUStatus.CheckUpdateVersion;
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

            // 処理進捗画面に通知
            DFUProcess.NotifyDFUProgress(AppCommon.MSG_DFU_PROCESS_TRANSFER_IMAGE, 0);

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
            // パラメーターのインスタンスを生成
            TransferUtil = new USBDFUTransferUtil();

            // レスポンスからMTUを取得（４〜５バイト目、リトルエンディアン）
            int mtu = AppUtil.ToInt16(response, 3, false);
            TransferUtil.SetMTU(mtu);

            // DATイメージ転送処理の開始
            // １回あたりの送信データ最大長を取得
            DoRequestSelectObject(USBDFUConst.NRF_DFU_BYTE_OBJ_INIT_CMD);
        }

        //
        // イメージ転送処理（DAT／BIN）
        //
        private void DoRequestSelectObject(byte type)
        {
            // パラメーターのインスタンスを生成
            TransferParameter = new USBDFUTransferParameter();

            // 転送対象オブジェクトの区分（DAT／BIN）を保持
            TransferParameter.ObjectType = type;

            // SELECT OBJECTコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                USBDFUConst.NRF_DFU_OP_OBJECT_SELECT, TransferParameter.ObjectType, USBDFUConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (DFUService.SendDFURequest(b) == false) {
                TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_REQUEST_FAILED);
            }
        }

        private void DoResponseSelectObject(byte[] response)
        {
            // レスポンスからMaxCreateSizeを取得（4〜7バイト目、リトルエンディアン）
            TransferParameter.MaxCreateSize = AppUtil.ToInt32(response, 3, false);

            // データサイズを設定
            TransferParameter.AlreadySent = 0;
            if (TransferParameter.ObjectType == USBDFUConst.NRF_DFU_BYTE_OBJ_INIT_CMD) {
                TransferParameter.RemainingToSend = Parameter.UpdateImageData.NRF52AppDatSize;
            } else {
                TransferParameter.RemainingToSend = Parameter.UpdateImageData.NRF52AppBinSize;
            }

            // チェックサムを初期化
            TransferUtil.DFUObjectChecksumReset();

            // データ分割送信開始
            DoRequestCreateObject();
        }

        //
        // データ分割送信処理
        //
        private void DoRequestCreateObject()
        {
            // 送信すべきデータがない場合は終了
            if (TransferParameter.RemainingToSend < 1) {
                if (TransferParameter.ObjectType == USBDFUConst.NRF_DFU_BYTE_OBJ_INIT_CMD) {
                    // DATイメージ転送完了
                    AppLogUtil.OutputLogDebug("USB DFU: update init command object done");

                    // BINイメージ転送処理の開始
                    // １回あたりの送信データ最大長を取得
                    DoRequestSelectObject(USBDFUConst.NRF_DFU_BYTE_OBJ_DATA);

                } else if (TransferParameter.ObjectType == USBDFUConst.NRF_DFU_BYTE_OBJ_DATA) {
                    // BINイメージ転送完了
                    AppLogUtil.OutputLogDebug("USB DFU: update data object done");
                    TerminateDFUTransferProcess(true, AppCommon.MSG_NONE);
                }
                return;
            }

            // 送信サイズを通知
            TransferParameter.SizeToSend = (TransferParameter.MaxCreateSize < TransferParameter.RemainingToSend) ? TransferParameter.MaxCreateSize : TransferParameter.RemainingToSend;

            // CREATE OBJECTコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                USBDFUConst.NRF_DFU_OP_OBJECT_CREATE, TransferParameter.ObjectType, 0x00, 0x00, 0x00, 0x00,
                USBDFUConst.NRF_DFU_BYTE_EOM};
            int offset = 2;
            AppUtil.ConvertUint32ToLEBytes((UInt32)TransferParameter.SizeToSend, b, offset);

            // DFUリクエストを送信
            if (DFUService.SendDFURequest(b) == false) {
                TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_REQUEST_FAILED);
            }
        }

        private void DoResponseCreateObject(byte[] response)
        {
            // オブジェクト種別に対応するデータ／サイズを設定
            if (TransferParameter.ObjectType == USBDFUConst.NRF_DFU_BYTE_OBJ_INIT_CMD) {
                TransferUtil.DFUObjectFrameInit(Parameter.UpdateImageData.NRF52AppDat, TransferParameter.SizeToSend, TransferParameter.AlreadySent);
            } else {
                TransferUtil.DFUObjectFrameInit(Parameter.UpdateImageData.NRF52AppBin, TransferParameter.SizeToSend, TransferParameter.AlreadySent);
            }

            // データを送信
            DoRequestWriteCommandObject();
        }

        private void DoRequestWriteCommandObject()
        {
            // 送信フレームを生成
            while (TransferUtil.DFUObjectFramePrepare(TransferParameter.ObjectType)) {
                // DFUリクエスト（生成したフレーム）を送信
                if (DFUService.SendDFURequest(TransferUtil.GetPreparedFrame()) == false) {
                    TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_REQUEST_FAILED);
                    return;
                }
            }

            // 送信済みサイズを更新
            TransferParameter.AlreadySent += TransferParameter.SizeToSend;

            // 送信データのチェックサム検証に移る
            DoRequestGetCrc();
        }

        private void DoRequestGetCrc()
        {
            // CRC GETコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                USBDFUConst.NRF_DFU_OP_CRC_GET, USBDFUConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (DFUService.SendDFURequest(b) == false) {
                TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_REQUEST_FAILED);
            }
        }

        private void DoResponseGetCrc(byte[] response)
        {
            // レスポンスデータから、エスケープシーケンスを取り除く
            byte[] respUnesc = USBDFUTransferUtil.UnescapeResponseData(response);

            // レスポンスからデータ長を取得（4〜7バイト目、リトルエンディアン）
            int recvSize = AppUtil.ToInt32(respUnesc, 3, false);

            // 送信データ長を検証
            if (recvSize != TransferParameter.AlreadySent) {
                AppLogUtil.OutputLogError(string.Format("USB DFU: send object {0} failed (expected {1} bytes, recv {2} bytes)",
                    TransferParameter.ObjectType, TransferParameter.AlreadySent, recvSize));
                TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_VERIFY_SENDSIZE_FAILED);
                return;
            }

            // レスポンスからチェックサムを取得（8〜11バイト目、リトルエンディアン）
            UInt32 checksum = (UInt32)AppUtil.ToInt32(respUnesc, 7, false);

            // チェックサムを検証
            if (checksum != TransferUtil.DFUObjectChecksumGet()) {
                AppLogUtil.OutputLogError(string.Format("USB DFU: send object {0} failed (checksum error)",
                    TransferParameter.ObjectType));
                TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_VERIFY_CHECKSUM_FAILED);
                return;
            }

            // 送信データのチェックサム検証に移る
            DoRequestExecuteObject();
        }

        private void DoRequestExecuteObject()
        {
            // EXECUTE OBJECTコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                USBDFUConst.NRF_DFU_OP_OBJECT_EXECUTE, USBDFUConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (DFUService.SendDFURequest(b) == false) {
                TerminateDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_REQUEST_FAILED);
            }
        }

        private void DoResponseExecuteObject(byte[] response)
        {
            // 未送信サイズを更新
            TransferParameter.RemainingToSend -= TransferParameter.SizeToSend;

            // 処理進捗画面に通知
            if (TransferParameter.ObjectType == USBDFUConst.NRF_DFU_BYTE_OBJ_DATA) {
                // 転送比率を計算
                int imageBytesTotal = Parameter.UpdateImageData.NRF52AppBinSize;
                int percentage = TransferParameter.AlreadySent * 100 / imageBytesTotal;

                // 転送状況を画面表示
                string progressMessage = string.Format(AppCommon.MSG_DFU_PROCESS_TRANSFER_IMAGE_FORMAT, percentage);
                DFUProcess.NotifyDFUProgress(progressMessage, percentage);
            }

            // 次ブロックの送信処理に移る
            DoRequestCreateObject();
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
            case USBDFUConst.NRF_DFU_OP_OBJECT_SELECT:
                DoResponseSelectObject(response);
                break;
            case USBDFUConst.NRF_DFU_OP_OBJECT_CREATE:
                DoResponseCreateObject(response);
                break;
            case USBDFUConst.NRF_DFU_OP_CRC_GET:
                DoResponseGetCrc(response);
                break;
            case USBDFUConst.NRF_DFU_OP_OBJECT_EXECUTE:
                DoResponseExecuteObject(response);
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
