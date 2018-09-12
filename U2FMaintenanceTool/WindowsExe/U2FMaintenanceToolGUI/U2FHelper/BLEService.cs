using System;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Storage.Streams;

namespace U2FHelper
{
    class BLEService
    {
        private GattDeviceService service;
        private GattCharacteristic U2FControlPointChar;
        private GattCharacteristic U2FStatusChar;
        private MainForm mainForm;

        private Guid U2F_BLE_SERVICE_UUID = new Guid("0000FFFD-0000-1000-8000-00805f9b34fb");
        private Guid U2F_CONTROL_POINT_CHAR_UUID = new Guid("F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB");
        private Guid U2F_STATUS_CHAR_UUID = new Guid("F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB");

        // メッセージテキスト送信用のイベント
        public delegate void MessageTextEventHandler(string messageText);
        public event MessageTextEventHandler MessageTextEvent;

        public void SetMainForm(MainForm f)
        {
            // メインフォームの参照を保持
            mainForm = f;
        }

        public async Task GetU2FVersionAsync(object sender, EventArgs e)
        {
            // One CardとのBLE通信を開始
            await StartCommunicate();
            if (service == null) {
                // 接続失敗時は画面表示も行う
                OutputLogToFile(AppCommon.MSG_U2F_DEVICE_CONNECT_FAILED);
                MessageTextEvent(AppCommon.MSG_U2F_DEVICE_CONNECT_FAILED + "\r\n");
                return;
            }
            OutputLogToFile(AppCommon.MSG_U2F_DEVICE_CONNECTED);

            // リクエストデータを生成
            DataWriter writer = new DataWriter();
            byte[] u2fVersionFrameData = {
                    0x83, 0x00, 0x07,
                    0x00, 0x03, 0x00, 0x00,
                    0x00, 0x00, 0x00
                };
            writer.WriteBytes(u2fVersionFrameData);

            // リクエストを実行（U2F Control Pointに書込）
            GattCommunicationStatus result = await U2FControlPointChar.WriteValueAsync(writer.DetachBuffer(), GattWriteOption.WriteWithoutResponse);
            if (result != GattCommunicationStatus.Success) {
                OutputLogToFile(AppCommon.MSG_REQUEST_SEND_FAILED);
                MessageTextEvent(AppCommon.MSG_REQUEST_SEND_FAILED + "\r\n");
                StopCommunicate();
                return;
            }
            OutputLogToFile(AppCommon.MSG_REQUEST_SENT);
        }

        public async Task StartCommunicate()
        {
            string selector = GattDeviceService.GetDeviceSelectorFromUuid(U2F_BLE_SERVICE_UUID);
            DeviceInformationCollection collection = await DeviceInformation.FindAllAsync(selector);

            DeviceInformation infoBLE = null;
            foreach (DeviceInformation info in collection) {
                infoBLE = info;
                break;
            }
            if (infoBLE == null) {
                OutputLogToFile(AppCommon.MSG_BLE_U2F_SERVICE_NOT_FOUND);
                return;
            }
            OutputLogToFile(string.Format("{0} (name={1} isEnabled={2})",
                AppCommon.MSG_BLE_U2F_SERVICE_FOUND, infoBLE.Name, infoBLE.IsEnabled));

            service = null;
            try {
                service = await GattDeviceService.FromIdAsync(infoBLE.Id);
                if (service == null) {
                    OutputLogToFile(AppCommon.MSG_BLE_CHARACT_NOT_DISCOVERED);
                    return;
                }

                U2FControlPointChar = service.GetCharacteristics(U2F_CONTROL_POINT_CHAR_UUID)[0];
                U2FStatusChar = service.GetCharacteristics(U2F_STATUS_CHAR_UUID)[0];
                U2FStatusChar.ValueChanged += OnCharacteristicValueChanged;

                GattCommunicationStatus result = await U2FStatusChar.WriteClientCharacteristicConfigurationDescriptorAsync(
                    GattClientCharacteristicConfigurationDescriptorValue.Notify);
                if (result != GattCommunicationStatus.Success) {
                    OutputLogToFile(AppCommon.MSG_BLE_NOTIFICATION_FAILED);
                    MessageTextEvent(AppCommon.MSG_BLE_NOTIFICATION_FAILED + "\r\n");
                    StopCommunicate();
                    return;
                }
                OutputLogToFile(AppCommon.MSG_BLE_NOTIFICATION_START);

            } catch (Exception e) {
                OutputLogToFile(string.Format("StartCommunicate: {0}", e.Message));
                StopCommunicate();
            }
        }

        private void StopCommunicate()
        {
            try {
                U2FStatusChar.ValueChanged -= OnCharacteristicValueChanged;
                service.Dispose();

            } catch (Exception e) {
                OutputLogToFile(string.Format("StopCommunicate: {0}", e.Message));
            } finally {
                service = null;
            }
        }

        private void OnCharacteristicValueChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            // レスポンスを受領（U2F Statusを読込）
            uint len = eventArgs.CharacteristicValue.Length;
            byte[] responseBytes = new byte[len];
            DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(responseBytes);

            // 受信したデータをHexダンプ
            string dump = AppCommon.DumpMessage(responseBytes, (int)len);
            OutputLogToFile(string.Format("Received {0} bytes: \r\n{1}",
                len, dump));

            // 切断
            StopCommunicate();
            OutputLogToFile(AppCommon.MSG_U2F_DEVICE_DISCONNECTED);

            // テスト終了
            MessageTextEvent("One CardからU2F Versionを取得しました。\r\n");
        }

        private void OutputLogToFile(string message)
        {
            // メッセージに現在時刻を付加し、ログファイルに出力する
            AppCommon.OutputLogToFile(message, true);
        }
    }
}
