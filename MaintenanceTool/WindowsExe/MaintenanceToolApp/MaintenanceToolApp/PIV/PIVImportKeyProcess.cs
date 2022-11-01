using System;
using ToolAppCommon;

namespace MaintenanceToolApp.PIV
{
    public class PIVImportKeyParameter
    {
        // スロットID
        public byte PkeySlotId { get; set; }

        // 鍵・証明書のアルゴリズムを保持
        public string PkeyAlgName { get; set; }
        public string CertAlgName { get; set; }
        public byte PkeyAlgorithm { get; set; }
        public byte CertAlgorithm { get; set; }

        // 秘密鍵インポート用APDU
        public byte[] PkeyAPDUBytes = Array.Empty<byte>();

        // 証明書インポート用APDU
        public byte[] CertAPDUBytes = Array.Empty<byte>();

        // RSA鍵のバイナリーイメージ
        public byte[] RsaEBytes = Array.Empty<byte>();
        public byte[] RsaPBytes = Array.Empty<byte>();
        public byte[] RsaQBytes = Array.Empty<byte>();
        public byte[] RsaDpBytes = Array.Empty<byte>();
        public byte[] RsaDqBytes = Array.Empty<byte>();
        public byte[] RsaQinvBytes = Array.Empty<byte>();

        // EC鍵のバイナリーイメージ
        public byte[] ECPrivKeyBytes = Array.Empty<byte>();

        // 証明書のバイナリーイメージ
        public byte[] CertBytes = Array.Empty<byte>();

        public PIVImportKeyParameter(byte pkeySlotId)
        {
            PkeySlotId = pkeySlotId;
            PkeyAlgName = string.Empty;
            CertAlgName = string.Empty;
        }
    }

    internal class PIVImportKeyProcess
    {
        // 処理実行のためのプロパティー
        private PIVParameter Parameter = null!;

        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnCommandResponse(bool success, string errorMessage);
        private HandlerOnCommandResponse OnCommandResponse = null!;

        public void DoProcess(PIVParameter parameterRef, HandlerOnCommandResponse handlerRef)
        {
            // パラメーターを保持
            Parameter = parameterRef;

            // コールバックを保持
            OnCommandResponse = handlerRef;

            // 鍵・証明書ファイルを読込み、インポート処理用のリクエストデータを生成
            string errorMessage;
            if (PIVImportKeyRequest.PrepareRequestDataForImport(Parameter, out errorMessage) == false) {
                OnCommandResponse(false, errorMessage);
                return;
            }

            // CCIDインタフェース経由で認証器に接続
            if (CCIDProcess.ConnectCCID() == false) {
                // PIV機能を認識できなかった旨のエラーメッセージを設定し
                // 上位クラスに制御を戻す
                OnCommandResponse(false, AppCommon.MSG_ERROR_PIV_APPLET_SELECT_FAILED);
                return;
            }

            // CCID I/F経由で、PIN番号による認証を実行
            new PIVCCIDProcess().DoRequestPinAuth(Parameter, DoResponsePinAuth);
        }

        private void DoResponsePinAuth(bool success, string errorMessage)
        {
            if (success == false) {
                DoCommandResponse(false, errorMessage);
            }

            // TODO: 仮の実装です。
            AppLogUtil.OutputLogInfo(AppCommon.MSG_PIV_PIN_AUTH_SUCCESS);
            DoCommandResponse(true, AppCommon.MSG_NONE);
        }

        private void DoCommandResponse(bool success, string errorMessage)
        {
            // CCIDデバイスから切断し、上位クラスに制御を戻す
            CCIDProcess.DisconnectCCID();
            OnCommandResponse(success, errorMessage);
        }
    }
}
