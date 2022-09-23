using System;
using System.Linq;
using ToolAppCommon;
using static MaintenanceToolApp.DFU.DFUParameter;

namespace MaintenanceToolApp.DFU
{
    internal class DFUTransferProcess
    {
        // このクラスのインスタンス
        private static readonly DFUTransferProcess Instance = new DFUTransferProcess();

        private DFUTransferProcess()
        {
        }

        // BLE SMPサービスの参照を保持（インスタンス生成は１度だけ行われる）
        private static readonly BLESMPService SMPService = new BLESMPService();

        // 上位クラス／パラメーターの参照を保持
        private DFUProcess DFUProcess = null!;
        private DFUParameter Parameter = null!;

        public static void InvokeTransferProcess(DFUProcess process, DFUParameter parameter)
        {
            // 上位クラス／パラメーターの参照を保持
            Instance.DFUProcess = process;
            Instance.Parameter = parameter;

            // 転送処理を起動
            Instance.InvokeDFUProcess();
        }

        //
        // 内部処理
        //
        private void InvokeDFUProcess()
        {
            // ステータスを更新
            Parameter.Status = DFUStatus.UploadProcess;

            // DFU処理開始時の画面処理
            int maximum = 100 + DFUProcessConst.DFU_WAITING_SEC_ESTIMATED;
            DFUProcess.NotifyDFUProcessStarting(maximum);

            // DFU本処理を開始（処理の終了は、処理進捗画面に通知される）
            // SMPサービスに接続
            SMPService.ConnectBLESMPService(OnConnectedToSMPService);
        }

        private void OnConnectedToSMPService(bool success)
        {
            if (success == false) {
                // 接続失敗時は転送処理を開始しない
                OnTerminatedDFUTransferProcess(false, AppCommon.MSG_DFU_PROCESS_CONNECT_FAILED);
                return;
            }

            // イベントを登録
            SMPService.OnDataReceived += OnDataReceived;
            SMPService.OnTransactionFailed += OnTransactionFailed;

            // 転送処理を開始する
            DoRequestGetSlotInfo();
        }

        private void OnTerminatedDFUTransferProcess(bool success, string message)
        {
            // BLE接続を破棄
            SMPService.DisconnectBLESMPService();

            if (success == false) {
                // エラーメッセージ文言を画面とログに出力
                Parameter.ErrorMessage = message;
                AppLogUtil.OutputLogError(message);
            }
            DFUProcess.OnTerminatedTransferProcess(success);

            // イベントを解除
            SMPService.OnDataReceived -= OnDataReceived;
            SMPService.OnTransactionFailed -= OnTransactionFailed;
        }

        //
        // スロット照会
        //
        private void DoRequestGetSlotInfo()
        {
            // DFU実行開始を通知
            DFUProcess.NotifyDFUProgress(AppCommon.MSG_DFU_PROCESS_TRANSFER_IMAGE, 0);

            // リクエストデータを生成
            byte[] bodyBytes = new byte[] { 0xbf, 0xff };
            ushort len = (ushort)bodyBytes.Length;
            byte[] headerBytes = DFUCommon.BuildSMPHeader(OP_READ_REQ, 0x00, len, GRP_IMG_MGMT, 0x00, CMD_IMG_MGMT_STATE);

            // リクエストデータを送信
            Parameter.Command = BLEDFUCommand.GetSlotInfo;
            SendSMPRequestData(bodyBytes, headerBytes);
        }

        private void DoResponseGetSlotInfo(byte[] responseData)
        {
            // スロット照会情報を参照し、チェックでNGの場合は以降の処理を行わない
            string errorMessage;
            if (CheckSlotInfo(responseData, out errorMessage) == false) {
                OnTerminatedDFUTransferProcess(false, errorMessage);
                return;
            }

            // 転送済みバイト数を事前にクリア
            Parameter.ImageBytesSent = 0;

            // 転送処理に移行
            DoRequestUploadImage();
        }

