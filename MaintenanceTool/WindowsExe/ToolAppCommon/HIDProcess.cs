using MaintenanceToolApp;
using System.Threading.Tasks;

namespace ToolAppCommon
{
    public class HIDProcess
    {
        // このクラスのインスタンス
        private static readonly HIDProcess Instance = new HIDProcess();

        // HID接続完了時のイベント
        public delegate void HandlerOnConnectHIDDevice(bool connected);
        public event HandlerOnConnectHIDDevice OnConnectHIDDevice = null!;

        //
        // 外部公開用
        //
        public static void RegisterHandlerOnConnectHIDDevice(HandlerOnConnectHIDDevice handler)
        {
            Instance.OnConnectHIDDevice += handler;
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
    }
}
