using MaintenanceToolApp;
using System.Linq;

namespace ToolAppCommon
{
    public class BLEProcess
    {
        // このクラスのインスタンス
        private static readonly BLEProcess Instance = new BLEProcess();

        // BLEメッセージ受信時のイベント
        public delegate void HandlerOnReceivedResponse(byte[] responseData, bool success, string errorMessage);
        private event HandlerOnReceivedResponse OnReceivedResponse = null!;

        //
        // 外部公開用
        //
        public static void RegisterHandlerOnReceivedResponse(HandlerOnReceivedResponse handler)
        {
            Instance.OnReceivedResponse += handler;
        }

        public static void DoRequestCommand(byte CMD, byte[] data)
        {
            Instance.SendBLEMessage(CMD, data);
        }

        //
        // 内部処理
        //
        internal static class Const
        {
            public const int INIT_HEADER_LEN = 3;
            public const int CONT_HEADER_LEN = 1;
            public const int BLE_FRAME_LEN = 64;
        }

        // BLEデバイス関連
        private BLEService BleService = new BLEService();

        // ペアリング完了時のイベント
        public delegate void FIDOPeripheralPairedEvent(bool success, string messageOnFail);
        public event FIDOPeripheralPairedEvent FIDOPeripheralPaired = null!;

        private BLEProcess()
        {
            // BLEデバイスのイベントを登録
            BleService.OnDataReceived += new BLEService.DataReceivedEvent(OnDataReceived);
            BleService.OnTransactionFailed += new BLEService.TransactionFailedEvent(OnTransactionFailed);
            BleService.FIDOPeripheralFound += new BLEService.FIDOPeripheralFoundEvent(OnFIDOPeripheralFound);
            BleService.FIDOPeripheralPaired += new BLEService.FIDOPeripheralPairedEvent(OnFIDOPeripheralPaired);
        }

        private void PairWithFIDOPeripheral(string passkey)
        {
            BleService.Pair(passkey);
        }

        private void OnFIDOPeripheralFound()
        {
            BleService.PairWithFIDOPeripheral();
        }

        private void OnFIDOPeripheralPaired(bool success, string messageOnFail)
        {
            FIDOPeripheralPaired(success, messageOnFail);
        }

