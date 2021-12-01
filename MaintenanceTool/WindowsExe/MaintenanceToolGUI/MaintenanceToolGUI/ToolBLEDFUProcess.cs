namespace MaintenanceToolGUI
{
    public class ToolBLEDFUProcess
    {
        // クラスの参照を保持
        private ToolBLEDFU ToolBLEDFURef;
        private ToolBLEDFUImage ToolBLEDFUImageRef;

        public ToolBLEDFUProcess(ToolBLEDFUImage imageRef)
        {
            ToolBLEDFUImageRef = imageRef;
        }

        public void PerformDFU(ToolBLEDFU dfuRef)
        {
            ToolBLEDFURef = dfuRef;

            // TODO: 仮の実装です。
            System.Threading.Thread.Sleep(1000);
            TerminateDFUProcess(false);
        }

        private void TerminateDFUProcess(bool success)
        {
            // メイン画面にDFU処理の終了を通知
            ToolBLEDFURef.DFUProcessTerminated(success);
        }
    }
}
