namespace MaintenanceToolGUI
{
    public class ToolDFU
    {
        // 画面の参照を保持
        private MainForm mainForm;
        private DFUStartForm dfuStartForm;
        private DFUProcessingForm dfuProcessingForm;

        // 処理クラスの参照を保持
        private HIDMain hidMain;

        // リソース名称検索用キーワード
        private const string ResourceNamePrefix = "MaintenanceToolGUI.Resources.app_dfu_package.";
        private const string ResourceNameSuffix = ".zip";

        // 更新イメージファイルのリソース名称
        private string DFUImageResourceName;

        // 更新イメージファイル名から取得したバージョン
        public string UpdateVersion { get; set; }

        // 認証器からHID経由で取得したバージョン
        public string CurrentVersion { get; set; }

        public ToolDFU(MainForm f, HIDMain h)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // HID処理クラスの参照を保持
            hidMain = h;

            // HID処理クラスに、本クラスの参照を設定
            hidMain.ToolDFURef = this;

            // 処理開始／進捗画面を生成
            dfuStartForm = new DFUStartForm(this);
            dfuProcessingForm = new DFUProcessingForm(this);

            // ファームウェア更新イメージファイルから、更新バージョンを取得
            GetDFUImageFileResourceName();
            ExtractUpdateVersion(DFUImageResourceName);
        }

        private void GetDFUImageFileResourceName()
        {
            // リソース名称を初期化
            DFUImageResourceName = "";

            // このアプリケーションに同梱されているリソース名を取得
            System.Reflection.Assembly myAssembly = System.Reflection.Assembly.GetExecutingAssembly();
            string[] resnames = myAssembly.GetManifestResourceNames();
            foreach (string resName in resnames) {
                // リソース名が
                // "MaintenanceToolGUI.Resources.app_dfu_package."
                // という名称で始まっている場合は、
                // ファームウェア更新イメージファイルと判定
                if (resName.StartsWith(ResourceNamePrefix)) {
                    DFUImageResourceName = resName;
                }
            }
        }

        private void ExtractUpdateVersion(string resName)
        {
            // バージョン文字列を初期化
            UpdateVersion = "";
            if (resName.Equals("")) {
                return;
            }
            if (resName.EndsWith(ResourceNameSuffix) == false) {
                return;
            }
            // リソース名称文字列から、バージョン文字列だけを抽出
            UpdateVersion = resName.Replace(ResourceNamePrefix, "").Replace(ResourceNameSuffix, "");
        }

        public void OpenDFUStartForm()
        {
            // バージョンチェックが不正の場合は処理を終了
            if (DFUImageIsUnavailable()) {
                return;
            }

            // 処理開始画面を表示
            if (dfuStartForm.OpenForm()) {
                // 処理開始画面でOKボタンがクリックされた場合、
                // 処理進捗画面を表示
                dfuProcessingForm.ShowDialog();
            }
        }

        private bool DFUImageIsUnavailable()
        {
            // 更新イメージファイル名からバージョンが取得できていない場合は利用不可
            if (UpdateVersion == "") {
                FormUtil.ShowWarningMessage(
                    ToolGUICommon.MSG_DFU_IMAGE_NOT_AVAILABLE, 
                    ToolGUICommon.MSG_DFU_UPDATE_VERSION_UNKNOWN);
                return true;
            }

            // HID経由で認証器の現在バージョンが取得できていない場合は利用不可
            if (CurrentVersion == "") {
                FormUtil.ShowWarningMessage(
                    ToolGUICommon.MSG_DFU_IMAGE_NOT_AVAILABLE,
                    ToolGUICommon.MSG_DFU_CURRENT_VERSION_UNKNOWN);
                return true;
            }

            return false;
        }
    }
}
