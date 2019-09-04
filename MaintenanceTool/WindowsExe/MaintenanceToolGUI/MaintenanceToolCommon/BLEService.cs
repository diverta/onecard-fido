using System;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Devices.Radios;
using Windows.Storage.Streams;

namespace MaintenanceToolCommon
{
    public class BLEService
    {
        private GattDeviceService service;
        private GattCharacteristic U2FControlPointChar;
        private GattCharacteristic U2FStatusChar;

        private Guid U2F_BLE_SERVICE_UUID = new Guid("0000FFFD-0000-1000-8000-00805f9b34fb");
        private Guid U2F_CONTROL_POINT_CHAR_UUID = new Guid("F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB");
        private Guid U2F_STATUS_CHAR_UUID = new Guid("F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB");

        private BluetoothLEAdvertisementWatcher watcher;
        private ulong BluetoothAddress;
        private DeviceInformation infoBLE;

        public delegate void dataReceivedEvent(byte[] message, int length);
        public event dataReceivedEvent DataReceived;

        public delegate void FIDOPeripheralPairedEvent(bool success, string messageOnFail);
        public event FIDOPeripheralPairedEvent FIDOPeripheralPaired;

        public delegate void FIDOPeripheralFoundEvent();
        public event FIDOPeripheralFoundEvent FIDOPeripheralFound;

        public BLEService()
        {
            watcher = new BluetoothLEAdvertisementWatcher();
            watcher.Received += OnAdvertisementReceived;
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
            // Bluetoothがオンになっていることを確認
            bool bton = false;
            try {
                var radios = await Radio.GetRadiosAsync();
                foreach (var radio in radios) {
                    if (radio.Kind == RadioKind.Bluetooth) {
                        if (radio.State == RadioState.On) {
                            OutputLogToFile("Bluetoothはオンです。");
                            bton = true;
                            break;
                        }
                    }
                }
            } catch {
                // Bluetoothオン状態が確認できない場合はオフ状態であるとみなす
                OutputLogToFile("Bluetooth状態を確認できません。");
            }

            if (!bton) {
                // 画面スレッドに失敗を通知
                FIDOPeripheralPaired(false, AppCommon.MSG_BLE_PARING_ERR_BT_OFF);
                OutputLogToFile("Bluetoothはオフです。");
                return;
            }

            OutputLogToFile("FIDO認証器とのペアリングを開始します。");
            BluetoothAddress = 0;
            watcher.Start();

            // FIDO認証器がみつかるまで待機（最大10秒）
            OutputLogToFile("FIDO認証器からのアドバタイズ監視を開始します。");
            for (int i = 0; i < 10 && BluetoothAddress == 0; i++) {
                await Task.Run(() => System.Threading.Thread.Sleep(1000));
            }

            watcher.Stop();
            OutputLogToFile("FIDO認証器からのアドバタイズ監視を終了しました。");

            if (BluetoothAddress != 0) {
                // FIDO認証器が見つかった場合はペアリング実行
                FIDOPeripheralFound();
            } else {
                // 画面スレッドに失敗を通知
                FIDOPeripheralPaired(false, AppCommon.MSG_BLE_PARING_ERR_TIMED_OUT);
                OutputLogToFile("FIDO認証器とのペアリングがタイムアウトしました。");
            }
        }

        private void OnAdvertisementReceived(BluetoothLEAdvertisementWatcher watcher, BluetoothLEAdvertisementReceivedEventArgs eventArgs)
        {
            // FIDO認証器が見つかったら、
            // アドレス情報を保持し、画面スレッドに通知
            string name = eventArgs.Advertisement.LocalName;
            OutputLogToFile(string.Format("BLEデバイス[{0}]が見つかりました: BluetoothAddress={1}, {2} services",
                name, eventArgs.BluetoothAddress, eventArgs.Advertisement.ServiceUuids.Count));
            foreach (Guid g in eventArgs.Advertisement.ServiceUuids) {
                OutputLogToFile(string.Format("  service={0}", g.ToString()));
                if (g.Equals(U2F_BLE_SERVICE_UUID)) {
                    BluetoothAddress = eventArgs.BluetoothAddress;
                    OutputLogToFile("FIDO認証器が見つかりました.");
                    break;
                }
            }
        }

        private void CustomOnPairingRequested(DeviceInformationCustomPairing sender, DevicePairingRequestedEventArgs args)
        {
            OutputLogToFile("FIDO認証器とのペアリングが自動的に実行されます。");
            args.Accept();
        }

        public async void PairWithFIDOPeripheral()
        {
            bool success = false;
            string messageOnFail = "";
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
                    OutputLogToFile("FIDO認証器とのペアリングが成功しました。");
                } else if (result.Status == DevicePairingResultStatus.Failed) {
                    success = false;
                    messageOnFail = AppCommon.MSG_BLE_PARING_ERR_PAIR_MODE;
                    OutputLogToFile("FIDO認証器とのペアリングが失敗しました。");
                } else {
                    success = false;
                    messageOnFail = AppCommon.MSG_BLE_PARING_ERR_UNKNOWN;
                    OutputLogToFile(string.Format("FIDO認証器とのペアリングが失敗しました。reason={0}", result.Status));
                }

                // BLEデバイスを解放
                device.Dispose();
                device = null;

            } catch (Exception e) {
                OutputLogToFile(string.Format("BLEService.PairWithFIDOPeripheral: {0}", e.Message));
            }

            // 画面スレッドに成否を通知
            FIDOPeripheralPaired(success, messageOnFail);
        }

        public async Task<bool> StartCommunicate()
        {
            try {
                OutputLogToFile(string.Format("FIDO BLEサービス({0})を検索します。", U2F_BLE_SERVICE_UUID));
                string selector = GattDeviceService.GetDeviceSelectorFromUuid(U2F_BLE_SERVICE_UUID);
                DeviceInformationCollection collection = await DeviceInformation.FindAllAsync(selector);

                infoBLE = null;
                foreach (DeviceInformation info in collection) {
                    // 受信データ監視を開始
                    if (await StartBLENotification(info)) {
                        OutputLogToFile(string.Format("BLEデバイス [{0}] に接続します。", info.Name));
                        infoBLE = info;
                        break;
                    } else {
                        OutputLogToFile(string.Format("BLEデバイス [{0}] には接続できません。", info.Name));
                    }
                }
                if (infoBLE == null) {
                    OutputLogToFile(AppCommon.MSG_BLE_U2F_SERVICE_NOT_FOUND);
                    return false;
                }
                OutputLogToFile(AppCommon.MSG_BLE_U2F_SERVICE_FOUND);
                return true;

            } catch (Exception e) {
                OutputLogToFile(string.Format("BLEService.StartCommunicate: {0}", e.Message));
                return false;
            }
        }

        public async Task<bool> StartBLENotification(DeviceInformation deviceInfo)
        {
            service = null;
            try {
                service = await GattDeviceService.FromIdAsync(deviceInfo.Id);
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
                OutputLogToFile(string.Format("BLEService.StartBLENotification: {0}", e.Message));
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
                infoBLE = null;
            }
        }

        private void OutputLogToFile(string message)
        {
            // メッセージに現在時刻を付加し、ログファイルに出力する
            AppCommon.OutputLogToFile(message, true);
        }
    }
}
