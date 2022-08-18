using MaintenanceToolApp.ToolAppCommon;
using System;
using System.Diagnostics;
using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp
{
    internal class UtilityProcess
    {
        // このクラスのインスタンス
        private static readonly UtilityProcess Instance = new UtilityProcess();

        private UtilityProcess()
        {
            // HIDインターフェースからデータ受信時のコールバックを登録
            HIDProcess.RegisterHandlerOnReceivedResponse(OnReceivedResponse);
        }

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
            if (CommandTitle.Equals(AppCommon.PROCESS_NAME_GET_FLASH_STAT)) {
                // 処理開始メッセージを表示
                NotifyCommandStarted(CommandTitle);

                // Flash ROM情報照会を実行
                DoRequestHIDGetFlashStat();

            } else if (CommandTitle.Equals(AppCommon.PROCESS_NAME_GET_VERSION_INFO)) {
                // 処理開始メッセージを表示
                NotifyCommandStarted(CommandTitle);

                // TODO: 仮の実装です。
                NotifyCommandTerminated(CommandTitle, "", true, ParentWindow);

            } else if (CommandTitle.Equals(AppCommon.PROCESS_NAME_TOOL_VERSION_INFO)) {
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

        public void DoRequestHIDGetFlashStat()
        {
            // コマンドバイトだけを送信する
            DoRequestCommand(Command.COMMAND_HID_GET_FLASH_STAT, HIDProcessConst.HID_CMD_GET_FLASH_STAT, new byte[0]);
        }

        private void DoResponseHIDGetFlashStat(byte[] responseData)
        {
            // TODO: 仮の実装です。
            NotifyCommandTerminated(CommandTitle, "", true, ParentWindow);
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

        private void OnCommandResponse(Command command, byte[] responseData, bool success, string errorMessage)
        {
            // 即時でアプリケーションに制御を戻す
            if (success == false) {
                NotifyCommandTerminated(CommandTitle, errorMessage, success, ParentWindow);
                return;
            }

            // レスポンスメッセージの１バイト目（ステータスコード）を確認
            if (responseData[0] != FIDODefine.CTAP1_ERR_SUCCESS) {
                // エラーの場合は画面に制御を戻す
                NotifyCommandTerminated(CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false, ParentWindow);
                return;
            }

            // 実行コマンドにより処理分岐
            switch (command) {
            case Command.COMMAND_HID_GET_FLASH_STAT:
                DoResponseHIDGetFlashStat(responseData);
                break;
            default:
                // メイン画面に制御を戻す
                NotifyCommandTerminated(CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false, ParentWindow);
                break;
            }
        }

        //
        // HID関連共通処理
        //
        // ダミーCID
        private static readonly byte[] InitialCidBytes = new byte[] { 0xff, 0xff, 0xff, 0xff };

        // リクエスト送信時のコマンド種別を保持
        private Command CommandRef = Command.COMMAND_NONE;

        private void DoRequestCommand(Command command, byte CMD, byte[] data)
        {
            // 実行コマンドを保持
            CommandRef = command;

            // HIDコマンド／データを送信（CIDはダミーを使用する）
            HIDProcess.DoRequestCommand(InitialCidBytes, CMD, data);
        }

        private void OnReceivedResponse(byte[] cid, byte CMD, byte[] data)
        {
            // 正常終了扱い
            // 以降の処理を、UIスレッドに引き戻す
            Application.Current.Dispatcher.Invoke(new Action(() => {
                OnCommandResponse(CommandRef, data, true, "");
            }));
        }
    }
}
