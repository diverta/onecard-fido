using MaintenanceToolCommon;
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
            ToolBLEDFUImageRef = imageRef;
            ToolBLESMPService = new ToolBLESMPService(this);
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

        public void OnConnectionFailed()
        {
            // 画面に異常終了を通知
            TerminateDFUProcess(false);
        }

        public void OnConnected()
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

        private void DoResponseGetSlotInfo()
        {
            // TODO: 仮の実装です。
            System.Threading.Thread.Sleep(1000);
            TerminateDFUProcess(false);
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

        public void OnDataReceived(byte[] receivedData)
        {
            // ログ出力
            string dump = AppCommon.DumpMessage(receivedData, receivedData.Length);
            AppCommon.OutputLogDebug(string.Format(
                "Incoming SMP response ({0} bytes)\r\n{1}",
                receivedData.Length, dump));

            // 処理区分に応じて分岐
            switch (Command) {
                case BLEDFUCommand.GetSlotInfo:
                    DoResponseGetSlotInfo();
                    break;
                default:
                    break;
            }
        }

        public void OnTransactionFailed()
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
