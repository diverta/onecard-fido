using System;
using System.Diagnostics;
using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp
{
    internal class UtilityProcess
    {
        // このクラスのインスタンス
        private static readonly UtilityProcess Instance = new UtilityProcess();

        // 処理実行のためのプロパティー
        private string CommandTitle = string.Empty;

        // HID接続完了時のイベント
        public delegate void HandlerOnNotifyMessageToMainUI(string messageText);
        public event HandlerOnNotifyMessageToMainUI OnNotifyMessageToMainUI = null!;

        //
        // 外部公開用
        //
        public static void RegisterHandlerOnNotifyMessageToMainUI(HandlerOnNotifyMessageToMainUI handler)
        {
            Instance.OnNotifyMessageToMainUI += handler;
        }

        public static void SetCommandTitle(string commandTitle)
        {
            Instance.CommandTitle = commandTitle;
        }

        public static void DoProcess()
        {
            Instance.DoUtilityProcess();
        }

        //
        // 内部処理
        //
        public void DoUtilityProcess()
        {
            if (CommandTitle.Equals(AppCommon.PROCESS_NAME_TOOL_VERSION_INFO)) {
                // メイン画面を親ウィンドウとし、バージョン参照画面を開く
                ToolVersionWindow w = new ToolVersionWindow();
                Window parentWindow = App.Current.MainWindow;
                w.ShowDialogWithOwner(parentWindow);

            } else if (CommandTitle.Equals(AppCommon.PROCESS_NAME_VIEW_LOG_FILE)) {
                // 管理ツールのログファイルを格納している
                // フォルダーを、Windowsのエクスプローラで参照
                try {
                    var procInfo = new ProcessStartInfo {
                        FileName = AppLogUtil.OutputLogFileDirectoryPath(),
                        UseShellExecute = true
                    };
                    Process.Start(procInfo);

                } catch (Exception e) {
                    AppLogUtil.OutputLogError(String.Format(AppCommon.MSG_FORMAT_UTILITY_VIEW_LOG_FILE_ERR, e.Message));
                }

            } else {
                // 処理完了を通知
                OnNotifyMessageToMainUI(AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
            }
        }
    }
}