        private bool CheckSlotInfo(byte[] responseData, out string errorMessage)
        {
            // エラーメッセージを初期化
            errorMessage = AppCommon.MSG_NONE;

            // CBORをデコードしてスロット照会情報を抽出
            BLESMPCBORDecoder decoder = new BLESMPCBORDecoder();
            if (decoder.DecodeSlotInfo(responseData) == false) {
                errorMessage = AppCommon.MSG_DFU_SUB_PROCESS_FAILED;
                return false;
            }

            // スロット照会情報から、スロット#0のハッシュを抽出
            byte[] hashSlot = decoder.SlotInfos[0].Hash;

            // SHA-256ハッシュデータをイメージから抽出
            byte[] hashUpdate = Parameter.UpdateImageData.SHA256Hash;

            // スロット#0と転送対象イメージのハッシュを比較
            bool hashIsEqual = true;
            for (int i = 0; i < 32; i++) {
                if (hashSlot[i] != hashUpdate[i]) {
                    hashIsEqual = false;
                    break;
                }
            }

            // 既に転送対象イメージが導入されている場合は、画面／ログにその旨を出力し、処理を中止
            bool active = decoder.SlotInfos[0].Active;
            if (active && hashIsEqual) {
                errorMessage = AppCommon.MSG_DFU_IMAGE_ALREADY_INSTALLED;
                return false;
            }
            return true;
        }

        //
        // イメージ転送
        //
        private void DoRequestUploadImage()
        {
            // リクエストデータを生成
            byte[] bodyBytes = DFUCommon.GenerateBodyForRequestUploadImage(Parameter);
            ushort len = (ushort)bodyBytes.Length;
            byte[] headerBytes = DFUCommon.BuildSMPHeader(OP_WRITE_REQ, 0x00, len, GRP_IMG_MGMT, 0x00, CMD_IMG_MGMT_UPLOAD);

            // リクエストデータを送信
            Parameter.Command = BLEDFUCommand.UploadImage;
            SendSMPRequestData(bodyBytes, headerBytes);
        }

        private void DoResponseUploadImage(byte[] responseData)
        {
            // 処理進捗画面でCancelボタンが押下された時は、転送処理を終了し、BLE接続を切断
            if (Parameter.Status == DFUStatus.Canceled) {
                OnTerminatedDFUTransferProcess(false, AppCommon.MSG_NONE);
                return;
            }

            // 転送結果情報を参照し、チェックでNGの場合、BLE接続を切断
            string errorMessage;
            if (CheckUploadResultInfo(responseData, out errorMessage) == false) {
                OnTerminatedDFUTransferProcess(false, errorMessage);
                return;
            }

            // 転送比率を計算
            int imageBytesTotal = Parameter.UpdateImageData.NRF53AppBinSize;
            int percentage = Parameter.ImageBytesSent * 100 / imageBytesTotal;

            // 転送状況を画面表示
            string progressMessage = string.Format(AppCommon.MSG_DFU_PROCESS_TRANSFER_IMAGE_FORMAT, percentage);
            DFUProcess.NotifyDFUProgress(progressMessage, percentage);

            // イメージ全体が転送されたかどうかチェック
            if (Parameter.ImageBytesSent < imageBytesTotal) {
                // 転送中であることを通知
                DFUProcess.NotifyDFUTransferring(true);

                // 転送処理を続行
                DoRequestUploadImage();

            } else {
                // 転送が完了したことを通知
                DFUProcess.NotifyDFUTransferring(false);

                // 反映要求に移行
                DoRequestChangeImageUpdateMode();
            }
        }

