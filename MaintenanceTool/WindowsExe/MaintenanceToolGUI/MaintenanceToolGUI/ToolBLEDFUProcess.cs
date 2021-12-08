using MaintenanceToolCommon;
using System;
using System.Linq;
using System.Security.Cryptography;

namespace MaintenanceToolGUI
{
    public class ToolBLEDFUProcess
    {
        // SMPトランザクションに関する定義
        private const int OP_READ_REQ = 0;
        private const int OP_READ_RES = 1;
        private const int OP_WRITE_REQ = 2;
        private const int OP_WRITE_RES = 3;

        private const int GRP_IMG_MGMT = 1;
        private const int CMD_IMG_MGMT_STATE = 0;
        private const int CMD_IMG_MGMT_UPLOAD = 1;

        private const int GRP_OS_MGMT = 0;
        private const int CMD_OS_MGMT_RESET = 5;

        private const int SMP_HEADER_SIZE = 8;

        private const bool IMAGE_UPDATE_TEST_MODE = true;

        // 処理区分
        private enum BLEDFUCommand
        {
            GetSlotInfo = 0,
            UploadImage,
            ChangeImageUpdateMode,
            ResetApplication,
        };
        private BLEDFUCommand Command;

        // 転送元データを保持
        byte[] ImageDataTotal;

        // 転送済みバイト数を保持
        private int ImageBytesSent = 0;

        // 処理進捗画面でCancelボタンが押下されたら true
        public bool CancelFlag { get; set; }

        // クラスの参照を保持
        private ToolBLEDFUImage ToolBLEDFUImageRef;
        private ToolBLESMPService ToolBLESMPService;

        // DFU転送処理のイベントを定義
        public delegate void NotifyDFUProgressEvent(string message, int progressValue);
        public event NotifyDFUProgressEvent OnNotifyDFUProgress;

        public delegate void NotifyDFUInfoMessageEvent(string message);
        public event NotifyDFUInfoMessageEvent OnNotifyDFUInfoMessage;

        public delegate void NotifyDFUErrorMessageEvent(string message);
        public event NotifyDFUErrorMessageEvent OnNotifyDFUErrorMessage;

        public delegate void NotifyDFUTransferEvent(bool transferring);
        public event NotifyDFUTransferEvent OnNotifyDFUTransfer;

        public delegate void TerminatedDFUProcessEvent(bool success);
        public event TerminatedDFUProcessEvent OnTerminatedDFUProcess;

        // 応答タイムアウト監視用タイマー
        private CommandTimer responseTimer = null;

        public ToolBLEDFUProcess(ToolBLEDFUImage imageRef)
        {
            // クラスの参照を保持
            ToolBLEDFUImageRef = imageRef;
            ToolBLESMPService = new ToolBLESMPService();

            // BLE SMPサービスのイベントを登録
            ToolBLESMPService.OnConnected += new ToolBLESMPService.ConnectedEvent(OnConnected);
            ToolBLESMPService.OnConnectionFailed += new ToolBLESMPService.ConnectionFailedEvent(OnConnectionFailed);
            ToolBLESMPService.OnDataReceived += new ToolBLESMPService.DataReceivedEvent(OnDataReceived);
            ToolBLESMPService.OnTransactionFailed += new ToolBLESMPService.TransactionFailedEvent(OnTransactionFailed);

            // 応答タイムアウト発生時のイベントを登録
            responseTimer = new CommandTimer("ToolBLEDFUProcess", 10000);
            responseTimer.CommandTimeoutEvent += OnResponseTimerElapsed;
        }

        public void PerformDFU()
        {
            // BLE SMPサービスに接続
            DoConnect();
        }

        private void TerminateDFUProcess(bool success)
        {
            // メイン画面にDFU処理の終了を通知
            OnTerminatedDFUProcess(success);
        }

        //
        // 接続／切断処理
        // 
        private void DoConnect()
        {
            // BLE SMPサービスに接続
            ToolBLESMPService.ConnectBLESMPService();
        }

