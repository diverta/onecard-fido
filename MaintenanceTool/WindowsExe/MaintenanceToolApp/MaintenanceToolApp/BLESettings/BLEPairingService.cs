using System;
using System.Threading.Tasks;
using ToolAppCommon;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Radios;

namespace MaintenanceToolApp.BLESettings
{
    internal class BLEPairingService
    {
        //
        // BLEペアリング関連イベント
        //
        public delegate void HandlerOnFIDOPeripheralFound(bool found, string errorMessage);
        private event HandlerOnFIDOPeripheralFound OnFIDOPeripheralFound = null!;

        // 監視対象UUID
        private readonly Guid U2F_BLE_SERVICE_UUID = new Guid(BLEServiceConst.U2F_BLE_SERVICE_UUID_STR);

        private BluetoothLEAdvertisementWatcher watcher = null!;
        private ulong BluetoothAddress = 0;

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
                FuncOnFIDOPeripheralFound(false, AppCommon.MSG_BLE_PARING_ERR_BT_STATUS_CANNOT_GET, handler);
                return;
            }

            if (bton == false) {
                // Bluetoothがオンになっていない場合は失敗を通知
                FuncOnFIDOPeripheralFound(false, AppCommon.MSG_BLE_PARING_ERR_BT_OFF, handler);
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
                FuncOnFIDOPeripheralFound(false, AppCommon.MSG_BLE_PARING_ERR_TIMED_OUT, handler);
                return;
            }

            // FIDO認証器が見つかった場合は成功を通知
            FuncOnFIDOPeripheralFound(true, AppCommon.MSG_NONE, handler);
        }

        private void OnAdvertisementReceived(BluetoothLEAdvertisementWatcher watcher, BluetoothLEAdvertisementReceivedEventArgs eventArgs)
        {
            // FIDO認証器が見つかったら、
            // アドレス情報を保持し、画面スレッドに通知
            string name = eventArgs.Advertisement.LocalName;
            AppLogUtil.OutputLogDebug(string.Format("BLE device found [{0}]: BluetoothAddress={1}, {2} services",
                name, eventArgs.BluetoothAddress, eventArgs.Advertisement.ServiceUuids.Count));

            foreach (Guid g in eventArgs.Advertisement.ServiceUuids) {
                AppLogUtil.OutputLogDebug(string.Format("  service={0}", g.ToString()));

                if (g.Equals(U2F_BLE_SERVICE_UUID)) {
                    BluetoothAddress = eventArgs.BluetoothAddress;
                    AppLogUtil.OutputLogDebug("FIDO BLE device found.");
                    break;
                }
            }
        }

        private void FuncOnFIDOPeripheralFound(bool found, string errorMessage, HandlerOnFIDOPeripheralFound handler)
        {
            OnFIDOPeripheralFound(found, errorMessage);
            OnFIDOPeripheralFound -= handler;
        }
    }
}
