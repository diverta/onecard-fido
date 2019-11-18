using System;
using System.Threading.Tasks;

namespace MaintenanceToolCommon
{
    public class BLEProcess
    {
        public const int MSG_HEADER_LEN = 3;

        internal static class Const
        {
            public const int INIT_HEADER_LEN = 3;
            public const int CONT_HEADER_LEN = 1;
            public const int BLE_FRAME_LEN = 64;
        }

        // BLEデバイス関連
        private BLEService bleService = new BLEService();

        // メッセージテキスト送信用のイベント
        public delegate void MessageTextEventHandler(string messageText);
        public event MessageTextEventHandler MessageTextEvent;

        // BLEメッセージ受信時のイベント
        public delegate void ReceiveBLEMessageEventHandler(byte[] receivedMessage, int receivedLen);
        public event ReceiveBLEMessageEventHandler ReceiveBLEMessageEvent;

        // BLEメッセージ受信失敗時のイベント
        public delegate void ReceiveBLEFailedEventHandler(bool critical, byte reserved);
        public event ReceiveBLEFailedEventHandler ReceiveBLEFailedEvent;

        // ペアリング完了時のイベント
        public delegate void FIDOPeripheralPairedEvent(bool success, string messageOnFail);
        public event FIDOPeripheralPairedEvent FIDOPeripheralPaired;

        public BLEProcess()
        {
            // BLEデバイスのイベントを登録
            bleService.DataReceived += new BLEService.dataReceivedEvent(BLEDeviceDataReceived);
            bleService.FIDOPeripheralFound += new BLEService.FIDOPeripheralFoundEvent(OnFIDOPeripheralFound);
            bleService.FIDOPeripheralPaired += new BLEService.FIDOPeripheralPairedEvent(OnFIDOPeripheralPaired);
        }

        public void PairWithFIDOPeripheral()
        {
            bleService.Pair();
        }

        private void OnFIDOPeripheralFound()
        {
            bleService.PairWithFIDOPeripheral();
        }

        private void OnFIDOPeripheralPaired(bool success, string messageOnFail)
        {
            FIDOPeripheralPaired(success, messageOnFail);
        }

        public async void DoXferMessage(byte[] message, int length)
        {
            // メッセージがない場合は終了
            if (message == null || length == 0) {
                ReceiveBLEFailedEvent(false, 0);
                return;
            }

            if (bleService.IsConnected() == false) {
                // 未接続の場合はFIDO認証器とのBLE通信を開始
                if (await bleService.StartCommunicate() == false) {
                    AppCommon.OutputLogError(AppCommon.MSG_U2F_DEVICE_CONNECT_FAILED);
                    MessageTextEvent(AppCommon.MSG_U2F_DEVICE_CONNECT_FAILED);
                    ReceiveBLEFailedEvent(bleService.IsCritical(), 0);
                    return;
                }
                AppCommon.OutputLogInfo(AppCommon.MSG_U2F_DEVICE_CONNECTED);
            }

            // BLEデバイスにメッセージをフレーム分割して送信
            if (await SendBLEMessageFrames(message, length) == false) {
                // 送信失敗時
                MessageTextEvent(AppCommon.MSG_REQUEST_SEND_FAILED);
                ReceiveBLEFailedEvent(bleService.IsCritical(), 0);
                return;
            }

            // リクエスト送信完了メッセージを出力
            AppCommon.OutputLogInfo(AppCommon.MSG_REQUEST_SENT);
        }