        private void OnConnectionFailed()
        {
            // 画面に異常終了を通知
            TerminateDFUProcess(false);
        }

        private void OnConnected()
        {
            // スロット照会実行からスタート
            DoRequestGetSlotInfo();
        }

        public void DoDisconnect()
        {
            // BLE SMPサービスから切断
            ToolBLESMPService.DisconnectBLESMPService();
        }

        //
        // スロット照会
        //
        private void DoRequestGetSlotInfo()
        {
            // DFU実行開始を通知
            OnNotifyDFUProgress(ToolGUICommon.MSG_DFU_PROCESS_TRANSFER_IMAGE, 0);

            // リクエストデータを生成
            byte[] bodyBytes = new byte[] { 0xbf, 0xff };
            ushort len = (ushort)bodyBytes.Length;
            byte[] headerBytes = BuildSMPHeader(OP_READ_REQ, 0x00, len, GRP_IMG_MGMT, 0x00, CMD_IMG_MGMT_STATE);

            // リクエストデータを送信
            Command = BLEDFUCommand.GetSlotInfo;
            SendSMPRequestData(bodyBytes, headerBytes);
        }

        private void DoResponseGetSlotInfo(byte[] responseData)
        {
            // スロット照会情報を参照し、チェックでNGの場合は以降の処理を行わない
            if (CheckSlotInfo(responseData) == false) {
                TerminateDFUProcess(false);
                return;
            }

            // 転送元データを抽出
            ImageDataTotal = ToolBLEDFUImageRef.NRF53AppBin.Take(ToolBLEDFUImageRef.NRF53AppBinSize).ToArray();

            // 転送済みバイト数を事前にクリア
            ImageBytesSent = 0;

            // 転送処理に移行
            DoRequestUploadImage();
        }

        private bool CheckSlotInfo(byte[] responseData)
        {
            // CBORをデコードしてスロット照会情報を抽出
            BLESMPCBORDecoder decoder = new BLESMPCBORDecoder();
            if (decoder.DecodeSlotInfo(responseData) == false) {
                AppCommon.OutputLogError(ToolGUICommon.MSG_DFU_SUB_PROCESS_FAILED);
                return false;
            }

            // スロット照会情報から、スロット#0のハッシュを抽出
            byte[] hashSlot = decoder.SlotInfos[0].Hash;

            // SHA-256ハッシュデータをイメージから抽出
            byte[] hashUpdate = ToolBLEDFUImageRef.SHA256Hash;

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
                OnNotifyDFUErrorMessage(ToolGUICommon.MSG_DFU_IMAGE_ALREADY_INSTALLED);
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
            byte[] bodyBytes = GenerateBodyForRequestUploadImage();
            ushort len = (ushort)bodyBytes.Length;
            byte[] headerBytes = BuildSMPHeader(OP_WRITE_REQ, 0x00, len, GRP_IMG_MGMT, 0x00, CMD_IMG_MGMT_UPLOAD);

            // リクエストデータを送信
            Command = BLEDFUCommand.UploadImage;
            SendSMPRequestData(bodyBytes, headerBytes);
        }

        private void DoResponseUploadImage(byte[] responseData)
        {
            // 処理進捗画面でCancelボタンが押下された時は、転送処理を終了し、BLE接続を切断
            if (CancelFlag) {
                TerminateDFUProcess(false);
                return;
            }

            // 転送結果情報を参照し、チェックでNGの場合、BLE接続を切断
            if (CheckUploadResultInfo(responseData) == false) {
                TerminateDFUProcess(false);
                return;
            }

            // 転送比率を計算
            int imageBytesTotal = ToolBLEDFUImageRef.NRF53AppBinSize;
            int percentage = ImageBytesSent * 100 / imageBytesTotal;
            AppCommon.OutputLogDebug(string.Format("DFU image sent {0} bytes ({1}%)", ImageBytesSent, percentage));

            // 転送状況を画面表示
            string progressMessage = string.Format(ToolGUICommon.MSG_DFU_PROCESS_TRANSFER_IMAGE_FORMAT, percentage);
            OnNotifyDFUProgress(progressMessage, percentage);

            // イメージ全体が転送されたかどうかチェック
            if (ImageBytesSent < imageBytesTotal) {
                // 転送中であることを通知
                OnNotifyDFUTransfer(true);
                // 転送処理を続行
                DoRequestUploadImage();

            } else {
                // 転送が完了したことを通知
                OnNotifyDFUTransfer(false);
                // 反映要求に移行
                DoRequestChangeImageUpdateMode();
            }
        }

