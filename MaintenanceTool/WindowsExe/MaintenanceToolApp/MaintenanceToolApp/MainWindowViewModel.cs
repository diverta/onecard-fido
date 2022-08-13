using System.Reflection;
using ToolAppCommon;

namespace MaintenanceToolApp
{
    public class MainWindowViewModel
    {
        public string Title { get; set; }

        // コマンドクラスの参照を保持
        public MainWindowQuitCommand MainWindowQuitCommandRef { get; set; }

        public MainWindowViewModel()
        {
            // メイン画面のタイトル
            Title = AppCommon.MSG_TOOL_TITLE;

            // コマンドクラスを生成
            MainWindowQuitCommandRef = new MainWindowQuitCommand();
        }
    }
}
