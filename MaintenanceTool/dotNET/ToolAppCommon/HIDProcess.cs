using MaintenanceToolApp;
using MaintenanceToolApp.Common;
using System;
using System.Linq;
using System.Threading.Tasks;

namespace ToolAppCommon
{
    public static class HIDProcessConst
    {
        // HIDフレームに関する定義
        public const int HID_FRAME_LEN = 64;
        public const int HID_INIT_HEADER_LEN = 7;
        public const int HID_CONT_HEADER_LEN = 5;

        // HIDコマンドバイトに関する定義
        public const int HID_CMD_CTAPHID_PING = 0x81;
        public const int HID_CMD_CTAPHID_INIT = 0x86;
        public const int HID_CMD_CTAPHID_CBOR = 0x90;
        public const int HID_CMD_UNKNOWN_ERROR = 0xbf;
    }

    public class HIDProcess
    {
        // このクラスのインスタンス
        private static readonly HIDProcess Instance = new HIDProcess();

        // 応答タイムアウト監視用タイマー
        private readonly CommonTimer ResponseTimer = null!;

        public HIDProcess()
        {
            // 応答タイムアウト発生時のイベントを登録
            ResponseTimer = new CommonTimer("HIDProcess", 10000);
            ResponseTimer.CommandTimeoutEvent += OnResponseTimerElapsed;
        }

        // HID接続完了時のイベント
        public delegate void HandlerOnConnectHIDDevice(bool connected);
        public event HandlerOnConnectHIDDevice OnConnectHIDDevice = null!;

        // HID接続完了時のイベント
        public delegate void HandlerOnReceivedResponse(byte[] cid, byte CMD, byte[] data, bool success, string errorMessage);
        public event HandlerOnReceivedResponse OnReceivedResponse = null!;

        //
        // 外部公開用
        //
        public static void RegisterHandlerOnConnectHIDDevice(HandlerOnConnectHIDDevice handler)
        {
            Instance.OnConnectHIDDevice += handler;
        }

        public static void UnregisterHandlerOnConnectHIDDevice(HandlerOnConnectHIDDevice handler)
        {
            Instance.OnConnectHIDDevice -= handler;
        }

        public static void ConnectHIDDevice()
        {
            //
            // アプリケーション開始時点、またはUSBデバイス接続検知時で実行
            //
            // USB HIDデバイスに自動接続
            Instance.StartAsyncOperation();
        }

        public static void DisconnectHIDDevice()
        {
            //
            // アプリケーション終了時点、またはUSBデバイス切断検知時で実行
            //
            // デバイスが既に切断されている場合は終了
            if (Instance.device == null) {
                return;
            }

            // HIDデバイス管理クラスを解放
            Instance.CloseDevice();
        }

        public static bool IsUSBDeviceDisconnected()
        {
            return (Instance.device == null);
        }

        public static void RegisterHandlerOnReceivedResponse(HandlerOnReceivedResponse handler)
        {
            Instance.OnReceivedResponse += handler;
        }

        public static void DoRequestCommand(byte[] cid, byte CMD, byte[] data)
        {
            Instance.SendHIDMessage(cid, CMD, data);
        }

        //
        // 接続／切断関連
        //
        // HIDデバイス管理クラス
        private HIDDevice device = null!;

        private async void StartAsyncOperation()
        {
            // 0.25秒待機後に、もう一度USB HIDデバイスの有無を確認
            await Task.Run(() => System.Threading.Thread.Sleep(250));
            string devicePath = GetHIDDevicePath();
            if (devicePath.Equals("")) {
                return;
            }

            // デバイスが既に初期化されている場合は終了
            if (device != null) {
                return;
            }

            // デバイスを初期化
            device = new HIDDevice(devicePath);
            device.DataReceived += new HIDDevice.dataReceivedEvent(ReceiveHIDMessage);
            AppLogUtil.OutputLogInfo(string.Format(AppCommon.MSG_HID_CONNECTED + "{0}", devicePath));

            // ハンドラー経由で接続通知
            OnConnectHIDDevice(true);
        }

        private static string GetHIDDevicePath()
        {
            // 接続済みのHIDデバイスから、
            // VID=0xf055であるデバイスの
            // インターフェースを探す
            HIDDevice.InterfaceDetails[] devices = HIDDevice.GetConnectedDevices();
            for (int i = 0; i < devices.Length; i++) {
                ushort VID = devices[i].VID;
                string devicePath = devices[i].devicePath;
                if (VID == 0xf055) {
                    return devicePath;
                }
            }
            return "";
        }

        private void CloseDevice()
        {
            // HIDデバイス管理クラスを解放
            if (device == null) {
                return;
            }
            device.Close();
            device = null!;
            AppLogUtil.OutputLogInfo(AppCommon.MSG_HID_REMOVED);

            // ハンドラー経由で接続切断通知
            OnConnectHIDDevice(false);
        }

        //
        // データ送信
        //
        // リクエスト送信時のCIDを保持
        private readonly byte[] SendHIDMessageCID = new byte[4];