        private byte[] GenerateBodyForRequestUploadImage()
        {
            // リクエストデータ
            byte[] body = { 0xbf };

            // 転送元データ長
            uint bytesTotal = (uint)ImageDataTotal.Length;

            if (ImageBytesSent == 0) {
                // 初回呼び出しの場合、イメージ長を設定
                body = body.Concat(GenerateLenBytes(bytesTotal)).ToArray();

                // イメージのハッシュ値を設定
                body = body.Concat(GenerateSHA256HashData(ImageDataTotal)).ToArray();
            }

            // 転送済みバイト数を設定
            body = body.Concat(GenerateOffBytes(ImageBytesSent)).ToArray();

            // 転送イメージを連結（データ本体が240バイトに収まるよう、上限サイズを事前計算）
            int remainingSize = 240 - body.Length - 1;
            body = body.Concat(GenerateDataBytes(ImageDataTotal, ImageBytesSent, remainingSize)).ToArray();

            // 終端文字を設定して戻す
            byte[] terminator = { 0xff };
            return body.Concat(terminator).ToArray();
        }

        private byte[] GenerateLenBytes(uint bytesTotal)
        {
            // イメージ長を設定
            byte[] lenBytes = {
                0x63, 0x6c, 0x65, 0x6e, 0x1a, 0x00, 0x00, 0x00, 0x00
            };
            AppCommon.ConvertUint32ToBEBytes(bytesTotal, lenBytes, 5);
            return lenBytes;
        }

        private byte[] GenerateSHA256HashData(byte[] data)
        {
            // イメージのハッシュ値を計算
            SHA256 sha = new SHA256CryptoServiceProvider();
            byte[] hash = sha.ComputeHash(data);

            // イメージのハッシュ値を設定
            byte[] shaBytes = {
                0x63, 0x73, 0x68, 0x61, 0x43, 0x00, 0x00, 0x00,
            };

            // 指定領域から３バイト分の領域に、SHA-256ハッシュの先頭３バイト分を設定
            for (int i = 0; i < 3; i++) {
                shaBytes[i + 5] = hash[i];
            }
            return shaBytes;
        }

        private byte[] GenerateOffBytes(int bytesSent)
        {
            // 転送済みバイト数を設定
            byte[] offBytes = {
                0x63, 0x6f, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00
            };
            int len = offBytes.Length;
            if (bytesSent == 0) {
                len = 5;

            } else if (bytesSent < 0x100) {
                offBytes[4] = 0x18;
                offBytes[5] = (byte)bytesSent;
                len = 6;

            } else if (bytesSent < 0x10000) {
                offBytes[4] = 0x19;
                AppCommon.ConvertUint16ToBEBytes((UInt16)bytesSent, offBytes, 5);
                len = 7;

            } else {
                offBytes[4] = 0x1a;
                AppCommon.ConvertUint32ToBEBytes((UInt32)bytesSent, offBytes, 5);
            }

            // 不要な末尾バイトを除去して戻す
            byte[] offData = offBytes.Take(len).ToArray();
            return offData;
        }

        private byte[] GenerateDataBytes(byte[] imageData, int bytesSent, int remaining)
        {
            // 転送バイト数を設定
            byte[] bodyBytes = {
                0x64, 0x64, 0x61, 0x74, 0x61, 0x58, 0x00
            };

            // 転送バイト数
            int bytesToSend = remaining - bodyBytes.Length;
            if (bytesToSend > imageData.Length - bytesSent) {
                bytesToSend = imageData.Length - bytesSent;
            }
            bodyBytes[6] = (byte)bytesToSend;

            // 転送イメージを抽出
            byte[] sendData = imageData.Skip(bytesSent).Take(bytesToSend).ToArray();

            // 転送イメージを連結して戻す
            return bodyBytes.Concat(sendData).ToArray();
        }

