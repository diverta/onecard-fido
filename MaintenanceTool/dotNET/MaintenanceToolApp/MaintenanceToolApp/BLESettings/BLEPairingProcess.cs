namespace MaintenanceToolApp.BLESettings
{
    internal class BLEPairingProcess
    {
        // BLEペアリングサービスの参照を保持（インスタンス生成は１度だけ行われる）
        private static readonly BLEPairingService PairingService = new BLEPairingService();

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
        public static void DoFindFIDOPeripheral(BLEPairingService.HandlerOnFIDOPeripheralFound handler)
        {
            // ペアリング対象のFIDO認証器を検索
            PairingService.FindFIDOPeripheral(handler);
        }

        public void DoRequestPairing(HandlerOnNotifyCommandTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyCommandTerminated = handler;

            // FIDO認証器とペアリングを実行
            PairingService.PairWithFIDOPeripheral(Parameter.BluetoothAddress, Parameter.Passcode, OnFIDOPeripheralPaired);
        }

        private void OnFIDOPeripheralPaired(bool success, string errorMessage)
        {
            // 上位クラスに制御を戻す
            NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, success);
        }
    }
}
