using System;
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

        public BLEPairingService()
        {
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

            // TODO: 仮の実装です。
            FuncOnFIDOPeripheralFound(true, AppCommon.MSG_NONE, handler);
        }

        private void FuncOnFIDOPeripheralFound(bool found, string errorMessage, HandlerOnFIDOPeripheralFound handler)
        {
            OnFIDOPeripheralFound(found, errorMessage);
            OnFIDOPeripheralFound -= handler;
        }
    }
}