        // フレームデータを保持
        private readonly byte[] SendFrameData = new byte[HIDProcessConst.HID_FRAME_LEN];

        private void SendHIDMessage(byte[] cid, byte CMD, byte[] message)
        {
            // メッセージがない場合は終了
            if (message == null) {
                AppLogUtil.OutputLogError("SendHIDMessage: invalid message buffer");
                return;
            }
            // リクエスト送信時のCIDを保持
            cid.CopyTo(SendHIDMessageCID, 0);
            // 
            // 送信データをフレーム分割
            // 
            //  INITフレーム
            //  1 - 4 バイト目: CID
            //  5     バイト目: コマンド
            //  6 - 7 バイト目: データ長
            //  残りのバイト  : データ部（64 - 7 = 57）
            //
            //  CONTフレーム
            //  1 - 4 バイト目: CID
            //  5     バイト目: シーケンス
            //  残りのバイト  : データ部（64 - 5 = 59）
            // 
            int transferred = 0;
            int seq = 0;
            int messageSize = message.Length;
            if (messageSize == 0) {
                // 送信メッセージサイズが０の場合は
                // CIDとCMDだけを送信する。
                SendHIDHeaderMessage(cid, CMD, message);
            }
            while (transferred < messageSize) {
                for (int j = 0; j < SendFrameData.Length; j++) {
                    // フレームデータを初期化
                    SendFrameData[j] = 0;
                }
                for (int c = 0; c < cid.Length; c++) {
                    // CIDをコピー
                    SendFrameData[c] = cid[c];
                }

                if (transferred == 0) {
                    // INITフレーム
                    // ヘッダーを設定
                    SendFrameData[4] = CMD;
                    SendFrameData[5] = (byte)(messageSize / 256);
                    SendFrameData[6] = (byte)(messageSize % 256);

                    // データをコピー
                    int maxLen = HIDProcessConst.HID_FRAME_LEN - HIDProcessConst.HID_INIT_HEADER_LEN;
                    int dataLenInFrame = (messageSize < maxLen) ? messageSize : maxLen;
                    for (int i = 0; i < dataLenInFrame; i++) {
                        SendFrameData[HIDProcessConst.HID_INIT_HEADER_LEN + i] = message[transferred++];
                    }

                    string dump = AppLogUtil.DumpMessage(SendFrameData, HIDProcessConst.HID_INIT_HEADER_LEN + dataLenInFrame);
                    AppLogUtil.OutputLogDebug(string.Format(
                        "HID Sent INIT frame: data size={0} length={1}\r\n{2}",
                        messageSize, dataLenInFrame, dump));

                } else {
                    // CONTフレーム
                    // ヘッダーをコピー
                    SendFrameData[4] = (byte)seq;

                    // データをコピー
                    int remaining = messageSize - transferred;
                    int maxLen = HIDProcessConst.HID_FRAME_LEN - HIDProcessConst.HID_CONT_HEADER_LEN;
                    int dataLenInFrame = (remaining < maxLen) ? remaining : maxLen;
                    for (int i = 0; i < dataLenInFrame; i++) {
                        SendFrameData[HIDProcessConst.HID_CONT_HEADER_LEN + i] = message[transferred++];
                    }

                    string dump = AppLogUtil.DumpMessage(SendFrameData, HIDProcessConst.HID_CONT_HEADER_LEN + dataLenInFrame);
                    AppLogUtil.OutputLogDebug(string.Format(
                        "HID Sent CONT frame: data seq={0} length={1}\r\n{2}",
                        seq++, dataLenInFrame, dump));
                }

                // フレームデータを転送
                device.Write(SendFrameData);
            }

            // 転送が完了したら、応答タイムアウト監視開始
            ResponseTimer.Start();
        }

        private void SendHIDHeaderMessage(byte[] cid, byte CMD, byte[] message)
        {
            // 送信メッセージサイズが０の場合は
            // CIDとCMDだけを送信する。
            for (int j = 0; j < SendFrameData.Length; j++) {
                // フレームデータを初期化
                SendFrameData[j] = 0;
            }

            for (int c = 0; c < cid.Length; c++) {
                // CIDをコピー
                SendFrameData[c] = cid[c];
            }

            // CMDをコピー
            SendFrameData[4] = CMD;

            string dump = AppLogUtil.DumpMessage(SendFrameData, HIDProcessConst.HID_INIT_HEADER_LEN);
            AppLogUtil.OutputLogDebug(string.Format(
                "HID Sent INIT frame: data size={0} length={1}\r\n{2}",
                message.Length, 0, dump));

            // フレームデータを転送
            device.Write(SendFrameData);
        }

        //
        // データ受信
        //
        // 受信データを保持
        private byte[] ReceivedMessage = new byte[0];
        private int ReceivedMessageLen = 0;
        private int Received = 0;
        private byte ReceivedCMD = 0;

        // 送信ログを保持
        private string ReceivedLogBuffer = "";

