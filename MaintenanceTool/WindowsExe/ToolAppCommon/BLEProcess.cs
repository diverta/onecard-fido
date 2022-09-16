using MaintenanceToolApp;
using System.Linq;
using System.Threading.Tasks;

namespace ToolAppCommon
{
    internal static class BLEProcessConst
    {
        // BLEフレームに関する定義
        public const int INIT_HEADER_LEN = 3;
        public const int CONT_HEADER_LEN = 1;
        public const int BLE_FRAME_LEN = 64;
    }

    public class BLEProcess
    {
        // このクラスのインスタンス
        private static readonly BLEProcess Instance = new BLEProcess();

        // BLEメッセージ受信時のイベント
        public delegate void HandlerOnReceivedResponse(byte CMD, byte[] responseData, bool success, string errorMessage);
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

        public static void DisconnctBLE()
        {
            // 接続破棄
            Instance.DisconnectBLE();
        }

        //
        // 内部処理
        //
        // BLEデバイス
        private BLEService BleService = new BLEService();

        // ペアリング完了時のイベント
        public delegate void HandlerOnPairingCompleted(bool success, string messageOnFail);
        public event HandlerOnPairingCompleted OnPairingCompleted = null!;

        private BLEProcess()
        {
            // BLEデバイスのイベントを登録
            BleService.OnDataReceived += new BLEService.HandlerOnDataReceived(OnDataReceived);
            BleService.OnTransactionFailed += new BLEService.HandlerOnTransactionFailed(OnTransactionFailed);
            BleService.OnFIDOPeripheralFound += new BLEService.HandlerOnFIDOPeripheralFound(OnFIDOPeripheralFound);
            BleService.OnFIDOPeripheralPaired += new BLEService.HandlerOnFIDOPeripheralPaired(OnFIDOPeripheralPaired);
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
            OnPairingCompleted(success, messageOnFail);
        }

        // 当初送信コマンドを保持
        private byte CMDToSend { get; set; }

        private async void SendBLEMessage(byte CMD, byte[] message)
        {
            // 送信コマンドを保持
            CMDToSend = CMD;

            // メッセージがない場合は終了
            if (message == null || message.Length == 0) {
                OnReceivedResponse(CMD, new byte[0], false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            if (BleService.IsConnected() == false) {
                for (int i = 0; i < 2; i++) {
                    // 未接続の場合はFIDO認証器とのBLE通信を開始
                    if (await BleService.StartCommunicate()) {
                        AppLogUtil.OutputLogInfo(AppCommon.MSG_U2F_DEVICE_CONNECTED);
                        break;
                    }
                    AppLogUtil.OutputLogWarn(string.Format("接続を再試行しています（{0}回目）", i + 1));
                    await Task.Run(() => System.Threading.Thread.Sleep(250));
                }
            }

            if (BleService.IsConnected() == false) {
                // 接続失敗の旨を通知（エラーログは上位クラスで出力させるようにする）
                OnReceivedResponse(CMD, new byte[0], false, AppCommon.MSG_U2F_DEVICE_CONNECT_FAILED);
                return;
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
            byte[] frameData = new byte[BLEProcessConst.BLE_FRAME_LEN];
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
                    int maxLen = BLEProcessConst.BLE_FRAME_LEN - BLEProcessConst.INIT_HEADER_LEN;
                    int dataLenInFrame = (transferMessageLen < maxLen) ? transferMessageLen : maxLen;
                    for (int i = 0; i < dataLenInFrame; i++) {
                        frameData[BLEProcessConst.INIT_HEADER_LEN + i] = message[transferred++];
                    }

                    // フレーム長を取得
                    frameLen = BLEProcessConst.INIT_HEADER_LEN + dataLenInFrame;

                    string dump = AppLogUtil.DumpMessage(frameData, frameLen);
                    AppLogUtil.OutputLogDebug(string.Format("BLE Sent INIT frame: data size={0} length={1}\r\n{2}",
                        transferMessageLen, dataLenInFrame, dump));

                } else {
                    // CONTフレーム
                    // ヘッダーをコピー
                    frameData[0] = (byte)seq;

                    // データをコピー
                    int remaining = transferMessageLen - transferred;
                    int maxLen = BLEProcessConst.BLE_FRAME_LEN - BLEProcessConst.CONT_HEADER_LEN;
                    int dataLenInFrame = (remaining < maxLen) ? remaining : maxLen;
                    for (int i = 0; i < dataLenInFrame; i++) {
                        frameData[BLEProcessConst.CONT_HEADER_LEN + i] = message[transferred++];
                    }

                    // フレーム長を取得
                    frameLen = BLEProcessConst.CONT_HEADER_LEN + dataLenInFrame;

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
            int bleInitDataLen = BLEProcessConst.BLE_FRAME_LEN - BLEProcessConst.INIT_HEADER_LEN;
            int bleContDataLen = BLEProcessConst.BLE_FRAME_LEN - BLEProcessConst.CONT_HEADER_LEN;
            byte cmd = message[0];
            if (cmd > 127) {
                // INITフレームであると判断
                byte cnth = message[1];
                byte cntl = message[2];
                ReceivedMessageLen = cnth * 256 + cntl;
                ReceivedMessage = new byte[BLEProcessConst.INIT_HEADER_LEN + ReceivedMessageLen];
                ReceivedSize = 0;

                // ヘッダーをコピー
                for (int i = 0; i < BLEProcessConst.INIT_HEADER_LEN; i++) {
                    ReceivedMessage[i] = message[i];
                }

                // データをコピー
                int dataLenInFrame = (ReceivedMessageLen < bleInitDataLen) ? ReceivedMessageLen : bleInitDataLen;
                for (int i = 0; i < dataLenInFrame; i++) {
                    ReceivedMessage[BLEProcessConst.INIT_HEADER_LEN + ReceivedSize++] = message[BLEProcessConst.INIT_HEADER_LEN + i];
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
                    ReceivedMessage[BLEProcessConst.INIT_HEADER_LEN + ReceivedSize++] = message[BLEProcessConst.CONT_HEADER_LEN + i];
                }

                string dump = AppLogUtil.DumpMessage(message, message.Length);
                AppLogUtil.OutputLogDebug(string.Format(
                    "BLE Recv CONT frame: seq={0} length={1}\r\n{2}",
                    seq, dataLenInFrame, dump));
            }

            // 全フレームがそろった場合
            if (ReceivedSize == ReceivedMessageLen) {
                // CMDを抽出
                byte CMD = ReceivedMessage[0];
                if (CMD == 0x82) {
                    // キープアライブの場合は引き続き次のレスポンスを待つ
                    return;
                }

                // 受信データを転送
                AppLogUtil.OutputLogInfo(AppCommon.MSG_RESPONSE_RECEIVED);
                if (ReceivedMessageLen == 0) {
                    OnReceivedResponse(CMD, new byte[0], true, "");

                } else {
                    byte[] response = ReceivedMessage.Skip(BLEProcessConst.INIT_HEADER_LEN).ToArray();
                    OnReceivedResponse(CMD, response, true, "");
                }
            }
        }

        private void OnTransactionFailed(string errorMessage)
        {
            // 送信失敗時
            OnReceivedResponse(CMDToSend, new byte[0], false, errorMessage);
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
