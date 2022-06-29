namespace MaintenanceToolGUI
{
    public class ToolPIV
    {
        // PIV機能設定画面
        private PIVPreferenceForm PreferenceForm;

        // メイン画面の参照を保持
        private MainForm MainFormRef;

        // 処理クラスの参照を保持
        private HIDMain HidMainRef;

        public ToolPIV(MainForm f, HIDMain h)
        {
            // メイン画面の参照を保持
            MainFormRef = f;

            // HID処理クラスの参照を保持
            HidMainRef = h;

            // PIV機能設定画面を生成
            PreferenceForm = new PIVPreferenceForm(this);
        }

        public void ShowDialog()
        {
            // ツール設定画面を表示
            PreferenceForm.ShowDialog();
        }
    }
}