        private async Task<bool> SendBLEMessageFrames(byte[] message, int length)
        {
            // 正しいAPDUの長さをメッセージ・ヘッダーから取得
            int transferMessageLen = message[1] * 256 + message[2];

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
                    frameData[0] = message[0];
                    frameData[1] = message[1];
                    frameData[2] = message[2];

                    // データをコピー
                    int maxLen = Const.BLE_FRAME_LEN - Const.INIT_HEADER_LEN;
                    int dataLenInFrame = (transferMessageLen < maxLen) ? transferMessageLen : maxLen;
                    for (int i = 0; i < dataLenInFrame; i++) {
                        frameData[Const.INIT_HEADER_LEN + i] =
                            message[MSG_HEADER_LEN + transferred++];
                    }

                    // フレーム長を取得
                    frameLen = Const.INIT_HEADER_LEN + dataLenInFrame;

                    string dump = AppCommon.DumpMessage(frameData, frameLen);
                    AppCommon.OutputLogDebug(string.Format("BLE Sent INIT frame: data size={0} length={1}\r\n{2}",
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
                        frameData[Const.CONT_HEADER_LEN + i] =
                            message[MSG_HEADER_LEN + transferred++];
                    }

                    // フレーム長を取得
                    frameLen = Const.CONT_HEADER_LEN + dataLenInFrame;

                    string dump = AppCommon.DumpMessage(frameData, frameLen);
                    AppCommon.OutputLogDebug(string.Format("BLE Sent CONT frame: data seq={0} length={1}\r\n{2}",
                        seq++, dataLenInFrame, dump));
                }

                // BLEデバイスにフレームを送信
                if (await bleService.Send(frameData, frameLen) == false) {
                    return false;
                }
            }

            return true;
        }

        // 受信データを保持
        private byte[] receivedMessage = new byte[1024];
        private int receivedMessageLen = 0;
        private int received = 0;

        private void BLEDeviceDataReceived(byte[] message, int length)
        {
            // メッセージがない場合は終了
            if (message == null || length == 0) {
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
                receivedMessageLen = cnth * 256 + cntl;
                received = 0;

                // ヘッダーをコピー
                for (int i = 0; i < Const.INIT_HEADER_LEN; i++) {
                    receivedMessage[i] = message[i];
                }

                // データをコピー
                int dataLenInFrame = (receivedMessageLen < bleInitDataLen) ? receivedMessageLen : bleInitDataLen;
                for (int i = 0; i < dataLenInFrame; i++) {
                    receivedMessage[Const.INIT_HEADER_LEN + received++] = message[Const.INIT_HEADER_LEN + i];
                }

                if (receivedMessage[0] != 0x82) {
                    // キープアライブ以外の場合はログを出力
                    string dump = AppCommon.DumpMessage(message, message.Length);
                    AppCommon.OutputLogDebug(string.Format(
                        "BLE Recv INIT frame: data size={0} length={1}\r\n{2}",
                        receivedMessageLen, dataLenInFrame, dump));
                }

            } else {
                // CONTフレームであると判断
                int seq = message[0];

                // データをコピー
                int remaining = receivedMessageLen - received;
                int dataLenInFrame = (remaining < bleContDataLen) ? remaining : bleContDataLen;
                for (int i = 0; i < dataLenInFrame; i++) {
                    receivedMessage[Const.INIT_HEADER_LEN + received++] = message[Const.CONT_HEADER_LEN + i];
                }

                string dump = AppCommon.DumpMessage(message, message.Length);
                AppCommon.OutputLogDebug(string.Format(
                    "BLE Recv CONT frame: seq={0} length={1}\r\n{2}",
                    seq, dataLenInFrame, dump));
            }

            // 全フレームがそろった場合
            if (received == receivedMessageLen) {
                if (receivedMessage[0] == 0x82) {
                    // キープアライブの場合は引き続き次のレスポンスを待つ
                    return;
                }
                int messageLength = Const.INIT_HEADER_LEN + receivedMessageLen;

                // 受信データを転送
                AppCommon.OutputLogInfo(AppCommon.MSG_RESPONSE_RECEIVED);
                ReceiveBLEMessageEvent(receivedMessage, messageLength);
            }
        }

        public void DisconnectBLE()
        {
            if (bleService.IsConnected()) {
                // 接続ずみの場合はBLEデバイスを切断
                bleService.Disconnect();
            }
        }
    }
}
