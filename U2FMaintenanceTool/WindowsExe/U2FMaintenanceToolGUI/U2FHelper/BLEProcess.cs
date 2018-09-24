using System;
using System.Threading.Tasks;

namespace U2FHelper
{
    public class BLEProcess
    {
        internal static class Const
        {
            public const int MSG_HEADER_LEN = 3;
            public const int INIT_HEADER_LEN = 3;
            public const int CONT_HEADER_LEN = 1;
            public const int BLE_FRAME_LEN = 64;
        }

        // BLE接続状態を保持
        private bool isConnected = false;

        // BLEデバイス関連
        private BLEService bleService = new BLEService();

        // メッセージテキスト送信用のイベント
        public delegate void MessageTextEventHandler(string messageText);
        public event MessageTextEventHandler MessageTextEvent;

        // BLEメッセージ受信時のイベント
        public delegate void ReceiveBLEMessageEventHandler(bool ret, byte[] receivedMessage, int receivedLen);
        public event ReceiveBLEMessageEventHandler ReceiveBLEMessageEvent;

        // ペアリング完了時のイベント
        public delegate void oneCardPeripheralPairedEvent(bool success);
        public event oneCardPeripheralPairedEvent OneCardPeripheralPaired;

        public BLEProcess()
        {
            // BLEデバイスのイベントを登録
            bleService.DataReceived += new BLEService.dataReceivedEvent(BLEDeviceDataReceived);
            bleService.OneCardPeripheralFound += new BLEService.oneCardPeripheralFoundEvent(OnOneCardPeripheralFound);
            bleService.OneCardPeripheralPaired += new BLEService.oneCardPeripheralPairedEvent(OnOneCardPeripheralPaired);
        }

        public void PairWithOneCardPeripheral()
        {
            bleService.Pair();
        }

        private void OnOneCardPeripheralFound()
        {
            bleService.PairWithOneCardPeripheral();
        }

        private void OnOneCardPeripheralPaired(bool success)
        {
            OneCardPeripheralPaired(success);
        }

        public async void DoXferMessage(byte[] message, int length)
        {
            // メッセージがない場合は終了
            if (message == null || length == 0) {
                return;
            }

            if (isConnected == false) {
                // 未接続の場合はBLEデバイスに接続
                if (await bleService.Connect() == false) {
                    MessageTextEvent(AppCommon.MSG_U2F_DEVICE_CONNECT_FAILED);
                    ReceiveBLEMessageEvent(false, null, 0);
                    return;
                }
                isConnected = true;
            }

            // BLEデバイスにメッセージをフレーム分割して送信
            if (await SendBLEMessageFrames(message, length) == false) {
                // 送信失敗時は切断
                bleService.Disconnect();
                MessageTextEvent(AppCommon.MSG_REQUEST_SEND_FAILED);
                ReceiveBLEMessageEvent(false, null, 0);
                return;
            }

            // リクエスト送信完了メッセージを出力
            AppCommon.OutputLogToFile(AppCommon.MSG_REQUEST_SENT, true);
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
                            message[Const.MSG_HEADER_LEN + transferred++];
                    }

                    // フレーム長を取得
                    frameLen = Const.INIT_HEADER_LEN + dataLenInFrame;

                    string dump = AppCommon.DumpMessage(frameData, frameLen);
                    AppCommon.OutputLogToFile(string.Format("INIT frame: data size={0} length={1}\r\n{2}",
                        transferMessageLen, frameLen, dump), true);

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
                            message[Const.MSG_HEADER_LEN + transferred++];
                    }

                    // フレーム長を取得
                    frameLen = Const.CONT_HEADER_LEN + dataLenInFrame;

                    string dump = AppCommon.DumpMessage(frameData, frameLen);
                    AppCommon.OutputLogToFile(string.Format("CONT frame: data seq={0} length={1}\r\n{2}",
                        seq++, frameLen, dump), true);
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

                AppCommon.OutputLogToFile(string.Format(
                    "INIT frame: data size={0} length={1}",
                    receivedMessageLen, dataLenInFrame), true);

            } else {
                // CONTフレームであると判断
                int seq = message[0];

                // データをコピー
                int remaining = receivedMessageLen - received;
                int dataLenInFrame = (remaining < bleContDataLen) ? remaining : bleContDataLen;
                for (int i = 0; i < dataLenInFrame; i++) {
                    receivedMessage[Const.INIT_HEADER_LEN + received++] = message[Const.CONT_HEADER_LEN + i];
                }

                AppCommon.OutputLogToFile(string.Format(
                    "CONT frame: seq={0} length={1}",
                    seq, dataLenInFrame), true);
            }

            // メッセージをダンプ
            AppCommon.OutputLogToFile(AppCommon.DumpMessage(message, message.Length), false);

            // 全フレームがそろった場合
            if (received == receivedMessageLen) {
                if (receivedMessage[0] == 0x82) {
                    // キープアライブの場合は引き続き次のレスポンスを待つ
                    return;
                }
                int messageLength = Const.INIT_HEADER_LEN + receivedMessageLen;

                if (receivedMessage[0] == 0x83) {
                    // コマンドレスポンスの場合はステータスワードをチェック
                    if (CheckStatusWord(receivedMessage, messageLength) == false) {
                        // ステータスワードが不正の場合は、BLEを切断し画面に制御を戻す
                        bleService.Disconnect();
                        ReceiveBLEMessageEvent(false, null, 0);
                        return;
                    }
                }

                // 受信データを転送
                AppCommon.OutputLogToFile(AppCommon.MSG_RESPONSE_RECEIVED, true);
                ReceiveBLEMessageEvent(true, receivedMessage, messageLength);
            }
        }

        private bool CheckStatusWord(byte[] receivedMessage, int receivedLen)
        {
            // ステータスワードをチェック
            byte[] statusBytes = new byte[2];
            Array.Copy(receivedMessage, receivedLen - 2, statusBytes, 0, 2);
            if (BitConverter.IsLittleEndian) {
                Array.Reverse(statusBytes);
            }
            ushort statusWord = BitConverter.ToUInt16(statusBytes, 0);

            if (statusWord == 0x6985) {
                // キーハンドルチェックの場合は成功とみなす
                return true;
            }
            if (statusWord == 0x6a80) {
                // invalid keyhandleエラーである場合はその旨を通知
                MessageTextEvent("キーハンドルが存在しません。再度U2F Register(Enroll)を実行してください。");
                return false;
            }
            if (statusWord == 0x9402) {
                // 鍵・証明書がインストールされていない旨のエラーである場合はその旨を通知
                MessageTextEvent("鍵・証明書がインストールされていません。鍵・証明書インストール処理を実行してください。");
                return false;
            }
            if (statusWord == 0x9601) {
                // ペアリングモード時はペアリング以外の機能を実行できない旨を通知
                MessageTextEvent("ペアリングモードでは、ペアリング実行以外の機能は使用できません。\r\nペアリングモードを解除してから、機能を再度実行してください。");
                return false;
            }
            if (statusWord != 0x9000) {
                // U2Fサービスの戻りコマンドが不正の場合はエラー
                MessageTextEvent("不明なエラーが発生しました。");
                return false;
            }

            return true;
        }

        public void DisconnectBLE()
        {
            if (isConnected) {
                // 接続ずみの場合はBLEデバイスを切断
                bleService.Disconnect();
                isConnected = false;
            }
        }
    }
}
