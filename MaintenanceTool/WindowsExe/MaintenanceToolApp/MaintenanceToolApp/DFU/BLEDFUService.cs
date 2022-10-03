using MaintenanceToolApp.Common;
using System;
using System.Threading.Tasks;
using ToolAppCommon;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Storage.Streams;

namespace MaintenanceToolApp.DFU
{
    public class BLEDFUServiceConst
    {
        public const string BLE_SMP_SERVICE_UUID_STR = "8D53DC1D-1DB7-4CD3-868B-8A527460AA84";
        public const string BLE_SMP_CHARACT_UUID_STR = "DA2E7828-FBCE-4E01-AE9E-261174997C48";
    }

    internal class BLEDFUService
    {
        // UUID
        private readonly Guid BLE_SMP_SERVICE_UUID = new Guid(BLEDFUServiceConst.BLE_SMP_SERVICE_UUID_STR);
        private readonly Guid BLE_SMP_CHARACT_UUID = new Guid(BLEDFUServiceConst.BLE_SMP_CHARACT_UUID_STR);

        // 応答タイムアウト監視用タイマー
        private CommonTimer ResponseTimer = null!;

        public BLEDFUService()
        {
            // 応答タイムアウト発生時のイベントを登録
            ResponseTimer = new CommonTimer("BLEDFUService", 10000);
            ResponseTimer.CommandTimeoutEvent += OnResponseTimerElapsed;
            FreeResources();
        }

        //
        // BLE接続／送受信関連
        //
        // サービスをディスカバーできたデバイスを保持
        private BluetoothLEDevice BluetoothLEDevice = null!;
        private GattDeviceService SMPService = null!;
        private GattCharacteristic SMPCharacteristic = null!;

        // ステータスを保持
        private GattCommunicationStatus CommunicationStatus;

        //
        // BLE送受信関連イベント
        //
        public delegate void HandlerOnConnectedToSMPService(bool success);
        public event HandlerOnConnectedToSMPService OnConnectedToSMPService = null!;

        public delegate void HandlerOnDataReceived(byte[] receivedData);
        public event HandlerOnDataReceived OnDataReceived = null!;

        public delegate void HandlerOnTransactionFailed(string errorMessage);
        public event HandlerOnTransactionFailed OnTransactionFailed = null!;

        public async void ConnectBLEDFUService(HandlerOnConnectedToSMPService handler)
        {
            // イベントを登録
            OnConnectedToSMPService += handler;

            if (IsConnected() == false) {
                // 未接続の場合はFIDO認証器とのBLE通信を開始
                if (await StartCommunicate() == false) {
                    OnConnectedToSMPService(false);
                    OnConnectedToSMPService -= handler;
                    return;
                }
                AppLogUtil.OutputLogInfo(AppCommon.MSG_BLE_SMP_SERVICE_CONNECTED);
            }

            // FIDO認証器に接続完了
            OnConnectedToSMPService(true);
            OnConnectedToSMPService -= handler;
        }

        //
        // BLE SMPサービスに対する操作
        // 
        public async Task<bool> StartCommunicate()
        {
            // サービスをディスカバー
            if (await DiscoverBLEService() == false) {
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

                if (await StartBLENotification(SMPService)) {
                    AppLogUtil.OutputLogInfo(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_START, SMPService.Device.Name));
                    return true;
                }

                // 物理接続がない場合は再試行せず、明示的に接続オブジェクトを破棄
                if (CommunicationStatus == GattCommunicationStatus.Unreachable) {
                    StopCommunicate();
                    return false;
                }
            }

            // 接続されなかった場合は false
            AppLogUtil.OutputLogError(string.Format("{0}({1})", AppCommon.MSG_BLE_SMP_NOTIFICATION_FAILED, SMPService.Device.Name));
            return false;
        }

