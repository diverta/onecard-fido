using MaintenanceToolApp;
using MaintenanceToolApp.Common;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Devices.Radios;
using Windows.Storage.Streams;

namespace ToolAppCommon
{
    public class BLEService
    {
        private Guid U2F_BLE_SERVICE_UUID = new Guid("0000FFFD-0000-1000-8000-00805f9b34fb");
        private Guid U2F_CONTROL_POINT_CHAR_UUID = new Guid("F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB");
        private Guid U2F_STATUS_CHAR_UUID = new Guid("F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB");

        // 応答タイムアウト監視用タイマー
        private CommonTimer ResponseTimer = null!;

        public BLEService()
        {
            // 応答タイムアウト発生時のイベントを登録
            ResponseTimer = new CommonTimer("BLEService", 10000);
            ResponseTimer.CommandTimeoutEvent += OnResponseTimerElapsed;

            watcher = new BluetoothLEAdvertisementWatcher();
            watcher.Received += OnAdvertisementReceived;
            FreeResources();
        }

        //
        // BLEペアリング関連
        //
        private BluetoothLEAdvertisementWatcher watcher;
        private ulong BluetoothAddress;

        // ペアリングに使用するパスキー（PIN）を保持
        private string? Passkey = null;

        //
        // BLEペアリング関連イベント
        //
        public delegate void HandlerOnFIDOPeripheralPaired(bool success, string messageOnFail);
        public event HandlerOnFIDOPeripheralPaired OnFIDOPeripheralPaired = null!;

        public delegate void HandlerOnFIDOPeripheralFound();
        public event HandlerOnFIDOPeripheralFound OnFIDOPeripheralFound = null!;

        public async void Pair(string passkey)
        {
            // ペアリングに使用するパスキー（PIN）を保持
            Passkey = passkey;

            // Bluetoothがオンになっていることを確認
            bool bton = false;
            try {
                var radios = await Radio.GetRadiosAsync();
                foreach (var radio in radios) {
                    if (radio.Kind == RadioKind.Bluetooth) {
                        if (radio.State == RadioState.On) {
                            AppLogUtil.OutputLogInfo("Bluetoothはオンです。");
                            bton = true;
                            break;
                        }
                    }
                }
            } catch {
                // Bluetoothオン状態が確認できない場合はオフ状態であるとみなす
                AppLogUtil.OutputLogError("Bluetooth状態を確認できません。");
            }

            if (!bton) {
                // 画面スレッドに失敗を通知
                OnFIDOPeripheralPaired(false, AppCommon.MSG_BLE_PARING_ERR_BT_OFF);
                AppLogUtil.OutputLogError("Bluetoothはオフです。");
                return;
            }

            AppLogUtil.OutputLogInfo("FIDO認証器とのペアリングを開始します。");
            BluetoothAddress = 0;
            watcher.Start();

            // FIDO認証器がみつかるまで待機（最大10秒）
            AppLogUtil.OutputLogInfo("FIDO認証器からのアドバタイズ監視を開始します。");
            for (int i = 0; i < 10 && BluetoothAddress == 0; i++) {
                await Task.Run(() => System.Threading.Thread.Sleep(1000));
            }

            watcher.Stop();
            AppLogUtil.OutputLogInfo("FIDO認証器からのアドバタイズ監視を終了しました。");

            if (BluetoothAddress != 0) {
                // FIDO認証器が見つかった場合はペアリング実行
                OnFIDOPeripheralFound();
            } else {
                // 画面スレッドに失敗を通知
                OnFIDOPeripheralPaired(false, AppCommon.MSG_BLE_PARING_ERR_TIMED_OUT);
                AppLogUtil.OutputLogError("FIDO認証器とのペアリングがタイムアウトしました。");
            }
        }

        private void OnAdvertisementReceived(BluetoothLEAdvertisementWatcher watcher, BluetoothLEAdvertisementReceivedEventArgs eventArgs)
        {
            // FIDO認証器が見つかったら、
            // アドレス情報を保持し、画面スレッドに通知
            string name = eventArgs.Advertisement.LocalName;
            AppLogUtil.OutputLogInfo(string.Format("BLEデバイス[{0}]が見つかりました: BluetoothAddress={1}, {2} services",
                name, eventArgs.BluetoothAddress, eventArgs.Advertisement.ServiceUuids.Count));
            foreach (Guid g in eventArgs.Advertisement.ServiceUuids) {
                AppLogUtil.OutputLogDebug(string.Format("  service={0}", g.ToString()));
                if (g.Equals(U2F_BLE_SERVICE_UUID)) {
                    BluetoothAddress = eventArgs.BluetoothAddress;
                    AppLogUtil.OutputLogInfo("FIDO認証器が見つかりました.");
                    break;
                }
            }
        }

