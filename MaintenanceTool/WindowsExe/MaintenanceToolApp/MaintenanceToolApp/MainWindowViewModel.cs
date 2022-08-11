using System.Reflection;
using ToolAppCommon;

namespace MaintenanceToolApp
{
    public class MainWindowViewModel
    {
        public string Title { get; set; }
        public string MaintenanceToolVersion { get; set; }

        // コマンドクラスの参照を保持
        public MainWindowCommand MainWindowCommandRef { get; set; }

        public MainWindowViewModel()
        {
            // メイン画面のタイトル
            Title = AppCommon.MSG_TOOL_TITLE;

            // コマンドクラスを生成
            MainWindowCommandRef = new MainWindowCommand();

            // ツールのバージョンを取得
            MaintenanceToolVersion = string.Format("Version {0}", GetMaintenanceToolVersion());

            // アプリケーション開始ログを出力
            AppLogUtil.SetOutputLogApplName("MaintenanceToolApp");
            AppLogUtil.OutputLogInfo(string.Format("{0}を起動しました: {1}", AppCommon.MSG_TOOL_TITLE, MaintenanceToolVersion));
        }

        private static string GetMaintenanceToolVersion()
        {
            // 製品バージョン文字列を戻す
            Assembly asm = Assembly.GetExecutingAssembly();
            System.Diagnostics.FileVersionInfo ver = System.Diagnostics.FileVersionInfo.GetVersionInfo(asm.Location);
            string? versionString = ver.ProductVersion;
            if (versionString == null) {
                return "";
            } else {
                return versionString;
            }
        }
    }
}
