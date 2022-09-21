using MaintenanceToolApp;
using MaintenanceToolApp.Common;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Storage.Streams;

namespace ToolAppCommon
{
    public class BLEServiceConst
    {
        public const string U2F_BLE_SERVICE_UUID_STR = "0000FFFD-0000-1000-8000-00805f9b34fb";
        public const string U2F_CONTROL_POINT_CHAR_UUID_STR = "F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB";
        public const string U2F_STATUS_CHAR_UUID_STR = "F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB";
    }

    public class BLEService
    {
        private readonly Guid U2F_BLE_SERVICE_UUID = new Guid(BLEServiceConst.U2F_BLE_SERVICE_UUID_STR);
        private readonly Guid U2F_CONTROL_POINT_CHAR_UUID = new Guid(BLEServiceConst.U2F_CONTROL_POINT_CHAR_UUID_STR);
        private readonly Guid U2F_STATUS_CHAR_UUID = new Guid(BLEServiceConst.U2F_STATUS_CHAR_UUID_STR);

        // 応答タイムアウト監視用タイマー
        private CommonTimer ResponseTimer = null!;

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
        private readonly List<GattDeviceService> BLEServices = new List<GattDeviceService>();
        private GattDeviceService BLEservice = null!;
        private GattCharacteristic U2FControlPointChar = null!;
        private GattCharacteristic U2FStatusChar = null!;

        //
        // BLE送受信関連イベント
        //
        public delegate void HandlerOnDataReceived(byte[] receivedData);
        public event HandlerOnDataReceived OnDataReceived = null!;

        public delegate void HandlerOnTransactionFailed(string errorMessage);
        public event HandlerOnTransactionFailed OnTransactionFailed = null!;

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

                for (int k = 0; k < 2; k++) {
                    if (k > 0) {
                        AppLogUtil.OutputLogWarn(string.Format(AppCommon.MSG_BLE_NOTIFICATION_RETRY, k));
                        await Task.Run(() => System.Threading.Thread.Sleep(100));
                    }

                    if (await StartBLENotification(service)) {
                        AppLogUtil.OutputLogInfo(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_START, service.Device.Name));
                        return true;
                    }
                }

                AppLogUtil.OutputLogError(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_FAILED, service.Device.Name));
            }

            // 接続されなかった場合は false
            return false;
        }

        private async Task<bool> DiscoverBLEService()
        {
            try {
                AppLogUtil.OutputLogInfo(string.Format(AppCommon.MSG_BLE_U2F_SERVICE_FINDING, U2F_BLE_SERVICE_UUID));
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
                    FreeResources();
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
                AppLogUtil.OutputLogDebug("IsConnected: BLEServices.Count == 0");
                return false;
            }

            if (BLEservice == null) {
                // データ受信ができない場合は false
                AppLogUtil.OutputLogDebug("IsConnected: BLEservice == null");
                return false;
            }

            // BLE接続されている場合は true
            return true;
        }

        private void FreeResources()
        {
            // オブジェクトへの参照を解除
            BLEservice = null!;
            U2FControlPointChar = null!;
            U2FStatusChar = null!;
            AppLogUtil.OutputLogDebug("FreeResources done.");
        }
    }
}