        private void CustomOnPairingRequested(DeviceInformationCustomPairing sender, DevicePairingRequestedEventArgs args)
        {
            if (args.PairingKind == DevicePairingKinds.ProvidePin && Passkey != null) {
                AppLogUtil.OutputLogInfo("指定のパスキーを使用し、FIDO認証器とのペアリングを実行します。");
                args.Accept(Passkey);

            } else {
                AppLogUtil.OutputLogInfo("FIDO認証器とのペアリングが自動的に実行されます。");
                args.Accept();
            }
        }

        public async void PairWithFIDOPeripheral()
        {
            bool success = false;
            string messageOnFail = "";
            try {
                // デバイス情報を取得
                BluetoothLEDevice? device = await BluetoothLEDevice.FromBluetoothAddressAsync(BluetoothAddress);
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
                    AppLogUtil.OutputLogInfo("FIDO認証器とのペアリングが成功しました。");
                } else if (result.Status == DevicePairingResultStatus.Failed) {
                    success = false;
                    messageOnFail = AppCommon.MSG_BLE_PARING_ERR_PAIR_MODE;
                    AppLogUtil.OutputLogError("FIDO認証器とのペアリングが失敗しました。");
                } else {
                    success = false;
                    messageOnFail = AppCommon.MSG_BLE_PARING_ERR_UNKNOWN;
                    AppLogUtil.OutputLogError(string.Format("FIDO認証器とのペアリングが失敗しました。reason={0}", result.Status));
                }

                // BLEデバイスを解放
                device.Dispose();
                device = null;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("BLEService.PairWithFIDOPeripheral: {0}", e.Message));
            }

