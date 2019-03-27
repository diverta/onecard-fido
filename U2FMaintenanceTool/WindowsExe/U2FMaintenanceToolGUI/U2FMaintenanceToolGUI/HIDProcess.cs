using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using U2FMaintenanceToolCommon;

namespace U2FMaintenanceToolGUI
{
    internal class HIDProcess
    {
        // HIDデバイス管理
        private HIDDevice device = null;

        // デバイス管理関連
        private const int DbtDevtypDeviceinterface = 5;
        private static readonly Guid GuidDevinterfaceUSBDevice = new Guid("A5DCBF10-6530-11D2-901F-00C04FB951ED");
        private static IntPtr notificationHandle;
        private const string USER32_DLL = "user32.dll";

        [DllImport(USER32_DLL, CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr RegisterDeviceNotification(
            IntPtr recipient, IntPtr notificationFilter, int flags);

        [DllImport(USER32_DLL)]
        private static extern bool UnregisterDeviceNotification(IntPtr handle);

        [StructLayout(LayoutKind.Sequential)]
        private struct DevBroadcastDeviceinterface
        {
            internal int Size;
            internal int DeviceType;
            internal int Reserved;
            internal Guid ClassGuid;
            internal short Name;
        }

        // メッセージテキスト送信用のイベント
        public delegate void MessageTextEventHandler(string messageText);
        public event MessageTextEventHandler MessageTextEvent;

        // HIDメッセージ受信時のイベント
        public delegate void ReceiveHIDMessageEventHandler(byte[] message, int length);
        public event ReceiveHIDMessageEventHandler ReceiveHIDMessageEvent;

        public HIDProcess()
        {
        }

        public void OnFormCreate(IntPtr handle)
        {
            DevBroadcastDeviceinterface dbi = new DevBroadcastDeviceinterface {
                DeviceType = DbtDevtypDeviceinterface,
                Reserved = 0,
                ClassGuid = GuidDevinterfaceUSBDevice,
                Name = 0
            };

            dbi.Size = Marshal.SizeOf(dbi);
            IntPtr buffer = Marshal.AllocHGlobal(dbi.Size);
            Marshal.StructureToPtr(dbi, buffer, true);

            notificationHandle = RegisterDeviceNotification(handle, buffer, 0);
            if (notificationHandle == null) {
                MessageTextEvent(AppCommon.MSG_USB_DETECT_FAILED);
                OutputLogToFile(AppCommon.MSG_USB_DETECT_FAILED);
                return;
            }
            OutputLogToFile(AppCommon.MSG_USB_DETECT_STARTED);

            // USB HIDデバイスに自動接続
            StartAsyncOperation();
        }

        public void OnFormDestroy()
        {
            // USB HIDデバイスを切断
            CloseDevice();

            // USBデバイス検知を終了
            if (notificationHandle != null) {
                UnregisterDeviceNotification(notificationHandle);
                OutputLogToFile(AppCommon.MSG_USB_DETECT_END);
            }
        }

        public void OnUSBDeviceArrival()
        {
            // USB HIDデバイスに自動接続
            StartAsyncOperation();
        }

        public void OnUSBDeviceRemoveComplete()
        {
            // デバイスが既に切断されている場合は終了
            if (device == null) {
                return;
            }

            if (GetHIDDevicePath().Equals("")) {
                // USB HIDデバイスが切断されてしまった場合
                CloseDevice();
                MessageTextEvent(AppCommon.MSG_HID_REMOVED);
            }
        }

        public bool IsUSBDeviceDisconnected()
        {
            return (device == null);
        }

        private string GetHIDDevicePath()
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

            // デバイスを初期化し、イベントを登録
            device = new HIDDevice(devicePath);
            device.dataReceived += new HIDDevice.dataReceivedEvent(Device_dataReceived);
            MessageTextEvent(AppCommon.MSG_HID_CONNECTED);
            OutputLogToFile(string.Format(AppCommon.MSG_HID_CONNECTED + "{0}", devicePath));
        }

        // 受信データを保持
        private byte[] receivedMessage = new byte[1024];
        private int receivedMessageLen = 0;
        private int received = 0;

        // 受信したCID、コマンドを保持
        public byte[] receivedCID = new byte[4];
        public byte receivedCMD;

        // フレームデータを保持
        private byte[] frameData = new byte[Const.HID_FRAME_LEN];

        private void Device_dataReceived(byte[] message)
        {
            // メッセージは最低 8 バイト
            if (message.Length < 8) {
                OutputLogToFile(String.Format(
                    "SendHIDMessage: invalid received message size({0})", message.Length));
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
            int hid_init_data_len = 57;
            int hid_cont_data_len = 59;
            if (frameData[4] > 127) {
                // INITフレームであると判断
                receivedCMD = frameData[4];
                byte cnth = frameData[5];
                byte cntl = frameData[6];
                receivedMessageLen = cnth * 256 + cntl;
                received = 0;

                // データをコピー
                int dataLenInFrame = (receivedMessageLen < hid_init_data_len) ? receivedMessageLen : hid_init_data_len;
                for (int i = 0; i < dataLenInFrame; i++) {
                    receivedMessage[received++] = frameData[Const.HID_INIT_HEADER_LEN + i];
                }
                OutputLogToFile(string.Format(
                    "INIT frame: data size={0} length={1} cmd=0x{2:x2}",
                    receivedMessageLen, dataLenInFrame, receivedCMD));

            } else {
                // CONTフレームであると判断
                int seq = frameData[4];

                // データをコピー
                int remaining = receivedMessageLen - received;
                int dataLenInFrame = (remaining < hid_cont_data_len) ? remaining : hid_cont_data_len;
                for (int i = 0; i < dataLenInFrame; i++) {
                    receivedMessage[received++] = frameData[Const.HID_CONT_HEADER_LEN + i];
                }

                OutputLogToFile(string.Format(
                    "CONT frame: seq={0} length={1}", 
                    seq, dataLenInFrame));
            }

            // メッセージをダンプ
            DumpMessage(frameData, frameData.Length);

            if (received == receivedMessageLen) {
                // 全フレームを受信できたら、
                // HIDデバイスからのデータをHIDMainに転送
                OutputLogToFile(AppCommon.MSG_HID_RESPONSE_RECEIVED);
                ReceiveHIDMessageEvent(receivedMessage, receivedMessageLen);
            }
        }

        public void SendHIDMessage(byte[] cid, byte cmd, byte[] message)
        {
            // メッセージがない場合は終了
            int transferMessageLen = message.Length;
            if (transferMessageLen == 0) {
                OutputLogToFile("SendHIDMessage: invalid message size(0)");
                return;
            }
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
            while (transferred < transferMessageLen) {
                for (int j = 0; j < frameData.Length; j++) {
                    // フレームデータを初期化
                    frameData[j] = 0;
                }
                for (int c = 0; c < cid.Length; c++) {
                    // CIDをコピー
                    frameData[c] = cid[c];
                }

                if (transferred == 0) {
                    // INITフレーム
                    // ヘッダーを設定
                    frameData[4] = cmd;
                    frameData[5] = (byte)(transferMessageLen / 256);
                    frameData[6] = (byte)(transferMessageLen % 256);

                    // データをコピー
                    int maxLen = Const.HID_FRAME_LEN - Const.HID_INIT_HEADER_LEN;
                    int dataLenInFrame = (transferMessageLen < maxLen) ? transferMessageLen : maxLen;
                    for (int i = 0; i < dataLenInFrame; i++) {
                        frameData[Const.HID_INIT_HEADER_LEN + i] = message[transferred++];
                    }

                    OutputLogToFile(string.Format(
                        "INIT frame: data size={0} length={1}",
                        transferMessageLen, dataLenInFrame));
                    DumpMessage(frameData, Const.HID_INIT_HEADER_LEN + dataLenInFrame);

                } else {
                    // CONTフレーム
                    // ヘッダーをコピー
                    frameData[4] = (byte)seq;

                    // データをコピー
                    int remaining = transferMessageLen - transferred;
                    int maxLen = Const.HID_FRAME_LEN - Const.HID_CONT_HEADER_LEN;
                    int dataLenInFrame = (remaining < maxLen) ? remaining : maxLen;
                    for (int i = 0; i < dataLenInFrame; i++) {
                        frameData[Const.HID_CONT_HEADER_LEN + i] = message[transferred++];
                    }

                    OutputLogToFile(string.Format(
                        "CONT frame: data seq={0} length={1}",
                        seq++, dataLenInFrame));
                    DumpMessage(frameData, Const.HID_CONT_HEADER_LEN + dataLenInFrame);
                }

                // フレームデータを転送
                device.Write(frameData);
            }

            // 転送完了メッセージ
            OutputLogToFile(string.Format(AppCommon.MSG_HID_REQUEST_SENT));
        }

        private void DumpMessage(byte[] message, int length)
        {
            OutputLogToFile(AppCommon.DumpMessage(message, length), false);
        }

        private void CloseDevice()
        {
            if (device == null) {
                return;
            }
            device.Close();
            device = null;
        }

        private void OutputLogToFile(string message)
        {
            // メッセージに現在時刻を付加し、ログファイルに出力する
            OutputLogToFile(message, true);
        }

        private void OutputLogToFile(string message, bool printTimeStamp)
        {
            // メッセージにログファイルに出力する
            // printTimeStampがfalse時は、現在時刻を付加しない
            AppCommon.OutputLogToFile(message, printTimeStamp);
        }
    }
}