        private bool CheckUploadResultInfo(byte[] responseData, out string errorMessage)
        {
            // メッセージの初期化
            errorMessage = AppCommon.MSG_NONE;

            // CBORをデコードして転送結果情報を抽出
            BLESMPCBORDecoder decoder = new BLESMPCBORDecoder();
            if (decoder.DecodeUploadResultInfo(responseData) == false) {
                errorMessage = AppCommon.MSG_DFU_SUB_PROCESS_FAILED;
                return false;
            }

            // 転送結果情報の rc が設定されている場合はエラー
            byte rc = decoder.ResultInfo.Rc;
            if (rc != 0) {
                errorMessage = string.Format(AppCommon.MSG_DFU_IMAGE_TRANSFER_FAILED_WITH_RC, rc);
                return false;
            }

            // 転送結果情報の off 値を転送済みバイト数に設定
            Parameter.ImageBytesSent = (int)decoder.ResultInfo.Off;
            return true;
        }

        //
        // 反映要求
        //
        private void DoRequestChangeImageUpdateMode()
        {
            // リクエストデータを生成
            byte[] bodyBytes = DFUCommon.GenerateBodyForRequestChangeImageUpdateMode(Parameter, DFUProcessConst.IMAGE_UPDATE_TEST_MODE);
            ushort len = (ushort)bodyBytes.Length;
            byte[] headerBytes = DFUCommon.BuildSMPHeader(OP_WRITE_REQ, 0x00, len, GRP_IMG_MGMT, 0x00, CMD_IMG_MGMT_STATE);

            // リクエストデータを送信
            Parameter.Command = BLEDFUCommand.ChangeImageUpdateMode;
            SendSMPRequestData(bodyBytes, headerBytes);
        }

        private void DoResponseChangeImageUpdateMode(byte[] responseData)
        {
            // スロット照会情報を参照し、チェックでNGの場合は以降の処理を行わない
            string errorMessage;
            if (CheckUploadedSlotInfo(responseData, out errorMessage) == false) {
                OnTerminatedDFUTransferProcess(false, errorMessage);
                return;
            }

            // DFU転送成功を通知
            AppLogUtil.OutputLogInfo(AppCommon.MSG_DFU_IMAGE_TRANSFER_SUCCESS);

            // リセット要求に移行
            DoRequestResetApplication();
        }

        private bool CheckUploadedSlotInfo(byte[] responseData, out string errorMessage)
        {
            // メッセージの初期化
            errorMessage = AppCommon.MSG_NONE;

            // CBORをデコードしてスロット照会情報を抽出
            BLESMPCBORDecoder decoder = new BLESMPCBORDecoder();
            if (decoder.DecodeSlotInfo(responseData) == false) {
                errorMessage = AppCommon.MSG_DFU_SUB_PROCESS_FAILED;
                return false;
            }

            // スロット情報の代わりに rc が設定されている場合はエラー
            byte rc = decoder.ResultInfo.Rc;
            if (rc != 0) {
                errorMessage = string.Format(AppCommon.MSG_DFU_IMAGE_INSTALL_FAILED_WITH_RC, rc);
                return false;
            }
            return true;
        }

        //
        // リセット要求
        //
        private void DoRequestResetApplication()
        {
            // リクエストデータを生成
            byte[] bodyBytes = new byte[] { 0xbf, 0xff };
            ushort len = (ushort)bodyBytes.Length;
            byte[] headerBytes = DFUCommon.BuildSMPHeader(OP_WRITE_REQ, 0x00, len, GRP_OS_MGMT, 0x00, CMD_OS_MGMT_RESET);

            // リクエストデータを送信
            Parameter.Command = BLEDFUCommand.ResetApplication;
            SendSMPRequestData(bodyBytes, headerBytes);
        }

        private void DoResponseResetApplication(byte[] responseData)
        {
            // DFU主処理の正常終了を通知
            OnTerminatedDFUTransferProcess(true, AppCommon.MSG_NONE);
        }

        //
        // データ送受信時の処理
        //
        private void SendSMPRequestData(byte[] requestBody, byte[] requestHeader)
        {
            // ヘッダーと本体を連結
            byte[] requestData = Enumerable.Concat(requestHeader, requestBody).ToArray();

            // リクエストデータを送信
            SMPService.Send(requestData);

            // ログ出力
            if (Parameter.Command != BLEDFUCommand.UploadImage) {
                string dump = AppLogUtil.DumpMessage(requestData, requestData.Length);
                AppLogUtil.OutputLogDebug(string.Format(
                    "Transmit SMP request ({0} bytes)\r\n{1}",
                    requestData.Length, dump));
            }
        }