            // 画面スレッドに成否を通知
            OnFIDOPeripheralPaired(success, messageOnFail);
        }

        //
        // BLE接続／送受信関連
        //
        // サービスをディスカバーできたデバイスを保持
        private readonly List<GattDeviceService> BLEServices = new List<GattDeviceService>();
        private GattDeviceService? BLEservice;
        private GattCharacteristic? U2FControlPointChar;
        private GattCharacteristic? U2FStatusChar;

        //
        // BLE送受信関連イベント
        //
        public delegate void HandlerOnDataReceived(byte[] receivedData);
        public event HandlerOnDataReceived OnDataReceived = null!;

        public delegate void HandlerOnTransactionFailed(string errorMessage);
        public event HandlerOnTransactionFailed OnTransactionFailed = null!;

        public async Task<bool> StartCommunicate()
        {
            try {
                if (BLEServices.Count == 0) {
                    // サービスをディスカバー
                    if (await DiscoverBLEService() == false) {
                        return false;
                    }
                }

                // データ受信監視を開始
                foreach (GattDeviceService service in BLEServices) {
                    if (await StartBLENotification(service)) {
                        AppLogUtil.OutputLogInfo(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_START, service.Device.Name));
                        break;
                    }
                    AppLogUtil.OutputLogError(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_FAILED, service.Device.Name));
                }

                // 接続された場合は true
                return IsConnected();

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("BLEService.StartCommunicate: {0}", e.Message));
                FreeResources();
                return false;
            }
        }

        private async Task<bool> DiscoverBLEService()
        {
            try {
                AppLogUtil.OutputLogInfo(string.Format("FIDO BLEサービス({0})を検索します。", U2F_BLE_SERVICE_UUID));
                string selector = GattDeviceService.GetDeviceSelectorFromUuid(U2F_BLE_SERVICE_UUID);
                DeviceInformationCollection collection = await DeviceInformation.FindAllAsync(selector);

                foreach (DeviceInformation info in collection) {
                    GattDeviceService service = await GattDeviceService.FromIdAsync(info.Id);
                    if (service != null) {
                        BLEServices.Add(service);
                        AppLogUtil.OutputLogDebug(string.Format("  FIDO BLE service found [{0}]", info.Name));
                    }
                }

                if (BLEServices.Count == 0) {
                    AppLogUtil.OutputLogError(AppCommon.MSG_BLE_U2F_SERVICE_NOT_FOUND);
                    return false;
                }

                AppLogUtil.OutputLogInfo(AppCommon.MSG_BLE_U2F_SERVICE_FOUND);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("BLEService.DiscoverBLEService: {0}", e.Message));
                FreeResources();
                return false;
            }
        }

        private async Task<bool> StartBLENotification(GattDeviceService service)
        {
            try {
                U2FStatusChar = service.GetCharacteristics(U2F_STATUS_CHAR_UUID)[0];

                GattCommunicationStatus result = await U2FStatusChar.WriteClientCharacteristicConfigurationDescriptorAsync(
                    GattClientCharacteristicConfigurationDescriptorValue.Notify);
                if (result != GattCommunicationStatus.Success) {
                    BLEservice = null;
                    return false;
                }

                U2FControlPointChar = service.GetCharacteristics(U2F_CONTROL_POINT_CHAR_UUID)[0];
                U2FStatusChar.ValueChanged += OnCharacteristicValueChanged;

                // 監視開始したサービスを退避
                BLEservice = service;
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("BLEService.StartBLENotification: {0}", e.Message));
                FreeResources();
                return false;
            }
        }

        private void OnCharacteristicValueChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            // 応答タイムアウト監視終了
            ResponseTimer.Stop();

            try {
                // レスポンスを受領（U2F Statusを読込）
                uint len = eventArgs.CharacteristicValue.Length;
                byte[] responseBytes = new byte[len];
                DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(responseBytes);

                // レスポンスを転送
                OnDataReceived(responseBytes);

            } catch (Exception e) {
                OnTransactionFailed(string.Format(AppCommon.MSG_REQUEST_SEND_FAILED_WITH_EXCEPTION, e.Message));
            }
        }

        public async void Send(byte[] requestData)
        {
            if (BLEservice == null) {
                AppLogUtil.OutputLogError(string.Format("BLEService.Send: service is null"));
                OnTransactionFailed(AppCommon.MSG_REQUEST_SEND_FAILED);
            }

            try {
                // リクエストデータを生成
                DataWriter writer = new DataWriter();
                for (int i = 0; i < requestData.Length; i++) {
                    writer.WriteByte(requestData[i]);
                }

                // リクエストを実行（U2F Control Pointに書込）
                if (U2FControlPointChar != null) {
                    GattCommunicationStatus result = await U2FControlPointChar.WriteValueAsync(writer.DetachBuffer(), GattWriteOption.WriteWithoutResponse);
                    if (result != GattCommunicationStatus.Success) {
                        OnTransactionFailed(AppCommon.MSG_REQUEST_SEND_FAILED);

                    } else {
                        // 応答タイムアウト監視開始
                        ResponseTimer.Start();
                    }

                } else {
                    AppLogUtil.OutputLogError(string.Format("BLEService.Send: U2F control point characteristic is null"));
                    OnTransactionFailed(AppCommon.MSG_REQUEST_SEND_FAILED);
                }

            } catch (Exception e) {
                OnTransactionFailed(string.Format(AppCommon.MSG_REQUEST_SEND_FAILED_WITH_EXCEPTION, e.Message));
            }
        }

        //
        // 応答タイムアウト時の処理
        //
        private void OnResponseTimerElapsed(object sender, EventArgs e)
        {
            // 応答タイムアウトを通知
            OnTransactionFailed(AppCommon.MSG_REQUEST_SEND_TIMED_OUT);
        }

        public void Disconnect()
        {
            // 切断
            StopCommunicate();
            AppLogUtil.OutputLogInfo(AppCommon.MSG_U2F_DEVICE_DISCONNECTED);
        }

        private void StopCommunicate()
        {
            try {
                if (U2FStatusChar != null) {
                    U2FStatusChar.ValueChanged -= OnCharacteristicValueChanged;
                }
                if (BLEservice != null) {
                    BLEservice.Dispose();
                }

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("BLEService.StopCommunicate: {0}", e.Message));

            } finally {
                FreeResources();
            }
        }

        public bool IsConnected()
        {
            if (BLEServices.Count == 0) {
                // 接続されていない場合は false
                return false;
            }

            if (BLEservice == null) {
                // データ受信ができない場合は false
                return false;
            }

            // BLE接続されている場合は true
            return true;
        }

        private void FreeResources()
        {
            // オブジェクトへの参照を解除
            BLEservice = null;
            U2FStatusChar = null;
        }
    }
}
