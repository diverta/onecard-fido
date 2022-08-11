using ToolAppCommon;

namespace MaintenanceToolApp
{
    public class MainWindowViewModel
    {
        public string Title { get; set; }

        public MainWindowViewModel()
        {
            // メイン画面のタイトル
            Title = AppCommon.MSG_TOOL_TITLE;

            // アプリケーション開始ログを出力
            AppLogUtil.SetOutputLogApplName("MaintenanceToolApp");
            AppLogUtil.OutputLogInfo(string.Format("{0}を起動しました: {1}", AppCommon.MSG_TOOL_TITLE, "0.0.0"));
        }
    }
}