        // 受信済みデータ／バイト数を保持
        private byte[] ResponseData = new byte[0];
        private int received = 0;
        private int totalSize = 0;

        private void OnDataReceived(byte[] receivedData)
        {
            // ログ出力
            if (Parameter.Command != BLEDFUCommand.UploadImage) {
                string dump = AppLogUtil.DumpMessage(receivedData, receivedData.Length);
                AppLogUtil.OutputLogDebug(string.Format(
                    "Incoming SMP response ({0} bytes)\r\n{1}",
                    receivedData.Length, dump));
            }

            // 受信したレスポンスデータを保持
            int responseSize = receivedData.Length;
            if (received == 0) {
                // レスポンスヘッダーからデータ長を抽出
                totalSize = DFUCommon.GetSMPResponseBodySize(receivedData);
                // 受信済みデータを保持
                received = responseSize - SMP_HEADER_SIZE;
                ResponseData = new byte[received];
                Array.Copy(receivedData, SMP_HEADER_SIZE, ResponseData, 0, received);

            } else {
                // 受信済みデータに連結
                received += responseSize;
                ResponseData = ResponseData.Concat(receivedData).ToArray();
            }

            // 全フレームを受信したら、レスポンス処理を実行
            if (received == totalSize) {
                // 処理区分に応じて分岐
                switch (Parameter.Command) {
                case BLEDFUCommand.GetSlotInfo:
                    DoResponseGetSlotInfo(ResponseData);
                    break;
                case BLEDFUCommand.UploadImage:
                    DoResponseUploadImage(ResponseData);
                    break;
                case BLEDFUCommand.ChangeImageUpdateMode:
                    DoResponseChangeImageUpdateMode(ResponseData);
                    break;
                case BLEDFUCommand.ResetApplication:
                    DoResponseResetApplication(ResponseData);
                    break;
                default:
                    break;
                }
                received = 0;
                totalSize = 0;
            }
        }

        //
        // 応答失敗時の処理
        //
        private void OnTransactionFailed(string errorBLEMessage)
        {
            // トランスポートのエラーメッセージをログ出力
            AppLogUtil.OutputLogError(errorBLEMessage);

            // 処理区分に応じて分岐
            string errorMessage = AppCommon.MSG_NONE;
            switch (Parameter.Command) {
            case BLEDFUCommand.GetSlotInfo:
                errorMessage = AppCommon.MSG_DFU_SLOT_INFO_GET_FAILED;
                break;
            case BLEDFUCommand.UploadImage:
                errorMessage = AppCommon.MSG_DFU_IMAGE_TRANSFER_FAILED;
                break;
            case BLEDFUCommand.ChangeImageUpdateMode:
                errorMessage = AppCommon.MSG_DFU_CHANGE_IMAGE_UPDATE_MODE_FAILED;
                break;
            case BLEDFUCommand.ResetApplication:
                errorMessage = AppCommon.MSG_DFU_RESET_APPLICATION_FAILED;
                break;
            default:
                break;
            }

            // 画面に異常終了を通知
            OnTerminatedDFUTransferProcess(false, errorMessage);
        }

        //
        // SMPトランザクションに関する定義
        //
        private const int OP_READ_REQ = 0;
        private const int OP_WRITE_REQ = 2;

        private const int GRP_IMG_MGMT = 1;
        private const int CMD_IMG_MGMT_STATE = 0;
        private const int CMD_IMG_MGMT_UPLOAD = 1;

        private const int GRP_OS_MGMT = 0;
        private const int CMD_OS_MGMT_RESET = 5;

        private const int SMP_HEADER_SIZE = 8;
    }
}
