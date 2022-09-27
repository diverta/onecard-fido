namespace MaintenanceToolApp.DFU
{
    internal class USBDFUService
    {
        //
        // CDC ACM送受信関連イベント
        //
        public delegate void HandlerOnConnectedToDFUService(bool success);
        private event HandlerOnConnectedToDFUService OnConnectedToDFUService = null!;

        //
        // 接続処理
        //
        // コールバック関数を保持
        private HandlerOnConnectedToDFUService HandlerRef = null!;

        public void ConnectUSBDFUService(HandlerOnConnectedToDFUService handler)
        {
            // イベントを登録
            HandlerRef = handler;
            OnConnectedToDFUService += HandlerRef;

            // TODO: 仮の実装です。
            FuncOnConnectedToDFUService(true);
        }

        private void FuncOnConnectedToDFUService(bool success)
        {
            // FIDO認証器に接続完了
            OnConnectedToDFUService(success);
            OnConnectedToDFUService -= HandlerRef;
        }
    }
}
