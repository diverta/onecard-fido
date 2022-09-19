using MaintenanceToolApp.Common;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using ToolAppCommon;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Storage.Streams;

namespace MaintenanceToolApp.DFU
{
    public class BLESMPServiceConst
    {
        public const string BLE_SMP_SERVICE_UUID_STR = "8D53DC1D-1DB7-4CD3-868B-8A527460AA84";
        public const string BLE_SMP_CHARACT_UUID_STR = "DA2E7828-FBCE-4E01-AE9E-261174997C48";
    }

    internal class BLESMPService
    {
        // UUID
        private readonly Guid BLE_SMP_SERVICE_UUID = new Guid(BLESMPServiceConst.BLE_SMP_SERVICE_UUID_STR);
        private readonly Guid BLE_SMP_CHARACT_UUID = new Guid(BLESMPServiceConst.BLE_SMP_CHARACT_UUID_STR);

        // BLE SMPサービス／キャラクタリスティック
        private GattDeviceService SMPService = null!;
        private GattCharacteristic SMPCharacteristic = null!;

        // サービスをディスカバーできたデバイスを保持
        private readonly List<GattDeviceService> BLEServices = new List<GattDeviceService>();

        // BLE SMPサービスのイベントを定義
        public delegate void HandlerOnConnectedToSMPService(bool success);
        public event HandlerOnConnectedToSMPService OnConnectedToSMPService = null!;

        public delegate void HandlerOnDataReceived(byte[] receivedData);
        public event HandlerOnDataReceived OnDataReceived = null!;

        public delegate void HandlerOnTransactionFailed(string errorMessage);
        public event HandlerOnTransactionFailed OnTransactionFailed = null!;

        // 応答タイムアウト監視用タイマー
        private CommonTimer ResponseTimer = null!;

        public BLESMPService()
        {
            // 応答タイムアウト発生時のイベントを登録
            ResponseTimer = new CommonTimer("BLESMPService", 10000);
            ResponseTimer.CommandTimeoutEvent += OnResponseTimerElapsed;

            FreeResources();
        }

        public async void ConnectBLESMPService(HandlerOnConnectedToSMPService handler)
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
                AppLogUtil.OutputLogInfo("BLE SMPサービスに接続されました。");
            }

            // FIDO認証器に接続完了
            OnConnectedToSMPService(true);
            OnConnectedToSMPService -= handler;
        }

        public void DisconnectBLESMPService()
        {
            // 切断
            StopCommunicate();
            AppLogUtil.OutputLogInfo("BLE SMPサービスから切断されました。");
        }

        //
        // BLE SMPサービスに対する操作
        // 
        public async Task<bool> StartCommunicate()
        {
            // サービスをディスカバー
            BLEServices.Clear();
            if (await DiscoverBLEService() == false) {
                return false;
            }

            // データ受信監視を開始
            for (int i = 0; i < BLEServices.Count; i++) {
                GattDeviceService service = BLEServices[i];
                if (await StartBLENotification(service)) {
                    AppLogUtil.OutputLogInfo(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_START, service.Device.Name));
                    break;
                }
                AppLogUtil.OutputLogError(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_FAILED, service.Device.Name));
            }

            // 接続された場合は true
            return IsConnected();
        }

        private async Task<bool> DiscoverBLEService()
        {
            try {
                AppLogUtil.OutputLogInfo(string.Format(AppCommon.MSG_BLE_SMP_SERVICE_FINDING, BLE_SMP_SERVICE_UUID));
                string selector = GattDeviceService.GetDeviceSelectorFromUuid(BLE_SMP_SERVICE_UUID);
                DeviceInformationCollection collection = await DeviceInformation.FindAllAsync(selector);

                foreach (DeviceInformation info in collection) {
                    GattDeviceService service = await GattDeviceService.FromIdAsync(info.Id);
                    if (service != null) {
                        BLEServices.Add(service);
                        AppLogUtil.OutputLogDebug(string.Format("  BLE SMP service found [{0}]", info.Name));
                    }
                }

                if (BLEServices.Count == 0) {
                    AppLogUtil.OutputLogError(AppCommon.MSG_BLE_SMP_SERVICE_NOT_FOUND);
                    return false;
                }

                AppLogUtil.OutputLogInfo(AppCommon.MSG_BLE_SMP_SERVICE_FOUND);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("DiscoverBLEService: {0}", e.Message));
                FreeResources();
                return false;
            }
        }

        public async Task<bool> StartBLENotification(GattDeviceService service)
        {
            try {
                SMPCharacteristic = service.GetCharacteristics(BLE_SMP_CHARACT_UUID)[0];

                GattCommunicationStatus result = await SMPCharacteristic.WriteClientCharacteristicConfigurationDescriptorAsync(
                    GattClientCharacteristicConfigurationDescriptorValue.Notify);
                if (result != GattCommunicationStatus.Success) {
                    SMPService = null!;
                    return false;
                }

                SMPCharacteristic.ValueChanged += OnCharacteristicValueChanged;

                // 監視開始したサービスを退避
                SMPService = service;
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("StartBLENotification: {0}", e.Message));
                FreeResources();
                return false;
            }
        }

        public async void Send(byte[] requestData)
        {
            if (SMPService == null) {
                AppLogUtil.OutputLogError(string.Format("BLESMPService.Send: service is null"));
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

        public bool IsConnected()
        {
            // 接続されていない場合は false
            if (BLEServices.Count == 0) {
                return false;
            }

            // データ受信ができない場合は false
            if (SMPService == null) {
                return false;
            }

            // BLE接続されている場合は true
            return true;
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

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("StopCommunicate: {0}", e.Message));

            } finally {
                FreeResources();
            }
        }

        private void FreeResources()
        {
            // オブジェクトへの参照を解除
            SMPService = null!;
            SMPCharacteristic = null!;
        }
    }
}