        private bool CheckUploadResultInfo(byte[] responseData)
        {
            // CBORをデコードして転送結果情報を抽出
            BLESMPCBORDecoder decoder = new BLESMPCBORDecoder();
            if (decoder.DecodeUploadResultInfo(responseData) == false) {
                AppCommon.OutputLogError(ToolGUICommon.MSG_DFU_SUB_PROCESS_FAILED);
                return false;
            }

            // 転送結果情報の rc が設定されている場合はエラー
            byte rc = decoder.ResultInfo.Rc;
            if (rc != 0) {
                AppCommon.OutputLogError(string.Format(ToolGUICommon.MSG_DFU_IMAGE_TRANSFER_FAILED_WITH_RC, rc));
                return false;
            }

            // 転送結果情報の off 値を転送済みバイト数に設定
            ImageBytesSent = (int)decoder.ResultInfo.Off;
            return true;
        }

        //
        // 反映要求
        //
        private void DoRequestChangeImageUpdateMode()
        {
            // リクエストデータを生成
            byte[] bodyBytes = GenerateBodyForRequestChangeImageUpdateMode(IMAGE_UPDATE_TEST_MODE);
            ushort len = (ushort)bodyBytes.Length;
            byte[] headerBytes = BuildSMPHeader(OP_WRITE_REQ, 0x00, len, GRP_IMG_MGMT, 0x00, CMD_IMG_MGMT_STATE);

            // リクエストデータを送信
            Command = BLEDFUCommand.ChangeImageUpdateMode;
            SendSMPRequestData(bodyBytes, headerBytes);
        }

        private void DoResponseChangeImageUpdateMode(byte[] responseData)
        {
            // スロット照会情報を参照し、チェックでNGの場合は以降の処理を行わない
            if (CheckUploadedSlotInfo(responseData) == false) {
                TerminateDFUProcess(false);
                return;
            }

            // DFU転送成功を通知
            OnNotifyDFUInfoMessage(ToolGUICommon.MSG_DFU_IMAGE_TRANSFER_SUCCESS);

            // TODO: 仮の実装です。
            System.Threading.Thread.Sleep(1000);
            TerminateDFUProcess(false);
        }

        private byte[] GenerateBodyForRequestChangeImageUpdateMode(bool imageUpdateTestMode)
        {
            // リクエストデータ
            byte[] body = {
                0xbf, 0x67, 0x63, 0x6f, 0x6e, 0x66, 0x69, 0x72, 0x6d, 0x00,
                0x64, 0x68, 0x61, 0x73, 0x68, 0x58, 0x20
            };

            // イメージ反映モードを設定（confirm=false/true）
            if (imageUpdateTestMode) {
                body[9] = 0xf4;
            } else {
                body[9] = 0xf5;
            }

            // SHA-256ハッシュデータをイメージから抽出
            byte[] hashUpdate = ToolBLEDFUImageRef.SHA256Hash;

            // 本体にSHA-256ハッシュを連結
            body = body.Concat(hashUpdate).ToArray();

            // 終端文字を設定して戻す
            byte[] terminator = { 0xff };
            return body.Concat(terminator).ToArray();
        }

        private bool CheckUploadedSlotInfo(byte[] responseData)
        {
            // CBORをデコードしてスロット照会情報を抽出
            BLESMPCBORDecoder decoder = new BLESMPCBORDecoder();
            if (decoder.DecodeSlotInfo(responseData) == false) {
                AppCommon.OutputLogError(ToolGUICommon.MSG_DFU_SUB_PROCESS_FAILED);
                return false;
            }

            // スロット情報の代わりに rc が設定されている場合はエラー
            byte rc = decoder.ResultInfo.Rc;
            if (rc != 0) {
                AppCommon.OutputLogError(string.Format(ToolGUICommon.MSG_DFU_IMAGE_INSTALL_FAILED_WITH_RC, rc));
                return false;
            }
            return true;
        }

