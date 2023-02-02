using MaintenanceToolApp;
using System.Threading;
using ToolAppCommon;
using VendorMaintenanceTool.VendorFunction;
using static MaintenanceToolApp.FIDODefine;
using static VendorMaintenanceTool.VendorAppCommon;

namespace VendorMaintenanceTool.FIDOSettings
{
    internal class FIDOAttestationProcess
    {
        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnNotifyCommandTerminated(bool success, string errorMessage);
        private HandlerOnNotifyCommandTerminated OnNotifyCommandTerminated = null!;

        // HIDからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        // 処理実行のためのプロパティー
        private readonly VendorFunctionParameter Parameter = null!;

        //
        // 外部公開用
        //
        public FIDOAttestationProcess(VendorFunctionParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
        }

        public void DoProcess(HandlerOnNotifyCommandTerminated handlerRef)
        {
            // コールバックを保持
            OnNotifyCommandTerminated = handlerRef;

            // CTAPHID_INITから実行
            DoRequestCtapHidInit();
        }

        //
        // 内部処理
        //
        private void DoRequestCtapHidInit()
        {
            // INITコマンドを実行し、nonce を送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidInit();
        }

        private void DoResponseCtapHidInit()
        {
            if (Parameter.Command == AppDefine.Command.COMMAND_INSTALL_SKEY_CERT) {
                DoRequestHidInstallAttestation();
            }
            if (Parameter.Command == AppDefine.Command.COMMAND_ERASE_SKEY_CERT) {
                DoRequestHidRemoveAttestation();
            }
        }

        private void DoRequestHidInstallAttestation()
        {
            // 鍵ファイルを読込
            byte[] keyPemBytes;
            string errorMessage;
            if (FIDOAttestationUtil.ReadKeyPemFile(Parameter.KeyPath, out keyPemBytes, out errorMessage) == false) {
                OnNotifyCommandTerminated(false, errorMessage);
                return;
            }

            // 秘密鍵を抽出
            byte[] keyBytes = FIDOAttestationUtil.ExtractPrivateKeyFromPemBytes(keyPemBytes);

            // 証明書ファイルを読込
            byte[] certBytes;
            if (FIDOAttestationUtil.ReadCertFile(Parameter.CertPath, out certBytes) == false) {
                OnNotifyCommandTerminated(false, MSG_CANNOT_READ_CERT_CRT_FILE);
                return;
            }

            // 鍵ファイルを使用し、証明書の公開鍵を検証
            if (FIDOAttestationUtil.ValidateSkeyCert(keyPemBytes, certBytes) == false) {
                OnNotifyCommandTerminated(false, MSG_INVALID_SKEY_OR_CERT);
                return;
            }

            // TODO: 仮の実装です。
            Thread.Sleep(2000);
            OnNotifyCommandTerminated(true, AppCommon.MSG_NONE);
        }

        private void DoResponseHidInstallAttestation()
        {
        }

        private void DoRequestHidRemoveAttestation()
        {
            // 鍵・証明書削除コマンドを実行する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidCommand(0x80 | MNT_COMMAND_BASE, new byte[] { MNT_COMMAND_RESET_ATTESTATION });
        }

        private void DoResponseHidRemoveAttestation()
        {
            OnNotifyCommandTerminated(true, AppCommon.MSG_NONE);
        }

        //
        // HIDからのレスポンス振分け処理
        //
        private void OnCommandResponse(byte CMD, byte[] responseData, bool success, string errorMessage)
        {
            // イベントを解除
            CommandProcess.UnregisterHandlerOnCommandResponse(OnCommandResponseRef);

            if (Parameter.Command != AppDefine.Command.COMMAND_INSTALL_SKEY_CERT &&
                Parameter.Command != AppDefine.Command.COMMAND_ERASE_SKEY_CERT) {
                return;
            }

            // 即時でアプリケーションに制御を戻す
            if (success == false) {
                OnNotifyCommandTerminated(success, errorMessage);
                return;
            }

            // INITからの戻りの場合
            if (CMD == HIDProcessConst.HID_CMD_CTAPHID_INIT) {
                DoResponseCtapHidInit();
                return;
            }

            // レスポンスメッセージの１バイト目（ステータスコード）を確認
            if (responseData[0] != 0x00) {
                string msg = string.Format(AppCommon.MSG_OCCUR_UNKNOWN_ERROR_ST, responseData[0]);
                OnNotifyCommandTerminated(false, msg);
            }

            // 後続の処理を実行
            if (Parameter.Command == AppDefine.Command.COMMAND_INSTALL_SKEY_CERT) {
                DoResponseHidInstallAttestation();
            }
            if (Parameter.Command == AppDefine.Command.COMMAND_ERASE_SKEY_CERT) {
                DoResponseHidRemoveAttestation();
            }
        }
    }
}
