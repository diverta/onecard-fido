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

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow = App.Current.MainWindow;

        // 親画面に対するイベント通知
        public delegate void HandlerOnEnableButtonsOfMainUI(bool enabled);
        public event HandlerOnEnableButtonsOfMainUI OnEnableButtonsOfMainUI = null!;

        public delegate void HandlerOnNotifyMessageToMainUI(string messageText);
        public event HandlerOnNotifyMessageToMainUI OnNotifyMessageToMainUI = null!;

        //
        // 外部公開用
        //
        public static void RegisterHandlerOnEnableButtonsOfMainUI(HandlerOnEnableButtonsOfMainUI handler)
        {
            Instance.OnEnableButtonsOfMainUI += handler;
        }

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
                w.ShowDialogWithOwner(ParentWindow);

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
                // エラーメッセージをポップアップ表示
                DialogUtil.ShowErrorMessage(ParentWindow, AppCommon.MSG_TOOL_TITLE, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
            }
        }

        //
        // 共通処理
        //
        private void NotifyCommandStarted(string commandName)
        {
            // ボタンを不活性化
            OnEnableButtonsOfMainUI(false);

            // コマンド名が設定されていない場合は終了
            if (commandName.Equals(String.Empty)) {
                return;
            }

            // コマンド開始メッセージを画面表示し、ログファイルにも出力
            string formatted = string.Format(AppCommon.MSG_FORMAT_START_MESSAGE, commandName);
            OnNotifyMessageToMainUI(formatted);
            AppLogUtil.OutputLogInfo(formatted);
        }

        private void NotifyCommandTerminated(string commandName, string errorMessage, bool success, Window parentWindow)
        {
            // 処理失敗時は、引数に格納されたエラーメッセージを画面出力
            if (success == false) {
                OnNotifyMessageToMainUI(errorMessage);
            }

            // コマンド名が設定されていない場合は、ボタンを活性化して終了
            if (commandName.Equals(String.Empty)) {
                OnEnableButtonsOfMainUI(true);
                return;
            }

            // テキストエリアとポップアップの両方に表示させる処理終了メッセージを作成
            string formatted = string.Format(AppCommon.MSG_FORMAT_END_MESSAGE,
                commandName, success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);

            // メッセージを画面のテキストエリアに表示
            OnNotifyMessageToMainUI(formatted);

            // メッセージをログファイルに出力してから、ポップアップを表示
            if (success) {
                AppLogUtil.OutputLogInfo(formatted);
                DialogUtil.ShowInfoMessage(parentWindow, AppCommon.MSG_TOOL_TITLE, formatted);

            } else {
                AppLogUtil.OutputLogError(formatted);
                DialogUtil.ShowWarningMessage(parentWindow, AppCommon.MSG_TOOL_TITLE, formatted);
            }

            // ボタンを活性化
            OnEnableButtonsOfMainUI(true);
        }
    }
}
