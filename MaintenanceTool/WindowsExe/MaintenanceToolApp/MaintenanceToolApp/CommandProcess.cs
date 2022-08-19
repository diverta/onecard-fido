using System;
using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp
{
    public class CommandProcess
    {
        // このクラスのインスタンス
        private static readonly CommandProcess Instance = new CommandProcess();

        private CommandProcess()
        {
            // HIDインターフェースからデータ受信時のコールバックを登録
            HIDProcess.RegisterHandlerOnReceivedResponse(OnReceivedResponse);
        }

        // 親画面に対するイベント通知
        public delegate void HandlerOnEnableButtonsOfMainUI(bool enabled);
        public event HandlerOnEnableButtonsOfMainUI OnEnableButtonsOfMainUI = null!;

        public delegate void HandlerOnNotifyMessageToMainUI(string messageText);
        public event HandlerOnNotifyMessageToMainUI OnNotifyMessageToMainUI = null!;

        // コマンド機能クラスに対するイベント通知
        public delegate void HandlerOnCommandResponse(byte[] responseData, bool success, string errorMessage);
        public event HandlerOnCommandResponse OnCommandResponse = null!;

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

        public static void RegisterHandlerOnCommandResponse(HandlerOnCommandResponse handler)
        {
            Instance.OnCommandResponse += handler;
        }

        public static void UnregisterHandlerOnCommandResponse(HandlerOnCommandResponse handler)
        {
            Instance.OnCommandResponse -= handler;
        }

        public static void NotifyCommandStarted(string commandName)
        {
            // ボタンを不活性化
            Instance.OnEnableButtonsOfMainUI(false);

            // コマンド名が設定されていない場合は終了
            if (commandName.Equals(String.Empty)) {
                return;
            }

            // コマンド開始メッセージを画面表示し、ログファイルにも出力
            string formatted = string.Format(AppCommon.MSG_FORMAT_START_MESSAGE, commandName);
            Instance.OnNotifyMessageToMainUI(formatted);
            AppLogUtil.OutputLogInfo(formatted);
        }

        public static void NotifyCommandTerminated(string commandName, string errorMessage, bool success, Window parentWindow)
        {
            // 処理失敗時は、引数に格納されたエラーメッセージを画面出力
            if (success == false) {
                Instance.OnNotifyMessageToMainUI(errorMessage);
            }

            // コマンド名が設定されていない場合は、ボタンを活性化して終了
            if (commandName.Equals(String.Empty)) {
                Instance.OnEnableButtonsOfMainUI(true);
                return;
            }

            // テキストエリアとポップアップの両方に表示させる処理終了メッセージを作成
            string formatted = string.Format(AppCommon.MSG_FORMAT_END_MESSAGE,
                commandName, success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);

            // メッセージを画面のテキストエリアに表示
            Instance.OnNotifyMessageToMainUI(formatted);

            // メッセージをログファイルに出力してから、ポップアップを表示
            if (success) {
                AppLogUtil.OutputLogInfo(formatted);
                DialogUtil.ShowInfoMessage(parentWindow, AppCommon.MSG_TOOL_TITLE, formatted);

            } else {
                AppLogUtil.OutputLogError(formatted);
                DialogUtil.ShowWarningMessage(parentWindow, AppCommon.MSG_TOOL_TITLE, formatted);
            }

            // ボタンを活性化
            Instance.OnEnableButtonsOfMainUI(true);
        }

        //
        // HID関連共通処理
        //
        // ダミーCID
        private static readonly byte[] InitialCidBytes = new byte[] { 0xff, 0xff, 0xff, 0xff };

        public static void DoRequestCommand(byte CMD, byte[] data)
        {
            // HIDコマンド／データを送信（CIDはダミーを使用する）
            HIDProcess.DoRequestCommand(InitialCidBytes, CMD, data);
        }

        private void OnReceivedResponse(byte[] cid, byte CMD, byte[] data)
        {
            // 正常終了扱い
            // 以降の処理を、UIスレッドに引き戻す
            Application.Current.Dispatcher.Invoke(new Action(() => {
                OnCommandResponse(data, true, "");
            }));
        }
    }
}
