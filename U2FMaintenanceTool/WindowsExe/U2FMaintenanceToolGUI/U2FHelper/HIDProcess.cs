using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace U2FHelper
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
                MessageTextEvent(AppCommon.MSG_USB_DETECT_FAILED + "\r\n");
                OutputLogToFile(AppCommon.MSG_USB_DETECT_FAILED);
                return;
            }
            OutputLogToFile(AppCommon.MSG_USB_DETECT_STARTED);

            // U2F HIDデバイスに自動接続
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
            // U2F HIDデバイスに自動接続
            StartAsyncOperation();
        }

        public void OnUSBDeviceRemoveComplete()
        {
            // デバイスが既に切断されている場合は終了
            if (device == null) {
                return;
            }

            if (GetHIDDevicePath().Equals("")) {
                // U2F HIDデバイスが切断されてしまった場合
                CloseDevice();
                MessageTextEvent(AppCommon.MSG_HID_REMOVED + "\r\n");
            }
        }

        private string GetHIDDevicePath()
        {
            // 接続済みのHIDデバイスから、
            // VID=0xf055であるデバイスの
            // ２番目のインターフェースを探す
            HIDDevice.InterfaceDetails[] devices = HIDDevice.GetConnectedDevices();
            for (int i = 0; i < devices.Length; i++) {
                ushort VID = devices[i].VID;
                string devicePath = devices[i].devicePath;
                if (VID == 0xf055 && devicePath.Contains("mi_01")) {
                    return devicePath;
                }
            }
            return "";
        }

        private async void StartAsyncOperation()
        {
            // 0.25秒待機後に、もう一度U2F HIDデバイスの有無を確認
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
            MessageTextEvent(AppCommon.MSG_HID_CONNECTED + "\r\n");
            OutputLogToFile(string.Format(AppCommon.MSG_HID_CONNECTED + "{0}", devicePath));
        }

        // 受信データを保持
        private byte[] receivedHeader = new byte[3];
        private byte[] receivedMessage = new byte[1024];
        private int receivedMessageLen = 0;
        private int received = 0;

        private void Device_dataReceived(byte[] message)
        {
            if (message == null) {
                return;
            }
            // 
            // 受信したデータをバッファにコピー
            // 
            //  INITフレーム
            //  1     バイト目: レポートID
            //  2 - 5 バイト目: CID
            //  6     バイト目: コマンド
            //  7 - 8 バイト目: データ長
            //  残りのバイト  : データ部（33 - 8 = 25）
            //
            //  CONTフレーム
            //  1     バイト目: レポートID
            //  2 - 5 バイト目: CID
            //  6     バイト目: シーケンス
            //  残りのバイト  : データ部（33 - 6 = 27）
            // 
            byte cmd = message[5];
            if (cmd > 127) {
                // INITフレームであると判断
                byte cnth = message[6];
                byte cntl = message[7];
                receivedMessageLen = cnth * 256 + cntl;
                received = 0;

                // ヘッダーをコピー
                for (int i = 0; i < receivedHeader.Length; i++) {
                    receivedHeader[i] = message[5 + i];
                }

                // データをコピー
                int dataLenInFrame = (receivedMessageLen < 25) ? receivedMessageLen : 25;
                for (int i = 0; i < dataLenInFrame; i++) {
                    receivedMessage[received++] = message[8 + i];
                }

                OutputLogToFile(string.Format(
                    "INIT frame: data size={0} length={1}",
                    receivedMessageLen, dataLenInFrame));

            } else {
                // CONTフレームであると判断
                int seq = message[5];

                // データをコピー
                int remaining = receivedMessageLen - received;
                int dataLenInFrame = (remaining < 27) ? remaining : 27;
                for (int i = 0; i < dataLenInFrame; i++) {
                    receivedMessage[received++] = message[6 + i];
                }

                OutputLogToFile(string.Format(
                    "CONT frame: seq={0} length={1}", 
                    seq, dataLenInFrame));
            }

            // メッセージをダンプ
            DumpMessage(message, message.Length);

            if (received == receivedMessageLen) {
                // メッセージをダンプ
                OutputLogToFile(string.Format(AppCommon.MSG_HID_MESSAGE_TRANSFERRED + "size={0}", receivedMessageLen));
                DumpMessage(receivedMessage, receivedMessageLen);

                // U2F管理コマンドを別プロセスで起動し、
                // HIDデバイスからのデータを標準出力経由で転送
                ReceiveHIDMessageEvent(receivedMessage, receivedMessageLen);
            }
        }

        public bool XferMessage()
        {
            // BLEから送信されたメッセージを
            // HIDデバイスに転送する処理

            // for research
            byte[] frameData = {
                0x00, 0x00, 0x00, 0x00,
                0x83, 0x00, 0x01,
                0xff,
                0x90, 0x00};
            device.Write(frameData);

            return true;
        }

        private void DumpMessage(byte[] message, int length)
        {
            StringBuilder sb = new StringBuilder();
            ;
            for (int i = 0; i < length; i++) {
                sb.Append(string.Format("{0:x2} ", message[i]));
                if (i % 16 == 15) {
                    sb.Append("\r\n");
                }
            }
            OutputLogToFile(sb.ToString(), false);
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
            OutputLogToFile(message, true);
        }

        private void OutputLogToFile(string message, bool printTimeStamp)
        {
            // メッセージに現在時刻を付加する
            string formatted;
            if (printTimeStamp) {
                formatted = string.Format("{0} {1}", DateTime.Now.ToString(), message);
            } else {
                formatted = string.Format("{0}", message);
            }

            // ログファイルにメッセージを出力する
            string fname = AppCommon.FILENAME_U2FHELPER_LOG;
            StreamWriter sr = new StreamWriter(
                (new FileStream(fname, FileMode.Append)),
                System.Text.Encoding.Default);
            sr.WriteLine(formatted);
            sr.Close();
        }
    }
}
