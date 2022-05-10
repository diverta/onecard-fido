using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Storage.Streams;

namespace MaintenanceToolGUI
{
    public class ToolBLESMPService
    {
        // UUID
        private Guid BLE_SMP_SERVICE_UUID = new Guid("8D53DC1D-1DB7-4CD3-868B-8A527460AA84");
        private Guid BLE_SMP_CHARACT_UUID = new Guid("DA2E7828-FBCE-4E01-AE9E-261174997C48");

        // BLE SMPサービス／キャラクタリスティック
        private GattDeviceService BLESMPService;
        private GattCharacteristic BLESMPCharacteristic;

        // サービスをディスカバーできたデバイスを保持
        private List<GattDeviceService> BLEServices = new List<GattDeviceService>();

        // BLE SMPサービスのイベントを定義
        public delegate void ConnectedEvent();
        public event ConnectedEvent OnConnected;

        public delegate void ConnectionFailedEvent();
        public event ConnectionFailedEvent OnConnectionFailed;

        public delegate void DataReceivedEvent(byte[] receivedData);
        public event DataReceivedEvent OnDataReceived;

        public delegate void TransactionFailedEvent();
        public event TransactionFailedEvent OnTransactionFailed;

        public ToolBLESMPService()
        {
            FreeResources();
        }

        public async void ConnectBLESMPService()
        {
            if (IsConnected() == false) {
                // 未接続の場合はFIDO認証器とのBLE通信を開始
                if (await StartCommunicate() == false) {
                    OnConnectionFailed();
                    return;
                }
                AppCommon.OutputLogInfo("BLE SMPサービスに接続されました。");
            }

            // FIDO認証器に接続完了
            OnConnected();
        }

        public void DisconnectBLESMPService()
        {
            // 切断
            StopCommunicate();
            AppCommon.OutputLogInfo("BLE SMPサービスから切断されました。");
        }

        //
        // BLE SMPサービスに対する操作
        // 
        public async Task<bool> StartCommunicate()
        {
            try {
                // サービスをディスカバー
                if (await DiscoverBLEService() == false) {
                    return false;
                }

                // データ受信監視を開始
                foreach (GattDeviceService service in BLEServices) {
                    if (await StartBLENotification(service)) {
                        AppCommon.OutputLogInfo(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_START, service.Device.Name));
                        break;
                    }
                    AppCommon.OutputLogError(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_FAILED, service.Device.Name));
                }

                // 接続された場合は true
                return IsConnected();

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("StartCommunicate: {0}", e.Message));
                FreeResources();
                return false;
            }
        }

        public async Task<bool> DiscoverBLEService()
        {
            try {
                AppCommon.OutputLogInfo(string.Format("BLE SMPサービス({0})を検索します。", BLE_SMP_SERVICE_UUID));
                string selector = GattDeviceService.GetDeviceSelectorFromUuid(BLE_SMP_SERVICE_UUID);
                DeviceInformationCollection collection = await DeviceInformation.FindAllAsync(selector);

                BLEServices.Clear();
                foreach (DeviceInformation info in collection) {
                    GattDeviceService service = await GattDeviceService.FromIdAsync(info.Id);
                    if (service != null) {
                        BLEServices.Add(service);
                        AppCommon.OutputLogDebug(string.Format("  BLE SMP service found [{0}]", info.Name));
                    }
                }

                if (BLEServices.Count == 0) {
                    AppCommon.OutputLogError("BLE SMPサービスが見つかりません。");
                    return false;
                }

                AppCommon.OutputLogInfo("BLE SMPサービスが見つかりました。");
                return true;

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("DiscoverBLEService: {0}", e.Message));
                FreeResources();
                return false;
            }
        }

        public async Task<bool> StartBLENotification(GattDeviceService service)
        {
            try {
                BLESMPCharacteristic = service.GetCharacteristics(BLE_SMP_CHARACT_UUID)[0];

                GattCommunicationStatus result = await BLESMPCharacteristic.WriteClientCharacteristicConfigurationDescriptorAsync(
                    GattClientCharacteristicConfigurationDescriptorValue.Notify);
                if (result != GattCommunicationStatus.Success) {
                    BLESMPService = null;
                    return false;
                }

                BLESMPCharacteristic.ValueChanged += OnCharacteristicValueChanged;

                // 監視開始したサービスを退避
                BLESMPService = service;
                return true;

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("StartBLENotification: {0}", e.Message));
                FreeResources();
                return false;
            }
        }

        public async void Send(byte[] requestData)
        {
            if (BLESMPService == null) {
                AppCommon.OutputLogError(string.Format("BLESMPService.Send: service is null"));
                OnTransactionFailed();
            }

            try {
                // リクエストデータを生成
                DataWriter writer = new DataWriter();
                for (int i = 0; i < requestData.Length; i++) {
                    writer.WriteByte(requestData[i]);
                }

                // 書込みオプションを設定
                GattWriteOption writeOption = GattWriteOption.WriteWithoutResponse;
                if ((BLESMPCharacteristic.CharacteristicProperties & GattCharacteristicProperties.WriteWithoutResponse) == 0) {
                    writeOption = GattWriteOption.WriteWithResponse;
                }

                // リクエストを実行（SMPキャラクタリスティックに書込）
                GattCommunicationStatus result = await BLESMPCharacteristic.WriteValueAsync(writer.DetachBuffer(), writeOption);
                if (result != GattCommunicationStatus.Success) {
                    AppCommon.OutputLogError(AppCommon.MSG_REQUEST_SEND_FAILED);
                    OnTransactionFailed();
                }

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("BLESMPService.Send: {0}", e.Message));
                OnTransactionFailed();
            }
        }

        private void OnCharacteristicValueChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            try {
                // レスポンスを受領（SMPキャラクタリスティックから読込）
                uint len = eventArgs.CharacteristicValue.Length;
                byte[] responseBytes = new byte[len];
                DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(responseBytes);

                // レスポンスを転送
                OnDataReceived(responseBytes);

            } catch (Exception e) {
                // エラー通知
                AppCommon.OutputLogError(string.Format("OnCharacteristicValueChanged: {0}", e.Message));
                OnTransactionFailed();
            }
        }

        public bool IsConnected()
        {
            // 接続されていない場合は false
            if (BLEServices.Count == 0) {
                return false;
            }

            // データ受信ができない場合は false
            if (BLESMPService == null) {
                return false;
            }

            // BLE接続されている場合は true
            return true;
        }

        private void StopCommunicate()
        {
            try {
                if (BLESMPCharacteristic != null) {
                    BLESMPCharacteristic.ValueChanged -= OnCharacteristicValueChanged;
                }
                if (BLESMPService != null) {
                    BLESMPService.Dispose();
                }

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("StopCommunicate: {0}", e.Message));

            } finally {
                FreeResources();
            }
        }

        private void FreeResources()
        {
            // オブジェクトへの参照を解除
            BLESMPService = null;
            BLESMPCharacteristic = null;
        }
    }
}
