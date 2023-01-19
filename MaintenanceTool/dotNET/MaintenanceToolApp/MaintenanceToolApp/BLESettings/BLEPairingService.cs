using System;
using System.Linq;
using System.Threading.Tasks;
using ToolAppCommon;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Enumeration;
using Windows.Devices.Radios;
using Windows.Storage.Streams;

namespace MaintenanceToolApp.BLESettings
{
    internal class BLEPairingService
    {
        //
        // BLEペアリング関連イベント
        //
        public delegate void HandlerOnFIDOPeripheralFound(bool found, ulong bluetoothAddress, string errorMessage);
        private event HandlerOnFIDOPeripheralFound OnFIDOPeripheralFound = null!;

        public delegate void HandlerOnFIDOPeripheralPaired(bool success, string errorMessage);
        private event HandlerOnFIDOPeripheralPaired OnFIDOPeripheralPaired = null!;

        // 監視対象UUID
        private readonly Guid U2F_BLE_SERVICE_UUID = new Guid(BLEServiceConst.U2F_BLE_SERVICE_UUID_STR);

        private BluetoothLEAdvertisementWatcher watcher = null!;
        private ulong BluetoothAddress = 0;
        private byte[] ServiceDataField = Array.Empty<byte>();
        private string Passkey = string.Empty;

        public BLEPairingService()
        {
            watcher = new BluetoothLEAdvertisementWatcher();
            watcher.Received += OnAdvertisementReceived;
        }

        //
        // ペアリング対象のFIDO認証器を検索
        //
        public async void FindFIDOPeripheral(HandlerOnFIDOPeripheralFound handler)
        {
            // イベントを登録
            OnFIDOPeripheralFound += handler;

            // Bluetoothがオンになっていることを確認
            bool bton = false;
            try {
                var radios = await Radio.GetRadiosAsync();
                foreach (var radio in radios) {
                    if (radio.Kind == RadioKind.Bluetooth) {
                        if (radio.State == RadioState.On) {
                            bton = true;
                            break;
                        }
                    }
                }
            } catch {
                // Bluetoothオン状態が確認できない場合は失敗を通知
                FuncOnFIDOPeripheralFound(false, 0, AppCommon.MSG_BLE_PARING_ERR_BT_STATUS_CANNOT_GET, handler);
                return;
            }

            if (bton == false) {
                // Bluetoothがオンになっていない場合は失敗を通知
                FuncOnFIDOPeripheralFound(false, 0, AppCommon.MSG_BLE_PARING_ERR_BT_OFF, handler);
                return;
            }

            // FIDO認証器からのアドバタイズ監視を開始
            WatchAdvertisement(handler);
        }

        private async void WatchAdvertisement(HandlerOnFIDOPeripheralFound handler)
        {
            // FIDO認証器からのアドバタイズ監視を開始
            AppLogUtil.OutputLogDebug("Watch FIDO BLE device advertisement start");
            BluetoothAddress = 0;
            watcher.Start();

            // FIDO認証器がみつかるまで待機（最大10秒）
            for (int i = 0; i < 10 && BluetoothAddress == 0; i++) {
                await Task.Run(() => System.Threading.Thread.Sleep(1000));
            }

            // FIDO認証器からのアドバタイズ監視を終了
            watcher.Stop();
            AppLogUtil.OutputLogDebug("Watch FIDO BLE device advertisement end");

            if (BluetoothAddress == 0) {
                // FIDO認証器が見つからなかった場合は失敗を通知
                FuncOnFIDOPeripheralFound(false, 0, AppCommon.MSG_BLE_PARING_ERR_TIMED_OUT, handler);
                return;
            }

            // FIDO認証器が見つかった場合でも、所定のサービスデータフィールドが存在しない場合は失敗を通知
            byte[] expect = { 0xfd, 0xff, 0x80 };
            if (ServiceDataField.Length != 3 || ServiceDataField.SequenceEqual(expect) == false) {
                FuncOnFIDOPeripheralFound(false, BluetoothAddress, AppCommon.MSG_BLE_PARING_ERR_PAIR_MODE, handler);
                return;
            }

            // FIDO認証器が見つかった場合は成功を通知
            FuncOnFIDOPeripheralFound(true, BluetoothAddress, AppCommon.MSG_NONE, handler);
        }

        private void OnAdvertisementReceived(BluetoothLEAdvertisementWatcher watcher, BluetoothLEAdvertisementReceivedEventArgs eventArgs)
        {
            // FIDO認証器が見つかったら、
            // アドレス情報を保持し、画面スレッドに通知
            string name = eventArgs.Advertisement.LocalName;
            foreach (Guid g in eventArgs.Advertisement.ServiceUuids) {
                if (g.Equals(U2F_BLE_SERVICE_UUID)) {
                    BluetoothAddress = eventArgs.BluetoothAddress;
                    AppLogUtil.OutputLogDebug("FIDO BLE device found.");
                    // アドバタイズデータからサービスデータフィールドを抽出
                    ServiceDataField = RetrieveServiceDataField(eventArgs.Advertisement);
                    break;
                }
            }
        }

