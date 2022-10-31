using System;

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
            if (PrepareRequestDataForImport(Parameter, out errorMessage) == false) {
                OnCommandResponse(false, errorMessage);
                return;
            }

            // CCID I/F経由で、鍵・証明書インポート処理を実行
            new PIVCCIDProcess().DoPIVCcidCommand(Parameter, DoResponsePIVImportKey);
        }

        private void DoResponsePIVImportKey(bool success, string errorMessage)
        {
            // 上位クラスに制御を戻す
            OnCommandResponse(success, errorMessage);
        }

        //
        // リクエストデータ生成処理
        //
        private static bool PrepareRequestDataForImport(PIVParameter parameter, out string errorMessage) 
        {
            // スロット１の処理
            parameter.ImportKeyParameter1 = new PIVImportKeyParameter(PIVConst.PIV_KEY_AUTHENTICATION);
            if (DoProcessImportKeySlot(parameter.PkeyFilePath1, parameter.CertFilePath1, parameter.ImportKeyParameter1, out errorMessage) == false) {
                return false;
            }

            // スロット２の処理
            parameter.ImportKeyParameter2 = new PIVImportKeyParameter(PIVConst.PIV_KEY_SIGNATURE);
            if (DoProcessImportKeySlot(parameter.PkeyFilePath2, parameter.CertFilePath2, parameter.ImportKeyParameter2, out errorMessage) == false) {
                return false;
            }

            // スロット３の処理
            parameter.ImportKeyParameter3 = new PIVImportKeyParameter(PIVConst.PIV_KEY_KEYMGM);
            if (DoProcessImportKeySlot(parameter.PkeyFilePath3, parameter.CertFilePath3, parameter.ImportKeyParameter3, out errorMessage) == false) {
                return false;
            }

            return true;
        }

        private static bool DoProcessImportKeySlot(string pkeyFilePath, string certFilePath, PIVImportKeyParameter importParam, out string errorMessage)
        {
            // 秘密鍵ファイル、証明書ファイルを読込
            if (ReadPrivateKeyPem(pkeyFilePath, importParam, out errorMessage) == false) {
                return false;
            }
            if (ReadCertificatePem(certFilePath, importParam, out errorMessage) == false) {
                return false;
            }
            if (importParam.PkeyAlgorithm != importParam.CertAlgorithm) {
                // 鍵・証明書のアルゴリズムが異なる場合は、エラーメッセージを表示し処理中止
                errorMessage = string.Format(
                    AppCommon.MSG_FORMAT_PIV_PKEY_CERT_ALGORITHM, importParam.PkeyAlgName, importParam.CertAlgName);
                return false;
            }

            // 鍵・証明書インポート用のAPDUを生成
            if (PIVImportKeyUtility.GeneratePrivateKeyAPDU(importParam) == false) {
                return false;
            }
            if (PIVImportKeyUtility.GenerateCertificateAPDU(importParam) == false) {
                return false;
            }

            return true;
        }

        private static bool ReadPrivateKeyPem(string pkeyPemPath, PIVImportKeyParameter parameter, out string errorMessage)
        {
            // エラーメッセージを初期化
            errorMessage = AppCommon.MSG_NONE;

            if (PIVImportKeyLoader.LoadPrivateKey(pkeyPemPath, parameter) == false) {
                errorMessage = AppCommon.MSG_PIV_LOAD_PKEY_FAILED;
                return false;
            }

            return true;
        }

        private static bool ReadCertificatePem(string certPemPath, PIVImportKeyParameter parameter, out string errorMessage)
        {
            // エラーメッセージを初期化
            errorMessage = AppCommon.MSG_NONE;

            if (PIVImportKeyLoader.LoadCertificate(certPemPath, parameter) == false) {
                errorMessage = AppCommon.MSG_PIV_LOAD_CERT_FAILED;
                return false;
            }

            return true;
        }
    }
}
