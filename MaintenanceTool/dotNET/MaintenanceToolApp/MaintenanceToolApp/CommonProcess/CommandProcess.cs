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

            // BLEからデータ受信時のコールバックを登録
            BLEProcess.RegisterHandlerOnReceivedResponse(OnReceivedBleResponse);

            // BLEデバイス接続／切断検知時のコールバックを登録
            BLEProcess.RegisterHandlerNotifyConnectionStatus(NotifyConnectionStatus);
        }

        // 親画面に対するイベント通知
        public delegate void HandlerOnEnableButtonsOfMainUI(bool enabled);
        public event HandlerOnEnableButtonsOfMainUI OnEnableButtonsOfMainUI = null!;

        public delegate void HandlerOnNotifyMessageToMainUI(string messageText);
        public event HandlerOnNotifyMessageToMainUI OnNotifyMessageToMainUI = null!;

        // コマンド機能クラスに対するイベント通知
        public delegate void HandlerOnCommandResponse(byte CMD, byte[] responseData, bool success, string errorMessage);
        public event HandlerOnCommandResponse OnCommandResponse = null!;

        public delegate void HandlerNotifyBLEConnectionStatus(bool connected);
        public event HandlerNotifyBLEConnectionStatus NotifyBLEConnectionStatus = null!;

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

        public static void RegisterHandlerNotifyBLEConnectionStatus(HandlerNotifyBLEConnectionStatus handler)
        {
            Instance.NotifyBLEConnectionStatus += handler;
        }

        public static void UnregisterHandlerNotifyBLEConnectionStatus(HandlerNotifyBLEConnectionStatus handler)
        {
            Instance.NotifyBLEConnectionStatus -= handler;
        }

        //
        // 画面関連共通処理
        //
        public static void NotifyMessageToMainUI(string message)
        {
            Instance.OnNotifyMessageToMainUI(message);
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
                DialogUtil.ShowWarningMessage(parentWindow, formatted, errorMessage);
            }

            // ボタンを活性化
            Instance.OnEnableButtonsOfMainUI(true);
        }

        public static void OnMainWindowTerminated()
        {
            // USB接続を解放
            HIDProcess.DisconnectHIDDevice();

            // USBデバイスの脱着検知を終了
            USBDevice.TerminateUSBDeviceNotification();

            // アプリケーション終了ログを出力
            AppLogUtil.OutputLogInfo(string.Format("{0}を終了しました", AppCommon.MSG_TOOL_TITLE));
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
            // CMDが0の場合はエラー扱い
            if (CMD == 0) {
                OnCommandResponseToMainThread(CMD, data, false, AppCommon.MSG_CMDTST_INVALID_CTAPHID_CMD);
                return;
            }

            // HIDデバイスからメッセージ受信時の処理を行う
            if (CMD == HIDProcessConst.HID_CMD_CTAPHID_INIT) {
                DoResponseCtapHidInit(CMD, data);
                return;
            }

            // 正常終了扱い
            // 以降の処理を、UIスレッドに引き戻す
            OnCommandResponseToMainThread(CMD, data, true, "");
        }

        private void OnCommandResponseToMainThread(byte CMD, byte[] responseData, bool success, string errorMessage)
        {
            Application.Current.Dispatcher.Invoke(new Action(() => {
                OnCommandResponse(CMD, responseData, success, errorMessage);
            }));
        }

        //
        // CTAP2HID_INITコマンド関連処理
        //
        // nonceを保持
        private readonly byte[] NonceBytes = new byte[8];

        // INITコマンドで受信したCIDを保持
        private byte[] ReceivedCIDBytes = new byte[0];

        public static void DoRequestCtapHidInit()
        {
            // 8バイトのランダムデータを生成
            Instance.GenerateNonceBytes();

            // HIDコマンド／データを送信（CIDはダミーを使用する）
            // INITコマンドを実行し、nonce を送信する
            HIDProcess.DoRequestCommand(InitialCidBytes, HIDProcessConst.HID_CMD_CTAPHID_INIT, Instance.NonceBytes);
        }

        private void DoResponseCtapHidInit(byte CMD, byte[] response)
        {
            // nonceの一致チェック
            for (int i = 0; i < NonceBytes.Length; i++) {
                if (NonceBytes[i] != response[i]) {
                    // nonceが一致しない場合は異常終了
                    OnCommandResponseToMainThread(CMD, response, false, AppCommon.MSG_CMDTST_INVALID_NONCE);
                    return;
                }
            }

            // レスポンスされたCIDを抽出し、クラス内で保持
            ReceivedCIDBytes = ExtractReceivedCIDBytes(response);

            // 上位クラスに制御を戻す
            OnCommandResponseToMainThread(CMD, response, true, "");
        }

        private void GenerateNonceBytes()
        {
            // 8バイトのランダムデータを生成
            new Random().NextBytes(NonceBytes);
        }

        private byte[] ExtractReceivedCIDBytes(byte[] message)
        {
            // メッセージからCIDを抽出して戻す
            byte[] receivedCID = new byte[4];
            for (int j = 0; j < receivedCID.Length; j++) {
                receivedCID[j] = message[8 + j];
            }
            return receivedCID;
        }

        public static void DoRequestCtapHidCommand(byte CMD, byte[] data)
        {
            // CTAPHID_INITから応答されたCIDを使用し、HIDコマンド／データを送信
            HIDProcess.DoRequestCommand(Instance.ReceivedCIDBytes, CMD, data);
        }

        //
        // BLE関連共通処理
        //
        public static void DoRequestBleCommand(byte CMD, byte[] data)
        {
            // BLEコマンド／データを送信
            BLEProcess.DoRequestCommand(CMD, data);
        }

        public static void DoConnectBleCommand()
        {
            // BLE接続を試行
            BLEProcess.DoConnectCommand();
        }

        private void OnReceivedBleResponse(byte CMD, byte[] responseData, bool success, string errorMessage)
        {
            // 上位クラスに制御を戻す
            OnCommandResponseToMainThread(CMD, responseData, success, errorMessage);
        }

        private void NotifyConnectionStatus(bool connected)
        {
            // BLE接続／切断検知を上位クラスに転送
            NotifyBLEConnectionStatus(connected);
        }
    }
}
