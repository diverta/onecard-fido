using MaintenanceToolApp.ToolAppCommon;
using System;
using System.Diagnostics;
using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp
{
    public class UtilityProcess
    {
        // 処理実行のためのプロパティー
        private string CommandTitle = string.Empty;
        private Command CommandRef = Command.COMMAND_NONE;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow = App.Current.MainWindow;

        // HIDインターフェースからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        public UtilityProcess()
        {
            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
        }

        public void SetCommand(Command command)
        {
            CommandRef = command;
        }

        public void DoProcess()
        {
            // 実行コマンドにより処理分岐
            switch (CommandRef) {
            case Command.COMMAND_HID_GET_FLASH_STAT:
                // 処理開始メッセージを表示
                CommandTitle = AppCommon.PROCESS_NAME_GET_FLASH_STAT;
                CommandProcess.NotifyCommandStarted(CommandTitle);

                // Flash ROM情報照会を実行
                DoRequestHIDGetFlashStat();
                break;

            case Command.COMMAND_HID_GET_VERSION_INFO:
                // 処理開始メッセージを表示
                CommandTitle = AppCommon.PROCESS_NAME_GET_VERSION_INFO;
                CommandProcess.NotifyCommandStarted(CommandTitle);

                // TODO: 仮の実装です。
                CommandProcess.NotifyCommandTerminated(CommandTitle, "", true, ParentWindow);
                break;

            case Command.COMMAND_VIEW_APP_VERSION:
                // メイン画面を親ウィンドウとし、バージョン参照画面を開く
                ToolVersionWindow w = new ToolVersionWindow();
                w.ShowDialogWithOwner(ParentWindow);
                break;

            case Command.COMMAND_VIEW_LOG_FILE:
                // 管理ツールのログファイルを格納している
                // フォルダーを、Windowsのエクスプローラで参照
                ViewLogFile();
                break;

            default:
                // エラーメッセージをポップアップ表示
                DialogUtil.ShowErrorMessage(ParentWindow, AppCommon.MSG_TOOL_TITLE, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
                break;
            }
        }

        //
        // 内部処理
        //
        private void DoRequestHIDGetFlashStat()
        {
            // コマンドバイトだけを送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCommand(HIDProcessConst.HID_CMD_GET_FLASH_STAT, new byte[0]);
        }

        private void DoResponseHIDGetFlashStat(byte[] responseData)
        {
            // TODO: 仮の実装です。
            CommandProcess.NotifyCommandTerminated(CommandTitle, "", true, ParentWindow);
        }

        private void ViewLogFile()
        {
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
        }

        //
        // HIDからのレスポンス振分け処理
        //
        private void OnCommandResponse(byte[] responseData, bool success, string errorMessage)
        {
            // イベントを解除
            CommandProcess.UnregisterHandlerOnCommandResponse(OnCommandResponseRef);

            // 即時でアプリケーションに制御を戻す
            if (success == false) {
                CommandProcess.NotifyCommandTerminated(CommandTitle, errorMessage, success, ParentWindow);
                return;
            }

            // レスポンスメッセージの１バイト目（ステータスコード）を確認
            if (responseData[0] != FIDODefine.CTAP1_ERR_SUCCESS) {
                // エラーの場合は画面に制御を戻す
                CommandProcess.NotifyCommandTerminated(CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false, ParentWindow);
                return;
            }

            // 実行コマンドにより処理分岐
            switch (CommandRef) {
            case Command.COMMAND_HID_GET_FLASH_STAT:
                DoResponseHIDGetFlashStat(responseData);
                break;
            default:
                // メイン画面に制御を戻す
                CommandProcess.NotifyCommandTerminated(CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false, ParentWindow);
                break;
            }
        }
    }
}
