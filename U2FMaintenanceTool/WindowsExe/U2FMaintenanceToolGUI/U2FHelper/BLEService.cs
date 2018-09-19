using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Storage.Streams;

namespace U2FHelper
{
    public class BLEService
    {
        private GattDeviceService service;
        private GattCharacteristic U2FControlPointChar;
        private GattCharacteristic U2FStatusChar;
        private MainForm mainForm;

        private Guid U2F_BLE_SERVICE_UUID = new Guid("0000FFFD-0000-1000-8000-00805f9b34fb");
        private Guid U2F_CONTROL_POINT_CHAR_UUID = new Guid("F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB");
        private Guid U2F_STATUS_CHAR_UUID = new Guid("F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB");

        private BluetoothLEAdvertisementWatcher watcher;
        private ulong BluetoothAddress;

        public delegate void dataReceivedEvent(byte[] message, int length);
        public event dataReceivedEvent DataReceived;

        public delegate void oneCardPeripheralPairedEvent(bool success);
        public event oneCardPeripheralPairedEvent OneCardPeripheralPaired;

        public delegate void oneCardPeripheralFoundEvent();
        public event oneCardPeripheralFoundEvent OneCardPeripheralFound;

        public BLEService()
        {
            watcher = new BluetoothLEAdvertisementWatcher();
            watcher.Received += OnAdvertisementReceived;
        }

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

        public async Task<bool> Send(byte[] u2fRequestFrameData, int frameLen)
        {
            if (service == null) {
                OutputLogToFile(string.Format("BLEService.Send: service is null"));
                return false;
            }

            try {
                // リクエストデータを生成
                DataWriter writer = new DataWriter();
                for (int i = 0; i < frameLen; i++) {
                    writer.WriteByte(u2fRequestFrameData[i]);
                }

                // リクエストを実行（U2F Control Pointに書込）
                GattCommunicationStatus result = await U2FControlPointChar.WriteValueAsync(writer.DetachBuffer(), GattWriteOption.WriteWithoutResponse);
                if (result != GattCommunicationStatus.Success) {
                    OutputLogToFile(AppCommon.MSG_REQUEST_SEND_FAILED);
                    StopCommunicate();
                    return false;
                }

                return true;

            } catch (Exception e) {
                OutputLogToFile(string.Format("BLEService.Send: {0}", e.Message));
                StopCommunicate();
                return false;
            }
        }

        public async void Pair()
        {
            OutputLogToFile("One Cardとのペアリングを開始します。");
            BluetoothAddress = 0;
            watcher.Start();

            // One Cardがみつかるまで待機（最大10秒）
            OutputLogToFile("One Cardからのアドバタイズ監視を開始します。");
            for (int i = 0; i < 10 && BluetoothAddress == 0; i++) {
                await Task.Run(() => System.Threading.Thread.Sleep(1000));
            }

            watcher.Stop();
            OutputLogToFile("One Cardからのアドバタイズ監視を終了しました。");

            if (BluetoothAddress != 0) {
                // One Cardが見つかった場合はペアリング実行
                OneCardPeripheralFound();
            } else {
                // 画面スレッドに失敗を通知
                OneCardPeripheralPaired(false);
            }
        }

        private void OnAdvertisementReceived(BluetoothLEAdvertisementWatcher watcher, BluetoothLEAdvertisementReceivedEventArgs eventArgs)
        {
            // One Cardが見つかったら、
            // アドレス情報を保持し、画面スレッドに通知
            string name = eventArgs.Advertisement.LocalName;
            if (name == "OneCard_Peripheral") {
                BluetoothAddress = eventArgs.BluetoothAddress;
                OutputLogToFile(string.Format("One Cardが見つかりました: BluetoothAddress={0}", BluetoothAddress));
            }
        }

        private void CustomOnPairingRequested(DeviceInformationCustomPairing sender, DevicePairingRequestedEventArgs args)
        {
            OutputLogToFile("One Cardとのペアリングが自動的に実行されます。");
            args.Accept();
        }

        public async void PairWithOneCardPeripheral()
        {
            bool success = false;
            try {
                // デバイス情報を取得
                BluetoothLEDevice device = await BluetoothLEDevice.FromBluetoothAddressAsync(BluetoothAddress);
                DeviceInformation deviceInfoForPair = device.DeviceInformation;

                // ペアリング実行
                deviceInfoForPair.Pairing.Custom.PairingRequested += CustomOnPairingRequested;
                DevicePairingResult result = await deviceInfoForPair.Pairing.Custom.PairAsync(
                                DevicePairingKinds.ConfirmOnly, 
                                DevicePairingProtectionLevel.Encryption);
                deviceInfoForPair.Pairing.Custom.PairingRequested -= CustomOnPairingRequested;

                // ペアリングが正常終了したら処理完了
                if (result.Status == DevicePairingResultStatus.Paired ||
                    result.Status == DevicePairingResultStatus.AlreadyPaired) {
                    success = true;
                    OutputLogToFile("One Cardとのペアリングが成功しました。");
                } else {
                    success = false;
                    OutputLogToFile("One Cardとのペアリングが失敗しました。");
                }

                // BLEデバイスを解放
                device.Dispose();
                device = null;

            } catch (Exception e) {
                OutputLogToFile(string.Format("BLEService.PairWithOneCardPeripheral: {0}", e.Message));
            }

            // 画面スレッドに成否を通知
            OneCardPeripheralPaired(success);
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
                OutputLogToFile(string.Format("BLEService.StartCommunicate: {0}", e.Message));
                StopCommunicate();
                return false;
            }
        }

        private void OnCharacteristicValueChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            try {
                // レスポンスを受領（U2F Statusを読込）
                uint len = eventArgs.CharacteristicValue.Length;
                byte[] responseBytes = new byte[len];
                DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(responseBytes);

                // レスポンスを転送
                DataReceived(responseBytes, (int)len);

            } catch (Exception e) {
                OutputLogToFile(string.Format("BLEService.OnCharacteristicValueChanged: {0}", e.Message));
            }
        }

        private void StopCommunicate()
        {
            try {
                U2FStatusChar.ValueChanged -= OnCharacteristicValueChanged;
                service.Dispose();

            } catch (Exception e) {
                OutputLogToFile(string.Format("BLEService.StopCommunicate: {0}", e.Message));
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