        private async Task<bool> DiscoverBLEService()
        {
            try {
                AppLogUtil.OutputLogInfo(string.Format(AppCommon.MSG_BLE_SMP_SERVICE_FINDING, BLE_SMP_SERVICE_UUID));
                string selector = GattDeviceService.GetDeviceSelectorFromUuid(BLE_SMP_SERVICE_UUID);
                DeviceInformationCollection collection = await DeviceInformation.FindAllAsync(selector);

                foreach (DeviceInformation info in collection) {
                    BluetoothLEDevice = await BluetoothLEDevice.FromIdAsync(info.Id);
                    var gattServices = await BluetoothLEDevice.GetGattServicesAsync();
                    foreach (var gattService in gattServices.Services) {
                        if (gattService.Uuid.Equals(BLE_SMP_SERVICE_UUID)) {
                            SMPService = gattService;
                            AppLogUtil.OutputLogDebug(string.Format("  BLE SMP service found [{0}]", info.Name));
                        }
                    }
                }

                if (BluetoothLEDevice == null || SMPService == null) {
                    AppLogUtil.OutputLogError(AppCommon.MSG_BLE_SMP_SERVICE_NOT_FOUND);
                    return false;
                }

                AppLogUtil.OutputLogInfo(AppCommon.MSG_BLE_SMP_SERVICE_FOUND);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("BLEDFUService.DiscoverBLEService: {0}", e.Message));
                return false;
            }
        }

        private async Task<bool> StartBLENotification(GattDeviceService service)
        {
            // ステータスを初期化（戻りの有無を上位関数で判別できるようにするため）
            CommunicationStatus = GattCommunicationStatus.Success;

            try {
                SMPCharacteristic = service.GetCharacteristics(BLE_SMP_CHARACT_UUID)[0];

                GattCommunicationStatus result = await SMPCharacteristic.WriteClientCharacteristicConfigurationDescriptorAsync(
                    GattClientCharacteristicConfigurationDescriptorValue.Notify);
                if (CommunicationStatus != GattCommunicationStatus.Success) {
                    AppLogUtil.OutputLogDebug(string.Format("BLEDFUService.StartBLENotification: GattCommunicationStatus={0}", CommunicationStatus));
                    return false;
                }

                SMPCharacteristic.ValueChanged += OnCharacteristicValueChanged;

                // 監視開始したサービスを退避
                SMPService = service;
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("BLEDFUService.StartBLENotification: {0}", e.Message));
                return false;
            }
        }

        private void OnCharacteristicValueChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            // 応答タイムアウト監視終了
            ResponseTimer.Stop();

            try {
                // レスポンスを受領（SMPキャラクタリスティックから読込）
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
            if (SMPService == null) {
                AppLogUtil.OutputLogError(string.Format("BLEDFUService.Send: service is null"));
                OnTransactionFailed(AppCommon.MSG_REQUEST_SEND_FAILED);
            }

            try {
                // リクエストデータを生成
                DataWriter writer = new DataWriter();
                for (int i = 0; i < requestData.Length; i++) {
                    writer.WriteByte(requestData[i]);
                }

                // 書込みオプションを設定
                GattWriteOption writeOption = GattWriteOption.WriteWithoutResponse;
                if ((SMPCharacteristic.CharacteristicProperties & GattCharacteristicProperties.WriteWithoutResponse) == 0) {
                    writeOption = GattWriteOption.WriteWithResponse;
                }

                // リクエストを実行（SMPキャラクタリスティックに書込）
                GattCommunicationStatus result = await SMPCharacteristic.WriteValueAsync(writer.DetachBuffer(), writeOption);
                if (result != GattCommunicationStatus.Success) {
                    OnTransactionFailed(AppCommon.MSG_REQUEST_SEND_FAILED);

                } else {
                    // 応答タイムアウト監視開始
                    ResponseTimer.Start();
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
        public void DisconnectBLEDFUService()
        {
            // 切断
            StopCommunicate();
            AppLogUtil.OutputLogInfo(AppCommon.MSG_BLE_SMP_SERVICE_DISCONNECTED);
        }

        private void StopCommunicate()
        {
            try {
                if (SMPCharacteristic != null) {
                    SMPCharacteristic.ValueChanged -= OnCharacteristicValueChanged;
                }
                if (SMPService != null) {
                    SMPService.Dispose();
                }
                if (BluetoothLEDevice != null) {
                    BluetoothLEDevice.Dispose();
                }

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("StopCommunicate: {0}", e.Message));

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

            if (SMPService == null) {
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
            SMPService = null!;
            SMPCharacteristic = null!;
        }
    }
}