        private byte[] BuildSMPHeader(byte op, byte flags, ushort len, ushort group, byte seq, byte id_int)
        {
            byte[] header = {
                op,
                flags,
                (byte)(len >> 8),   (byte)(len & 0xff),
                (byte)(group >> 8), (byte)(group & 0xff),
                seq,
                id_int
            };
            return header;
        }

        //
        // データ送受信時の処理
        //
        private void SendSMPRequestData(byte[] requestBody, byte[] requestHeader)
        {
            // ヘッダーと本体を連結
            byte[] requestData = Enumerable.Concat(requestHeader, requestBody).ToArray();

            // リクエストデータを送信
            ToolBLESMPService.Send(requestData);

            // ログ出力
            string dump = AppCommon.DumpMessage(requestData, requestData.Length);
            AppCommon.OutputLogDebug(string.Format(
                "Transmit SMP request ({0} bytes)\r\n{1}",
                requestData.Length, dump));

            // 応答タイムアウト監視開始
            responseTimer.Start();
        }

        // 受信済みデータ／バイト数を保持
        private byte[] ResponseData;
        private int received = 0;
        private int totalSize = 0;

        private void OnDataReceived(byte[] receivedData)
        {
            // 応答タイムアウト監視終了
            responseTimer.Stop();

            // ログ出力
            string dump = AppCommon.DumpMessage(receivedData, receivedData.Length);
            AppCommon.OutputLogDebug(string.Format(
                "Incoming SMP response ({0} bytes)\r\n{1}",
                receivedData.Length, dump));

            // 受信したレスポンスデータを保持
            int responseSize = receivedData.Length;
            if (received == 0) {
                // レスポンスヘッダーからデータ長を抽出
                totalSize = GetSmpResponseBodySize(receivedData);
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
                switch (Command) {
                    case BLEDFUCommand.GetSlotInfo:
                        DoResponseGetSlotInfo(ResponseData);
                        break;
                    case BLEDFUCommand.UploadImage:
                        DoResponseUploadImage(ResponseData);
                        break;
                    case BLEDFUCommand.ChangeImageUpdateMode:
                        DoResponseChangeImageUpdateMode(ResponseData);
                        break;
                    default:
                        break;
                }
                received = 0;
                totalSize = 0;
            }
        }

        private int GetSmpResponseBodySize(byte[] responseData)
        {
            // レスポンスヘッダーの３・４バイト目からデータ長を抽出
            int totalSize = ((responseData[2] << 8) & 0xff00) + (responseData[3] & 0x00ff);
            return totalSize;
        }

        private void OnTransactionFailed()
        {
            // 応答タイムアウト監視終了
            responseTimer.Stop();

            // 処理区分に応じて分岐
            switch (Command) {
                case BLEDFUCommand.GetSlotInfo:
                    OnNotifyDFUErrorMessage(ToolGUICommon.MSG_DFU_SLOT_INFO_GET_FAILED);
                    break;
                case BLEDFUCommand.UploadImage:
                    OnNotifyDFUErrorMessage(ToolGUICommon.MSG_DFU_IMAGE_TRANSFER_FAILED);
                    break;
                case BLEDFUCommand.ChangeImageUpdateMode:
                    OnNotifyDFUErrorMessage(ToolGUICommon.MSG_DFU_CHANGE_IMAGE_UPDATE_MODE_FAILED);
                    break;
                default:
                    break;
            }

            // 画面に異常終了を通知
            TerminateDFUProcess(false);
        }

        //
        // 応答タイムアウト時の処理
        //
        private void OnResponseTimerElapsed(object sender, EventArgs e)
        {
            // 応答タイムアウトを通知
            OnNotifyDFUErrorMessage(ToolGUICommon.MSG_DFU_PROCESS_TIMEOUT);
            TerminateDFUProcess(false);
        }
    }
}
