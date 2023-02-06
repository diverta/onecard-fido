using MaintenanceToolApp;
using MaintenanceToolApp.CommonProcess;
using ToolAppCommon;
using VendorMaintenanceTool.CommonProcess;
using VendorMaintenanceTool.FIDOSettings;
using static MaintenanceToolApp.AppDefine;

namespace VendorMaintenanceTool.VendorFunction
{
    internal class VendorFunctionParameter
    {
        public Command Command { get; set; }
        public string CommandTitle { get; set; }
        public bool CommandSuccess { get; set; }
        public string ResultMessage { get; set; }
        public string ResultInformativeMessage { get; set; }

        // 画面で選択された鍵・証明書ファイルパスを保持
        public string KeyPath { get; set; }
        public string CertPath { get; set; }

        public VendorFunctionParameter()
        {
            Command = Command.COMMAND_NONE;
            CommandTitle = string.Empty;
            ResultMessage = string.Empty;
            ResultInformativeMessage = string.Empty;
            KeyPath = string.Empty;
            CertPath = string.Empty;
        }
    }

    internal class VendorFunctionProcess
    {
        // 処理実行のためのプロパティー
        private VendorFunctionParameter Parameter = null!;

        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnNotifyProcessTerminated();
        private HandlerOnNotifyProcessTerminated OnNotifyProcessTerminated = null!;

        public void DoProcess(VendorFunctionParameter parameter, HandlerOnNotifyProcessTerminated handlerRef)
        {
            // 画面から引き渡されたパラメーターを退避
            Parameter = parameter;

            // コールバックを保持
            OnNotifyProcessTerminated = handlerRef;

            // 処理開始を通知
            NotifyProcessStarted();

            // コマンドに応じ、以下の処理に分岐
            switch (Parameter.Command) {
            case Command.COMMAND_INSTALL_SKEY_CERT:
                DoRequestInstallAttestation();
                break;
            case Command.COMMAND_ERASE_SKEY_CERT:
                DoRequestRemoveAttestation();
                break;
            case Command.COMMAND_HID_BOOTLOADER_MODE:
                DoRequestBootloaderMode();
                break;
            case Command.COMMAND_HID_FIRMWARE_RESET:
                DoRequestFirmwareReset();
                break;
            default:
                break;
            }
        }

        //
        // 鍵・証明書インストール
        //
        private void DoRequestInstallAttestation()
        {
            new FIDOAttestationProcess(Parameter).DoProcess(DoResponseInstallAttestation);
        }

        private void DoResponseInstallAttestation(bool success, string errorMessage)
        {
            // 画面に制御を戻す
            NotifyProcessTerminated(success, errorMessage);
        }

        //
        // 鍵・証明書の削除
        //
        private void DoRequestRemoveAttestation()
        {
            new FIDOAttestationProcess(Parameter).DoProcess(DoResponseRemoveAttestation);
        }

        private void DoResponseRemoveAttestation(bool success, string errorMessage)
        {
            // 画面に制御を戻す
            NotifyProcessTerminated(success, errorMessage);
        }

        //
        // ブートローダーモード遷移
        //
        private void DoRequestBootloaderMode()
        {
            new BootloaderModeProcess().DoProcess(DoResponseBootloaderMode);
        }

        private void DoResponseBootloaderMode(bool success, string errorMessage)
        {
            // 画面に制御を戻す
            NotifyProcessTerminated(success, errorMessage);
        }

        //
        // 認証器のファームウェア再起動
        //
        private void DoRequestFirmwareReset()
        {
            new FirmwareResetProcess().DoProcess(DoResponseFirmwareReset);
        }

        private void DoResponseFirmwareReset(bool success, string errorMessage)
        {
            // 画面に制御を戻す
            NotifyProcessTerminated(success, errorMessage);
        }

        // 
        // 共通処理
        //
        private void NotifyProcessStarted()
        {
            // コマンド開始メッセージをログファイルに出力
            string startMsg = string.Format(AppCommon.MSG_FORMAT_START_MESSAGE, Parameter.CommandTitle);
            AppLogUtil.OutputLogInfo(startMsg);
        }

        private void NotifyProcessTerminated(bool success, string errorMessage)
        {
            // エラーメッセージを画面＆ログ出力
            if (success == false && errorMessage.Length > 0) {
                // ログ出力する文言からは、改行文字を除去
                AppLogUtil.OutputLogError(AppUtil.ReplaceCRLF(errorMessage));
                Parameter.ResultInformativeMessage = errorMessage;
            }

            // コマンドの実行結果をログ出力
            string formatted = string.Format(AppCommon.MSG_FORMAT_END_MESSAGE,
                Parameter.CommandTitle,
                success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);
            if (success) {
                AppLogUtil.OutputLogInfo(formatted);
            } else {
                AppLogUtil.OutputLogError(formatted);
            }

            // パラメーターにコマンド成否を設定
            Parameter.CommandSuccess = success;
            Parameter.ResultMessage = formatted;

            // 画面に制御を戻す
            OnNotifyProcessTerminated();
        }
    }
}
