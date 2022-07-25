using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public class ToolPIVConst
    {
        public const byte PIV_INS_SELECT = 0xA4;
    }

    public class ToolPIVParameter
    {
        // 鍵作成用パラメーター
        public byte PkeySlotId { get; set; }
        public string PkeyPemPath { get; set; }
        public string CertPemPath { get; set; }
        public string AuthPin { get; set; }
        public string CurrentPin { get; set; }
        public string RenewalPin { get; set; }
        public AppCommon.RequestType SelectedPinCommand { get; set; }
        public string SelectedPinCommandName { get; set; }
    }

    public class ToolPIV
    {
        // PIV機能設定画面
        private PIVPreferenceForm PreferenceForm;

        // メイン画面の参照を保持
        private MainForm MainFormRef;

        // 処理クラスの参照を保持
        private HIDMain HidMainRef;
        private ToolPIVCcid PIVCcid;

        // 処理機能を保持
        private AppCommon.RequestType RequestType;

        // 処理機能名を保持
        private string NameOfCommand;

        // エラーメッセージテキストを保持
        private string ErrorMessageOfCommand;

        // コマンドが成功したかどうかを保持
        private bool CommandSuccess;

        // リクエストパラメーターを保持
        private ToolPIVParameter Parameter = null;

        public ToolPIV(MainForm f, HIDMain h)
        {
            // メイン画面の参照を保持
            MainFormRef = f;

            // HID処理クラスの参照を保持
            HidMainRef = h;

            // PIV機能設定画面を生成
            PreferenceForm = new PIVPreferenceForm(this);

            // CCID処理クラスを生成
            PIVCcid = new ToolPIVCcid();
            PIVCcid.OnCcidCommandTerminated += OnCcidCommandTerminated;
            PIVCcid.OnCcidCommandNotifyErrorMessage += OnCcidCommandNotifyErrorMessage;
        }

        public void ShowDialog()
        {
            // ツール設定画面を表示
            PreferenceForm.ShowDialog();
        }

        public bool CheckUSBDeviceDisconnected()
        {
            return MainFormRef.CheckUSBDeviceDisconnected();
        }

        //
        // ファームウェアリセット用関数
        //
        public void DoCommandResetFirmware()
        {
            // HIDインターフェース経由でファームウェアをリセット
            AppCommon.RequestType requestType = AppCommon.RequestType.HidFirmwareReset;
            NotifyProcessStarted(requestType);
            HidMainRef.DoFirmwareReset(requestType, this);
        }

        public void DoResponseResetFirmware(bool success)
        {
            if (success == false) {
                NotifyErrorMessage(AppCommon.MSG_FIRMWARE_RESET_UNSUPP);
            }
            NotifyProcessTerminated(success);
        }

        //
        // OpenPGP機能設定用関数
        // 
        public void DoOpenPIVCommand(AppCommon.RequestType requestType, ToolPIVParameter parameter)
        {
            // 画面から引き渡されたパラメーターを退避
            Parameter = parameter;

            // コマンド開始処理
            NotifyProcessStarted(requestType);

            // TODO: 仮の実装です。
            NotifyProcessTerminated(true);
        }

        // 
        // 共通処理
        //
        private void NotifyProcessStarted(AppCommon.RequestType requestType)
        {
            // コマンド処理結果を初期化
            CommandSuccess = false;

            // 処理機能に応じ、以下の処理に分岐
            RequestType = requestType;
            switch (RequestType) {
            case AppCommon.RequestType.HidFirmwareReset:
                NameOfCommand = AppCommon.PROCESS_NAME_FIRMWARE_RESET;
                break;
            case AppCommon.RequestType.PIVImportKey:
                NameOfCommand = AppCommon.PROCESS_NAME_CCID_PIV_IMPORT_KEY;
                break;
            case AppCommon.RequestType.PIVStatus:
                NameOfCommand = AppCommon.PROCESS_NAME_CCID_PIV_STATUS;
                break;
            case AppCommon.RequestType.PIVSetChuId:
                NameOfCommand = AppCommon.PROCESS_NAME_CCID_PIV_SET_CHUID;
                break;
            case AppCommon.RequestType.PIVReset:
                NameOfCommand = AppCommon.PROCESS_NAME_CCID_PIV_RESET;
                break;
            default:
                NameOfCommand = "";
                break;
            }

            // コマンド開始メッセージをログファイルに出力
            string startMsg = string.Format(AppCommon.MSG_FORMAT_START_MESSAGE, NameOfCommand);
            AppUtil.OutputLogInfo(startMsg);
        }

        private void NotifyErrorMessage(string message)
        {
            // エラーメッセージをログファイルに出力（出力前に改行文字を削除）
            AppUtil.OutputLogError(message.Replace("\n", ""));

            // 戻り先画面に表示させるためのエラーメッセージを保持
            ErrorMessageOfCommand = message;
        }

        private void NotifyProcessTerminated(bool success)
        {
            // コマンドの実行結果をログ出力
            string formatted = string.Format(AppCommon.MSG_FORMAT_END_MESSAGE,
                NameOfCommand,
                success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);
            if (success) {
                AppUtil.OutputLogInfo(formatted);
            } else {
                AppUtil.OutputLogError(formatted);
            }

            // 進捗画面を閉じる
            CommonProcessingForm.NotifyTerminate();

            // 画面に制御を戻す
            PreferenceForm.OnCommandProcessTerminated(RequestType, success, ErrorMessageOfCommand);
        }

        //
        // ToolPIVCcidクラスからのコールバック
        //
        private void OnCcidCommandNotifyErrorMessage(string errorMessage)
        {
            NotifyErrorMessage(errorMessage);
        }

        private void OnCcidCommandTerminated(bool success)
        {
            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
            default:
                NotifyProcessTerminated(false);
                break;
            }
        }
    }
}
