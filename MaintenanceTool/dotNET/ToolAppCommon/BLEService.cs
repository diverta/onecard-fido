using MaintenanceToolApp;
using MaintenanceToolApp.Common;
using MaintenanceToolApp.CommonProcess;
using System;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Storage.Streams;

namespace ToolAppCommon
{
    internal class BLEService
    {
        // 応答タイムアウト監視用タイマー
        private readonly CommonTimer ResponseTimer = null!;

        public BLEService()
        {
            // 応答タイムアウト発生時のイベントを登録
            ResponseTimer = new CommonTimer("BLEService", 10000);
            ResponseTimer.CommandTimeoutEvent += OnResponseTimerElapsed;
            FreeResources();
        }

        //
        // BLE接続／送受信関連
        //
        // サービスをディスカバーできたデバイスを保持
        private BluetoothLEDevice BluetoothLEDevice = null!;
        private GattDeviceService BLEservice = null!;
        private GattCharacteristic U2FControlPointChar = null!;
        private GattCharacteristic U2FStatusChar = null!;

        // ステータスを保持
        private GattCommunicationStatus CommunicationStatus;

        //
        // BLE送受信関連イベント
        //
        public delegate void HandlerOnDataReceived(byte[] receivedData);
        public event HandlerOnDataReceived OnDataReceived = null!;

        public delegate void HandlerOnTransactionFailed(string errorMessage);
        public event HandlerOnTransactionFailed OnTransactionFailed = null!;

        public async Task<bool> StartCommunicate(ScanBLEPeripheralParameter parameter)
        {
            // Bluetoothアドレスが不正の場合は処理を実行しない
            if (parameter.BluetoothAddress == 0) {
                FreeResources();
                return false;
            }

            // サービスをディスカバー
            if (await DiscoverBLEService(parameter) == false) {
                FreeResources();
                return false;
            }

            //
            // データ受信監視を開始
            // リトライ上限は３回とする
            //
            int retry = 3;
            for (int k = 0; k < retry + 1; k++) {
                if (k > 0) {
                    AppLogUtil.OutputLogWarn(string.Format(AppCommon.MSG_BLE_NOTIFICATION_RETRY, k));
                    await Task.Run(() => System.Threading.Thread.Sleep(100));
                }

                if (await StartBLENotification(BLEservice, parameter)) {
                    AppLogUtil.OutputLogInfo(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_START, BLEservice.Device.Name));
                    return true;
                }

                // 物理接続がない場合は再試行せず、明示的に接続オブジェクトを破棄
                if (CommunicationStatus == GattCommunicationStatus.Unreachable) {
                    StopCommunicate();
                    return false;
                }
            }

            // 接続されなかった場合は false
            AppLogUtil.OutputLogError(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_FAILED, BLEservice.Device.Name));
            return false;
        }

        private async Task<bool> DiscoverBLEService(ScanBLEPeripheralParameter parameter)
        {
            try {
                AppLogUtil.OutputLogInfo(string.Format(AppCommon.MSG_BLE_U2F_SERVICE_FINDING, parameter.ServiceUUID));
                BluetoothLEDevice = await BluetoothLEDevice.FromBluetoothAddressAsync(parameter.BluetoothAddress);
                if (BluetoothLEDevice == null) {
                    AppLogUtil.OutputLogError(AppCommon.MSG_BLE_U2F_DEVICE_NOT_FOUND);
                    return false;
                }

                var gattServices = await BluetoothLEDevice.GetGattServicesAsync();
                foreach (var gattService in gattServices.Services) {
                    if (gattService.Uuid.Equals(parameter.ServiceUUID)) {
                        BLEservice = gattService;
                        AppLogUtil.OutputLogDebug(string.Format("  FIDO BLE service found [{0}]", gattService.Device.Name));
                    }
                }

                if (BLEservice == null) {
                    AppLogUtil.OutputLogError(AppCommon.MSG_BLE_U2F_SERVICE_NOT_FOUND);
                    return false;
                }

                // 接続・切断検知ができるようにする
                BluetoothLEDevice.ConnectionStatusChanged += BluetoothLEDevice_ConnectionStatusChanged;

                AppLogUtil.OutputLogInfo(AppCommon.MSG_BLE_U2F_SERVICE_FOUND);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("BLEService.DiscoverBLEService: {0}", e.Message));
                return false;
            }
        }

        private async Task<bool> StartBLENotification(GattDeviceService service, ScanBLEPeripheralParameter parameter)
        {
            // ステータスを初期化（戻りの有無を上位関数で判別できるようにするため）
            CommunicationStatus = GattCommunicationStatus.Success;

            try {
                U2FStatusChar = service.GetCharacteristics(parameter.CharactForWriteUUID)[0];

                CommunicationStatus = await U2FStatusChar.WriteClientCharacteristicConfigurationDescriptorAsync(
                    GattClientCharacteristicConfigurationDescriptorValue.Notify);
                if (CommunicationStatus != GattCommunicationStatus.Success) {
                    AppLogUtil.OutputLogDebug(string.Format("BLEService.StartBLENotification: GattCommunicationStatus={0}", CommunicationStatus));
                    return false;
                }

                U2FControlPointChar = service.GetCharacteristics(parameter.CharactForReadUUID)[0];
                U2FStatusChar.ValueChanged += OnCharacteristicValueChanged;

                // 監視開始したサービスを退避
                BLEservice = service;
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("BLEService.StartBLENotification: {0}", e.Message));
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
                AppLogUtil.OutputLogDebug(string.Format("BLEService.Send: service is null"));
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
                    AppLogUtil.OutputLogDebug(string.Format("BLEService.Send: U2F control point characteristic is null"));
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

        //
        // 切断処理
        //
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
                if (BluetoothLEDevice != null) {
                    BluetoothLEDevice.ConnectionStatusChanged -= BluetoothLEDevice_ConnectionStatusChanged;
                    BluetoothLEDevice.Dispose();
                }

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("BLEService.StopCommunicate: {0}", e.Message));

            } finally {
                FreeResources();
            }
        }

        public bool IsConnected()
        {
            if (BluetoothLEDevice == null) {
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
            BluetoothLEDevice = null!;
            BLEservice = null!;
            U2FControlPointChar = null!;
            U2FStatusChar = null!;
        }

        public string ConnectedDeviceName()
        {
            if (BLEservice == null) {
                return string.Empty;
            } else {
                return BLEservice.Device.Name;
            }
        }

        //
        // BLE接続検知関連イベント
        //
        public delegate void HandlerOnConnectionStatusChanged(bool connected);
        public event HandlerOnConnectionStatusChanged OnConnectionStatusChanged = null!;

        private void BluetoothLEDevice_ConnectionStatusChanged(BluetoothLEDevice sender, object args)
        {
            OnConnectionStatusChanged(sender.ConnectionStatus == BluetoothConnectionStatus.Connected);
        }
    }
}
