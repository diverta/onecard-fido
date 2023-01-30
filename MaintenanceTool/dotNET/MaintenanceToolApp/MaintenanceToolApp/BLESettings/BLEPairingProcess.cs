using System;
using ToolAppCommon;
using Windows.Devices.Bluetooth;
using Windows.Devices.Enumeration;

namespace MaintenanceToolApp.BLESettings
{
    internal class BLEPairingProcess
    {
        // 処理実行のためのプロパティー
        private readonly BLESettingsParameter Parameter;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(string commandTitle, string errorMessage, bool success);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;
        
        public BLEPairingProcess(BLESettingsParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
        }

        //
        // 外部公開用
        //
        public void DoRequestPairing(HandlerOnNotifyCommandTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyCommandTerminated += handler;

            // FIDO認証器とペアリングを実行
            PairWithFIDOPeripheral();
        }

        //
        // ペアリング実行
        //
        public async void PairWithFIDOPeripheral()
        {
            // 変数を初期化
            bool success = false;
            string errorMessage = "";

            try {
                // デバイス情報を取得
                BluetoothLEDevice? device = await BluetoothLEDevice.FromBluetoothAddressAsync(Parameter.BluetoothAddress);
                DeviceInformation deviceInfoForPair = device.DeviceInformation;

                // ペアリング実行
                deviceInfoForPair.Pairing.Custom.PairingRequested += CustomOnPairingRequested;
                DevicePairingResult result;
                if (Parameter.Passcode == null || Parameter.Passcode.Length == 0) {
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
                if (result.Status == DevicePairingResultStatus.Paired) {
                    success = true;
                    AppLogUtil.OutputLogDebug("Pairing with FIDO device success");

                } else if (result.Status == DevicePairingResultStatus.AlreadyPaired) {
                    errorMessage = string.Format(AppCommon.MSG_BLE_PARING_ERR_ALREADY_PAIRED, deviceInfoForPair.Name);
                    AppLogUtil.OutputLogError("Already paired with FIDO device");

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

            // 上位クラスに制御を戻す
            NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, success);
        }

        private void CustomOnPairingRequested(DeviceInformationCustomPairing sender, DevicePairingRequestedEventArgs args)
        {
            if (args.PairingKind == DevicePairingKinds.ProvidePin && Parameter.Passcode != null) {
                args.Accept(Parameter.Passcode);
            } else {
                args.Accept();
            }
        }
    }
}
