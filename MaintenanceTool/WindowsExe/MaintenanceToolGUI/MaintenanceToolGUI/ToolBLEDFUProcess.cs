namespace MaintenanceToolGUI
{
    public class ToolBLEDFUProcess
    {
        // クラスの参照を保持
        private ToolBLEDFU ToolBLEDFURef;
        private ToolBLEDFUImage ToolBLEDFUImageRef;
        private ToolBLESMPService ToolBLESMPService;

        public ToolBLEDFUProcess(ToolBLEDFUImage imageRef)
        {
            ToolBLEDFUImageRef = imageRef;
            ToolBLESMPService = new ToolBLESMPService(this);
        }

        public void PerformDFU(ToolBLEDFU dfuRef)
        {
            ToolBLEDFURef = dfuRef;

            // BLE SMPサービスに接続
            DoConnect();
        }

        private void TerminateDFUProcess(bool success)
        {
            // メイン画面にDFU処理の終了を通知
            ToolBLEDFURef.DFUProcessTerminated(success);
        }

        //
        // 接続／切断処理
        // 
        private void DoConnect()
        {
            // BLE SMPサービスに接続
            ToolBLESMPService.ConnectBLESMPService();
        }

        public void OnConnectionFailed()
        {
            // 画面に異常終了を通知
            TerminateDFUProcess(false);
        }

        public void OnConnected()
        {
            // スロット照会実行からスタート
            DoRequestGetSlotInfo();
        }

        public void DoDisconnect()
        {
            // BLE SMPサービスから切断
            ToolBLESMPService.DisconnectBLESMPService();
        }

        //
        // スロット照会
        //
        private void DoRequestGetSlotInfo()
        {
            // DFU実行開始を通知
            ToolBLEDFURef.NotifyDFUProcess(ToolGUICommon.MSG_DFU_PROCESS_TRANSFER_IMAGE, 0);

            // TODO: 仮の実装です。
            System.Threading.Thread.Sleep(1000);
            TerminateDFUProcess(false);
        }
    }
}
