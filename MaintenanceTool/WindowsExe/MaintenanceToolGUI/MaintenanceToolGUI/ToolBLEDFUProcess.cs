using MaintenanceToolCommon;
using System;
using System.Linq;

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

        // 処理区分
        private enum BLEDFUCommand
        {
            GetSlotInfo = 0,
            UploadImage,
            ChangeImageUpdateMode,
            ResetApplication,
        };
        private BLEDFUCommand Command;

        // クラスの参照を保持
        private ToolBLEDFU ToolBLEDFURef;
        private ToolBLEDFUImage ToolBLEDFUImageRef;
        private ToolBLESMPService ToolBLESMPService;

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
        }

        public void PerformDFU(ToolBLEDFU dfuRef)
        {
            ToolBLEDFURef = dfuRef;

            // BLE SMPサービスに接続
            DoConnect();
        }

        private void TerminateDFUProcess(bool success)
        {
            // メイン画面にDFU処理の終了を通知
            ToolBLEDFURef.DFUProcessTerminated(success);
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
            ToolBLEDFURef.NotifyDFUProcess(ToolGUICommon.MSG_DFU_PROCESS_TRANSFER_IMAGE, 0);

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
            }

            // TODO: 仮の実装です。
            System.Threading.Thread.Sleep(1000);
            TerminateDFUProcess(false);
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

            // 既に転送対象イメージが導入されている場合は、画面／ログにその旨を出力し、処理を中止
            bool active = decoder.SlotInfos[0].Active;
            if (active && hashSlot.Equals(hashUpdate)) {
                AppCommon.OutputLogError(ToolGUICommon.MSG_DFU_IMAGE_ALREADY_INSTALLED);
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
        }

        // 受信済みデータ／バイト数を保持
        private byte[] ResponseData;
        private int received = 0;
        private int totalSize = 0;

        private void OnDataReceived(byte[] receivedData)
        {
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
                ResponseData.Concat(receivedData);
            }

            // 全フレームを受信したら、レスポンス処理を実行
            if (received == totalSize) {
                // 処理区分に応じて分岐
                switch (Command) {
                    case BLEDFUCommand.GetSlotInfo:
                        DoResponseGetSlotInfo(ResponseData);
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
            // 処理区分に応じて分岐
            switch (Command) {
                case BLEDFUCommand.GetSlotInfo:
                    NotifyErrorMessage(ToolGUICommon.MSG_DFU_SLOT_INFO_GET_FAILED);
                    break;
                default:
                    break;
            }

            // 画面に異常終了を通知
            TerminateDFUProcess(false);
        }

        private void NotifyErrorMessage(string message)
        {
            // エラーメッセージ文言を画面とログに出力
            ToolBLEDFURef.NotifyMessage(message);
            AppCommon.OutputLogError(message);
        }
    }
}
