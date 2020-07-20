namespace MaintenanceToolGUI
{
    public sealed class ToolContext
    {
        // このクラスのインスタンスを保持
        private static ToolContext SharedInstance = new ToolContext();

        // ヘルスチェック実行種別を保持
        public enum HealthCheckType
        {
            CTAP2 = 0,
            U2F
        };
        public HealthCheckType HchkType { get; set; }

        // 認証器の設定値を保持
        public bool BleScanAuthEnabled { get; set; }

        public static ToolContext GetInstance()
        {
            return SharedInstance;
        }

        private ToolContext()
        {
            InitBleScanAuthParamValues();
        }

        private void InitBleScanAuthParamValues()
        {
            // 自動認証パラメーターをクリア
            BleScanAuthEnabled = false;
        }

        public void SetBleScanAuthParamValues(string[] values)
        {
            if (values.Length != 3) {
                // データが完備していない場合は該当項目をクリア
                InitBleScanAuthParamValues();
                return;
            }

            // 配列の先頭から、自動認証機能の有効／無効区分を取得
            BleScanAuthEnabled = (values[0] == "1");
        }
    }
}
