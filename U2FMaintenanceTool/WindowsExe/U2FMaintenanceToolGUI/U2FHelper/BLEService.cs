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

        public delegate void dataReceivedEvent(byte[] message, int length);
        public event dataReceivedEvent DataReceived;

        public void SetMainForm(MainForm f)
        {
            // メインフォームの参照を保持
            mainForm = f;
        }

        public async Task<bool> Connect()
        {
            // One CardとのBLE通信を開始
            if (await StartCommunicate() == false) {
                OutputLogToFile(AppCommon.MSG_U2F_DEVICE_CONNECT_FAILED);
                return false;
            }
            OutputLogToFile(AppCommon.MSG_U2F_DEVICE_CONNECTED);
            return true;
        }

        public void Disconnect()
        {
            // 切断
            StopCommunicate();
            OutputLogToFile(AppCommon.MSG_U2F_DEVICE_DISCONNECTED);
        }

        public async Task<bool> Send(byte[] u2fVersionFrameData)
        {
            // リクエストデータを生成
            DataWriter writer = new DataWriter();
            writer.WriteBytes(u2fVersionFrameData);

            // リクエストを実行（U2F Control Pointに書込）
            GattCommunicationStatus result = await U2FControlPointChar.WriteValueAsync(writer.DetachBuffer(), GattWriteOption.WriteWithoutResponse);
            if (result != GattCommunicationStatus.Success) {
                OutputLogToFile(AppCommon.MSG_REQUEST_SEND_FAILED);
                StopCommunicate();
                return false;
            }

            OutputLogToFile(AppCommon.MSG_REQUEST_SENT);
            return true;
        }

        private async Task<bool> StartCommunicate()
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
                return false;
            }
            OutputLogToFile(string.Format("{0} (name={1} isEnabled={2})",
                AppCommon.MSG_BLE_U2F_SERVICE_FOUND, infoBLE.Name, infoBLE.IsEnabled));

            service = null;
            try {
                service = await GattDeviceService.FromIdAsync(infoBLE.Id);
                if (service == null) {
                    OutputLogToFile(AppCommon.MSG_BLE_CHARACT_NOT_DISCOVERED);
                    return false;
                }

                U2FControlPointChar = service.GetCharacteristics(U2F_CONTROL_POINT_CHAR_UUID)[0];
                U2FStatusChar = service.GetCharacteristics(U2F_STATUS_CHAR_UUID)[0];
                U2FStatusChar.ValueChanged += OnCharacteristicValueChanged;

                GattCommunicationStatus result = await U2FStatusChar.WriteClientCharacteristicConfigurationDescriptorAsync(
                    GattClientCharacteristicConfigurationDescriptorValue.Notify);
                if (result != GattCommunicationStatus.Success) {
                    OutputLogToFile(AppCommon.MSG_BLE_NOTIFICATION_FAILED);
                    StopCommunicate();
                    return false;
                }

                OutputLogToFile(AppCommon.MSG_BLE_NOTIFICATION_START);
                return true;

            } catch (Exception e) {
                OutputLogToFile(string.Format("StartCommunicate: {0}", e.Message));
                StopCommunicate();
                return false;
            }
        }

        private void OnCharacteristicValueChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            // レスポンスを受領（U2F Statusを読込）
            uint len = eventArgs.CharacteristicValue.Length;
            byte[] responseBytes = new byte[len];
            DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(responseBytes);

            // レスポンスを転送
            DataReceived(responseBytes, (int)len);
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

        private void OutputLogToFile(string message)
        {
            // メッセージに現在時刻を付加し、ログファイルに出力する
            AppCommon.OutputLogToFile(message, true);
        }
    }
}