        private void ReceiveHIDMessage(byte[] message)
        {
            // 応答タイムアウト監視終了
            ResponseTimer.Stop();

            // フレームデータを保持
            byte[] frameData = new byte[HIDProcessConst.HID_FRAME_LEN];

            // 受信したCID、コマンドを保持
            byte[] receivedCID = new byte[4];

            // メッセージは最低 8 バイト
            if (message.Length < 8) {
                AppLogUtil.OutputLogError(String.Format(
                    "Device_dataReceived: invalid received message size({0})", message.Length));
                return;
            }
            for (int f = 0; f < frameData.Length; f++) {
                // フレームデータを退避
                frameData[f] = message[f + 1];
            }
            // 
            // 受信したデータをバッファにコピー
            // 
            //  INITフレーム
            //  1     バイト目: レポートID
            //  2 - 5 バイト目: CID
            //  6     バイト目: コマンド
            //  7 - 8 バイト目: データ長
            //  残りのバイト  : データ部（65 - 8 = 57）
            //
            //  CONTフレーム
            //  1     バイト目: レポートID
            //  2 - 5 バイト目: CID
            //  6     バイト目: シーケンス
            //  残りのバイト  : データ部（65 - 6 = 59）
            // 
            for (int c = 0; c < receivedCID.Length; c++) {
                // CIDをコピー
                receivedCID[c] = frameData[c];
            }
            // レスポンス受信時のCIDが、リクエスト送信時のCIDと異なる場合は無視
            if (Enumerable.SequenceEqual(receivedCID, SendHIDMessageCID) == false) {
                return;
            }
            int hid_init_data_len = 57;
            int hid_cont_data_len = 59;
            if (frameData[4] > 127) {
                // INITフレームであると判断
                ReceivedCMD = frameData[4];
                byte cnth = frameData[5];
                byte cntl = frameData[6];
                ReceivedMessageLen = cnth * 256 + cntl;
                ReceivedMessage = new byte[ReceivedMessageLen];
                Received = 0;

                // データをコピー
                int dataLenInFrame = (ReceivedMessageLen < hid_init_data_len) ? ReceivedMessageLen : hid_init_data_len;
                for (int i = 0; i < dataLenInFrame; i++) {
                    ReceivedMessage[Received++] = frameData[HIDProcessConst.HID_INIT_HEADER_LEN + i];
                }

                ReceivedLogBuffer = "";
                string dump = AppLogUtil.DumpMessage(frameData, dataLenInFrame + HIDProcessConst.HID_INIT_HEADER_LEN);
                AppendLogToBuffer(string.Format(
                    "HID Recv INIT frame: data size={0} length={1}\r\n{2}",
                    ReceivedMessageLen, dataLenInFrame, dump));

            } else {
                // CONTフレームであると判断
                int seq = frameData[4];

                // INITフレームを未受信時はエラー
                if (Received == 0) {
                    string temp = AppLogUtil.DumpMessage(frameData, frameData.Length);
                    AppLogUtil.OutputLogDebug(string.Format(
                        "HID Recv irreagal frame: CMD=0x{0:x2} length={1}\r\n{2}", seq, frameData.Length, temp));
                    OnReceivedResponse(receivedCID, (byte)seq, ReceivedMessage, false, AppCommon.MSG_HID_RECV_IRREAGAL_FRAME);
                    return;
                }

                // データをコピー
                int remaining = ReceivedMessageLen - Received;
                int dataLenInFrame = (remaining < hid_cont_data_len) ? remaining : hid_cont_data_len;
                for (int i = 0; i < dataLenInFrame; i++) {
                    ReceivedMessage[Received++] = frameData[HIDProcessConst.HID_CONT_HEADER_LEN + i];
                }

                string dump = AppLogUtil.DumpMessage(frameData, dataLenInFrame + HIDProcessConst.HID_CONT_HEADER_LEN);
                AppendLogToBuffer(string.Format(
                    "HID Recv CONT frame: seq={0} length={1}\r\n{2}",
                    seq, dataLenInFrame, dump));
            }

            // キープアライブレスポンスの場合は無視
            if (ReceivedCMD == 0xbb) {
                return;
            }

            if (Received == ReceivedMessageLen) {
                // 全フレームを受信できたら、
                // この時点で一括してログ出力を行い、その後
                // HIDデバイスからのデータを転送
                AppLogUtil.OutputLogText(ReceivedLogBuffer);
                OnReceivedResponse(receivedCID, ReceivedCMD, ReceivedMessage, true, AppCommon.MSG_NONE);
            }
        }

        private void AppendLogToBuffer(string message)
        {
            // メッセージに現在時刻を付加する
            string formatted = string.Format("{0} [debug] {1}", DateTime.Now.ToString(), message);

            // メッセージを一時領域に連結
            if (ReceivedLogBuffer.Length > 0) {
                ReceivedLogBuffer += "\r\n";
            }
            ReceivedLogBuffer += formatted;
        }

        //
        // 応答タイムアウト時の処理
        //
        private void OnResponseTimerElapsed(object sender, EventArgs e)
        {
            // 応答タイムアウトを通知
            OnReceivedResponse(new byte[4], 0, Array.Empty<byte>(), false, AppCommon.MSG_REQUEST_SEND_TIMED_OUT);
        }
    }
}
