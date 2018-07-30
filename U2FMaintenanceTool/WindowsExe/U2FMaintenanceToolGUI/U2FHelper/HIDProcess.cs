using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace U2FHelper
{
    internal class HIDProcess
    {
        // メイン画面の参照を保持
        private MainForm mainForm;
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

        public HIDProcess()
        {
        }

        public void OnFormCreate(MainForm f)
        {
            // メイン画面の参照を保持
            mainForm = f;

            DevBroadcastDeviceinterface dbi = new DevBroadcastDeviceinterface {
                DeviceType = DbtDevtypDeviceinterface,
                Reserved = 0,
                ClassGuid = GuidDevinterfaceUSBDevice,
                Name = 0
            };

            dbi.Size = Marshal.SizeOf(dbi);
            IntPtr buffer = Marshal.AllocHGlobal(dbi.Size);
            Marshal.StructureToPtr(dbi, buffer, true);

            notificationHandle = RegisterDeviceNotification(f.Handle, buffer, 0);
            if (notificationHandle == null) {
                mainForm.PrintText("USBデバイス検知の開始に失敗しました.\r\n");
            }

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
            }
        }

        public void OnUSBDeviceArrival()
        {
            // U2F HIDデバイスに自動接続
            StartAsyncOperation();
        }

        public void OnUSBDeviceRemoveComplete()
        {
            if (GetHIDDevicePath().Equals("")) {
                // U2F HIDデバイスが切断されてしまった場合
                CloseDevice();
                mainForm.PrintText("U2F HIDデバイスが取り外されました. \r\n");
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
            // 0.5秒待機後に、もう一度U2F HIDデバイスの有無を確認
            await Task.Run(() => System.Threading.Thread.Sleep(500));
            string devicePath = GetHIDDevicePath();
            if (devicePath.Equals("")) {
                return;
            }

            // デバイスを初期化し、イベントを登録
            device = new HIDDevice(devicePath);
            device.dataReceived += new HIDDevice.dataReceivedEvent(Device_dataReceived);
            mainForm.PrintText(string.Format("U2F HIDデバイスに接続されました: {0}\r\n", devicePath));
        }

        private void Device_dataReceived(byte[] message)
        {
            if (message == null) {
                return;
            }
            for (int i = 0; i < message.Length; i++) {
                mainForm.PrintText(string.Format("{0:x2} ", message[i]));
                if (i % 16 == 15) {
                    mainForm.PrintText("\r\n");
                }
            }
            mainForm.PrintText("\r\n");

            // for research
            byte[] writeData = {
                0x00, 0x00, 0x00, 0x00,
                0x83, 0x00, 0x01,
                0xff,
                0x90, 0x00};
            device.Write(writeData);
        }

        private void CloseDevice()
        {
            if (device == null) {
                return;
            }
            device.Close();
            device = null;
        }
    }
}
