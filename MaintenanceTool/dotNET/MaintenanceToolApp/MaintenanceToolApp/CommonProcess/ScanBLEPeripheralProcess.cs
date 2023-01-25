using System;
using System.Linq;
using System.Threading.Tasks;
using ToolAppCommon;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Radios;
using Windows.Storage.Streams;
using static MaintenanceToolApp.FIDODefine;

namespace MaintenanceToolApp.CommonProcess
{
    internal class ScanBLEPeripheralParameter
    {
        public Guid ServiceUUID { get; set; }
        public Guid CharactForWriteUUID { get; set; }
        public Guid CharactForReadUUID { get; set; }
        public ulong BluetoothAddress { get; set; }
        public byte[] ServiceDataField { get; set; }
        public bool FIDOServiceDataFieldFound { get; set; }
        public bool BLEPeripheralFound { get; set; }
        public bool ConnectOnly { get; set; }

        public ScanBLEPeripheralParameter(string serviceUUIDString, string charactForWriteUUIDString, string charactForReadUUIDString)
        {
            ServiceUUID = new Guid(serviceUUIDString);
            CharactForWriteUUID = new Guid(charactForWriteUUIDString);
            CharactForReadUUID = new Guid(charactForReadUUIDString);
            BluetoothAddress = 0;
            ServiceDataField = Array.Empty<byte>();
            FIDOServiceDataFieldFound = false;
            BLEPeripheralFound = false;
            ConnectOnly = false;
        }

        public static ScanBLEPeripheralParameter PrepareParameterForFIDO()
        {
            return new ScanBLEPeripheralParameter(U2F_BLE_SERVICE_UUID_STR, U2F_STATUS_CHAR_UUID_STR, U2F_CONTROL_POINT_CHAR_UUID_STR);
        }
    }

    internal class ScanBLEPeripheralProcess
    {
        // 上位クラスに対するイベント通知
        public delegate void HandlerOnBLEPeripheralFound(bool success, string errorMessage, ScanBLEPeripheralParameter parameter);
        private event HandlerOnBLEPeripheralFound BLEPeripheralFound = null!;

        // 戻り先の関数を保持
        private HandlerOnBLEPeripheralFound HandlerRef = null!;

        // スキャン処理のパラメーターを保持
        private ScanBLEPeripheralParameter Parameter = null!;

        // スキャンに使用するWatcherを保持
        private readonly BluetoothLEAdvertisementWatcher Watcher = null!;

        // コンストラクター
        public ScanBLEPeripheralProcess()
        {
            // Watcherを初期化
            Watcher = new BluetoothLEAdvertisementWatcher();
            Watcher.Received += OnAdvertisementReceived;
        }

        public void DoProcess(ScanBLEPeripheralParameter parameter, HandlerOnBLEPeripheralFound handler)
        {
            // パラメーターを保持
            Parameter = parameter;

            // 戻り先の関数を保持
            HandlerRef = handler;
            BLEPeripheralFound += HandlerRef;

            // FIDO認証器を検索
            FindFIDOPeripheral();
        }

        private void OnProcessTerminated(bool success, string errorMessage)
        {
            // スキャン結果情報、エラーメッセージを戻す
            BLEPeripheralFound(success, errorMessage, Parameter);

            // 呼出元クラスの関数コールバックを解除
            BLEPeripheralFound -= HandlerRef;
        }

        //
        // 内部処理
        //
        private async void FindFIDOPeripheral()
        {
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
                OnProcessTerminated(false, AppCommon.MSG_BLE_PARING_ERR_BT_STATUS_CANNOT_GET);
                return;
            }

            if (bton == false) {
                // Bluetoothがオンになっていない場合は失敗を通知
                OnProcessTerminated(false, AppCommon.MSG_BLE_PARING_ERR_BT_OFF);
                return;
            }

            // FIDO認証器からのアドバタイズ監視を開始
            WatchAdvertisement();
        }

        private async void WatchAdvertisement()
        {
            // FIDO認証器からのアドバタイズ監視を開始
            AppLogUtil.OutputLogDebug("Watch FIDO BLE device advertisement start");
            Parameter.BluetoothAddress = 0;
            Watcher.Start();

            // FIDO認証器がみつかるまで待機（最大10秒）
            for (int i = 0; i < 10 && Parameter.BluetoothAddress == 0; i++) {
                await Task.Run(() => System.Threading.Thread.Sleep(1000));
            }

            // FIDO認証器からのアドバタイズ監視を終了
            Watcher.Stop();
            AppLogUtil.OutputLogDebug("Watch FIDO BLE device advertisement end");

            if (Parameter.BluetoothAddress == 0) {
                // FIDO認証器が見つからなかった場合は失敗を通知
                OnProcessTerminated(false, AppCommon.MSG_BLE_PARING_ERR_TIMED_OUT);
                return;
            }

            // FIDOのサービスデータフィールドが存在する場合はフラグを設定
            byte[] expect = { 0xfd, 0xff, 0x80 };
            if (Parameter.ServiceDataField.Length == 3 && Parameter.ServiceDataField.SequenceEqual(expect)) {
                Parameter.FIDOServiceDataFieldFound = true;
            }

            // FIDO認証器が見つかった場合は成功を通知
            Parameter.BLEPeripheralFound = true;
            OnProcessTerminated(true, AppCommon.MSG_NONE);
        }

        private void OnAdvertisementReceived(BluetoothLEAdvertisementWatcher watcher, BluetoothLEAdvertisementReceivedEventArgs eventArgs)
        {
            // FIDO認証器が見つかったら、
            // アドレス情報を保持し、画面スレッドに通知
            foreach (Guid g in eventArgs.Advertisement.ServiceUuids) {
                if (g.Equals(Parameter.ServiceUUID)) {
                    Parameter.BluetoothAddress = eventArgs.BluetoothAddress;
                    AppLogUtil.OutputLogDebug("FIDO BLE device found.");
                    // アドバタイズデータからサービスデータフィールドを抽出
                    Parameter.ServiceDataField = RetrieveServiceDataField(eventArgs.Advertisement);
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
    }
}