        private static byte[] RetrieveServiceDataField(BluetoothLEAdvertisement advertisement)
        {
            // アドバタイズデータを走査
            byte[] serviceDataField = Array.Empty<byte>();
            foreach (BluetoothLEAdvertisementDataSection datasection in advertisement.DataSections) {
                // サービスデータフィールドの場合は格納領域に設定
                byte dataType = datasection.DataType;
                if (dataType == 0x16) {
                    serviceDataField = new byte[datasection.Data.Length];
                    using (DataReader reader = DataReader.FromBuffer(datasection.Data)) {
                        reader.ReadBytes(serviceDataField);
                        break;
                    }
                }
            }
            if (serviceDataField.Length == 0) {
                AppLogUtil.OutputLogDebug("Service data field not found");
            } else {
                string dump = AppLogUtil.DumpMessage(serviceDataField, serviceDataField.Length);
                AppLogUtil.OutputLogDebug(string.Format("Service data field found ({0} bytes) {1}", serviceDataField.Length, dump));
            }
            return serviceDataField;
        }

        private void FuncOnFIDOPeripheralFound(bool found, ulong bluetoothAddress, string errorMessage, HandlerOnFIDOPeripheralFound handler)
        {
            OnFIDOPeripheralFound(found, bluetoothAddress, errorMessage);
            OnFIDOPeripheralFound -= handler;
        }

        //
        // ペアリング実行
        //
        public async void PairWithFIDOPeripheral(ulong bluetoothAddress, string passcode, HandlerOnFIDOPeripheralPaired handler)
        {
            // イベントを登録
            OnFIDOPeripheralPaired += handler;

            // 変数を初期化
            bool success = false;
            string errorMessage = "";

            // パスコードを設定
            Passkey = passcode;

            try {
                // デバイス情報を取得
                BluetoothLEDevice? device = await BluetoothLEDevice.FromBluetoothAddressAsync(bluetoothAddress);
                DeviceInformation deviceInfoForPair = device.DeviceInformation;

                // ペアリング実行
                deviceInfoForPair.Pairing.Custom.PairingRequested += CustomOnPairingRequested;
                DevicePairingResult result;
                if (Passkey == null || Passkey.Length == 0) {
                    // パスキーが指定されていない場合
                    result = await deviceInfoForPair.Pairing.Custom.PairAsync(
                        DevicePairingKinds.ConfirmOnly, DevicePairingProtectionLevel.Encryption);

                } else {
                    // パスキーが指定されている場合は、パスキーを使用
                    result = await deviceInfoForPair.Pairing.Custom.PairAsync(
                        DevicePairingKinds.ProvidePin, DevicePairingProtectionLevel.EncryptionAndAuthentication);
                }
                deviceInfoForPair.Pairing.Custom.PairingRequested -= CustomOnPairingRequested;

                // ペアリングが正常終了したら処理完了
                if (result.Status == DevicePairingResultStatus.Paired ||
                    result.Status == DevicePairingResultStatus.AlreadyPaired) {
                    success = true;
                    AppLogUtil.OutputLogDebug("Pairing with FIDO device success");

                } else if (result.Status == DevicePairingResultStatus.Failed) {
                    errorMessage = AppCommon.MSG_BLE_PARING_ERR_PAIR_MODE;
                    AppLogUtil.OutputLogError("Pairing with FIDO device fail");

                } else {
                    errorMessage = AppCommon.MSG_BLE_PARING_ERR_UNKNOWN;
                    AppLogUtil.OutputLogError(string.Format("Pairing with FIDO device fail: reason={0}", result.Status));
                }

                // BLEデバイスを解放
                device.Dispose();
                device = null;

            } catch (Exception e) {
                errorMessage = AppCommon.MSG_BLE_PARING_ERR_UNKNOWN;
                AppLogUtil.OutputLogError(string.Format("Pairing with FIDO device fail: {0}", e.Message));
            }

            // 上位クラスに成否を通知
            OnFIDOPeripheralPaired(success, errorMessage);
            OnFIDOPeripheralPaired -= handler;
        }

        private void CustomOnPairingRequested(DeviceInformationCustomPairing sender, DevicePairingRequestedEventArgs args)
        {
            if (args.PairingKind == DevicePairingKinds.ProvidePin && Passkey != null) {
                args.Accept(Passkey);
            } else {
                args.Accept();
            }
        }
    }
}
