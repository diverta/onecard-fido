﻿using System;
using ToolAppCommon;

namespace MaintenanceToolApp.PIV
{
    public class PIVImportKeyParameter
    {
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

        public PIVImportKeyParameter()
        {
            PkeyAlgName = string.Empty;
            CertAlgName = string.Empty;
        }
    }

    internal class PIVImportKeyProcess
    {
        public static bool PrepareRequestDataForImport(PIVParameter parameter, out string errorMessage) 
        {
            // 鍵・証明書をファイルから読込
            if (DoProcessImportKey(parameter, out errorMessage) == false) {
                return false;
            }

            // TODO:
            // 鍵・証明書インポート用のAPDUを生成

            return true;
        }

        private static bool DoProcessImportKey(PIVParameter parameter, out string errorMessage)
        {
            // スロット１の処理
            parameter.ImportKeyParameter1 = new PIVImportKeyParameter();
            if (DoProcessImportKeySlot(parameter.PkeyFilePath1, parameter.CertFilePath1, parameter.ImportKeyParameter1, out errorMessage) == false) {
                return false;
            }

            // スロット２の処理
            parameter.ImportKeyParameter2 = new PIVImportKeyParameter();
            if (DoProcessImportKeySlot(parameter.PkeyFilePath2, parameter.CertFilePath2, parameter.ImportKeyParameter2, out errorMessage) == false) {
                return false;
            }

            // スロット３の処理
            parameter.ImportKeyParameter3 = new PIVImportKeyParameter();
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

            return true;
        }

        private static bool ReadPrivateKeyPem(string pkeyPemPath, PIVImportKeyParameter parameter, out string errorMessage)
        {
            // エラーメッセージを初期化
            errorMessage = AppCommon.MSG_NONE;

            if (PIVImportKeyUtility.LoadPrivateKey(pkeyPemPath, parameter) == false) {
                errorMessage = AppCommon.MSG_PIV_LOAD_PKEY_FAILED;
                return false;
            }

            return true;
        }

        private static bool ReadCertificatePem(string certPemPath, PIVImportKeyParameter parameter, out string errorMessage)
        {
            // エラーメッセージを初期化
            errorMessage = AppCommon.MSG_NONE;

            if (PIVImportKeyUtility.LoadCertificate(certPemPath, parameter) == false) {
                errorMessage = AppCommon.MSG_PIV_LOAD_CERT_FAILED;
                return false;
            }

            return true;
        }
    }
}