        private async void SendBLEMessage(byte CMD, byte[] message)
        {
            // メッセージがない場合は終了
            if (message == null || message.Length == 0) {
                OnReceivedResponse(new byte[0], false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            if (BleService.IsConnected() == false) {
                // 未接続の場合はFIDO認証器とのBLE通信を開始
                if (await BleService.StartCommunicate() == false) {
                    AppLogUtil.OutputLogError(AppCommon.MSG_U2F_DEVICE_CONNECT_FAILED);
                    OnReceivedResponse(new byte[0], false, AppCommon.MSG_U2F_DEVICE_CONNECT_FAILED);
                    return;
                }
                AppLogUtil.OutputLogInfo(AppCommon.MSG_U2F_DEVICE_CONNECTED);
            }

            // BLEデバイスにメッセージをフレーム分割して送信
            SendBLEMessageFrames(CMD, message);

            // リクエスト送信完了メッセージを出力
            AppLogUtil.OutputLogInfo(AppCommon.MSG_REQUEST_SENT);
        }

        private void SendBLEMessageFrames(byte CMD, byte[] message)
        {
            // APDUの長さを取得
            int transferMessageLen = message.Length;

            // 
            // 送信データをフレーム分割
            // 
            //  INITフレーム
            //  1     バイト目: コマンド
            //  2 - 3 バイト目: データ長
            //  残りのバイト  : データ部（64 - 3 = 61）
            //
            //  CONTフレーム
            //  1     バイト目: シーケンス
            //  残りのバイト  : データ部（64 - 1 = 63）
            // 
            byte[] frameData = new byte[Const.BLE_FRAME_LEN];
            int frameLen = 0;
            int transferred = 0;
            int seq = 0;
            while (transferred < transferMessageLen) {
                for (int j = 0; j < frameData.Length; j++) {
                    // フレームデータを初期化
                    frameData[j] = 0;
                }

                if (transferred == 0) {
                    // INITフレーム
                    // ヘッダーをコピー
                    frameData[0] = CMD;
                    frameData[1] = (byte)(transferMessageLen / 256);
                    frameData[2] = (byte)(transferMessageLen % 256);

                    // データをコピー
                    int maxLen = Const.BLE_FRAME_LEN - Const.INIT_HEADER_LEN;
                    int dataLenInFrame = (transferMessageLen < maxLen) ? transferMessageLen : maxLen;
                    for (int i = 0; i < dataLenInFrame; i++) {
                        frameData[Const.INIT_HEADER_LEN + i] = message[transferred++];
                    }

                    // フレーム長を取得
                    frameLen = Const.INIT_HEADER_LEN + dataLenInFrame;

                    string dump = AppLogUtil.DumpMessage(frameData, frameLen);
                    AppLogUtil.OutputLogDebug(string.Format("BLE Sent INIT frame: data size={0} length={1}\r\n{2}",
                        transferMessageLen, dataLenInFrame, dump));

                } else {
                    // CONTフレーム
                    // ヘッダーをコピー
                    frameData[0] = (byte)seq;

                    // データをコピー
                    int remaining = transferMessageLen - transferred;
                    int maxLen = Const.BLE_FRAME_LEN - Const.CONT_HEADER_LEN;
                    int dataLenInFrame = (remaining < maxLen) ? remaining : maxLen;
                    for (int i = 0; i < dataLenInFrame; i++) {
                        frameData[Const.CONT_HEADER_LEN + i] = message[transferred++];
                    }

                    // フレーム長を取得
                    frameLen = Const.CONT_HEADER_LEN + dataLenInFrame;

                    string dump = AppLogUtil.DumpMessage(frameData, frameLen);
                    AppLogUtil.OutputLogDebug(string.Format("BLE Sent CONT frame: data seq={0} length={1}\r\n{2}",
                        seq++, dataLenInFrame, dump));
                }

                // BLEデバイスにフレームを送信
                BleService.Send(frameData.Take(frameLen).ToArray());
            }
        }

        // 受信データを保持
        private byte[] ReceivedMessage = new byte[0];
        private int ReceivedMessageLen = 0;
        private int ReceivedSize = 0;

        private void OnDataReceived(byte[] message)
        {
            // メッセージがない場合は終了
            if (message == null || message.Length == 0) {
                return;
            }

            // 
            // 受信したデータをバッファにコピー
            // 
            //  INITフレーム
            //  1     バイト目: コマンド
            //  2 - 3 バイト目: データ長
            //  残りのバイト  : データ部（64 - 3 = 61）
            //
            //  CONTフレーム
            //  1     バイト目: シーケンス
            //  残りのバイト  : データ部（64 - 1 = 63）
            // 
            int bleInitDataLen = Const.BLE_FRAME_LEN - Const.INIT_HEADER_LEN;
            int bleContDataLen = Const.BLE_FRAME_LEN - Const.CONT_HEADER_LEN;
            byte cmd = message[0];
            if (cmd > 127) {
                // INITフレームであると判断
                byte cnth = message[1];
                byte cntl = message[2];
                ReceivedMessageLen = cnth * 256 + cntl;
                ReceivedMessage = new byte[Const.INIT_HEADER_LEN + ReceivedMessageLen];
                ReceivedSize = 0;

                // ヘッダーをコピー
                for (int i = 0; i < Const.INIT_HEADER_LEN; i++) {
                    ReceivedMessage[i] = message[i];
                }

                // データをコピー
                int dataLenInFrame = (ReceivedMessageLen < bleInitDataLen) ? ReceivedMessageLen : bleInitDataLen;
                for (int i = 0; i < dataLenInFrame; i++) {
                    ReceivedMessage[Const.INIT_HEADER_LEN + ReceivedSize++] = message[Const.INIT_HEADER_LEN + i];
                }

                if (ReceivedMessage[0] != 0x82) {
                    // キープアライブ以外の場合はログを出力
                    string dump = AppLogUtil.DumpMessage(message, message.Length);
                    AppLogUtil.OutputLogDebug(string.Format(
                        "BLE Recv INIT frame: data size={0} length={1}\r\n{2}",
                        ReceivedMessageLen, dataLenInFrame, dump));
                }

            } else {
                // CONTフレームであると判断
                int seq = message[0];

                // データをコピー
                int remaining = ReceivedMessageLen - ReceivedSize;
                int dataLenInFrame = (remaining < bleContDataLen) ? remaining : bleContDataLen;
                for (int i = 0; i < dataLenInFrame; i++) {
                    ReceivedMessage[Const.INIT_HEADER_LEN + ReceivedSize++] = message[Const.CONT_HEADER_LEN + i];
                }

                string dump = AppLogUtil.DumpMessage(message, message.Length);
                AppLogUtil.OutputLogDebug(string.Format(
                    "BLE Recv CONT frame: seq={0} length={1}\r\n{2}",
                    seq, dataLenInFrame, dump));
            }

            // 全フレームがそろった場合
            if (ReceivedSize == ReceivedMessageLen) {
                if (ReceivedMessage[0] == 0x82) {
                    // キープアライブの場合は引き続き次のレスポンスを待つ
                    return;
                }
                int messageLength = Const.INIT_HEADER_LEN + ReceivedMessageLen;

                // 受信データを転送
                AppLogUtil.OutputLogInfo(AppCommon.MSG_RESPONSE_RECEIVED);
                OnReceivedResponse(ReceivedMessage, true, "");
            }
        }

        private void OnTransactionFailed()
        {
            // 送信失敗時
            OnReceivedResponse(new byte[0], false, AppCommon.MSG_REQUEST_SEND_FAILED);
        }

        private void DisconnectBLE()
        {
            if (BleService.IsConnected()) {
                // 接続ずみの場合はBLEデバイスを切断
                BleService.Disconnect();
            }
        }
    }
}
