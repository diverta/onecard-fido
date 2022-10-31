using System;

namespace MaintenanceToolApp.PIV
{
    public class PIVImportKeyParameter
    {
        // 鍵・証明書のアルゴリズムを保持
        public string PkeyAlgName { get; set; }
        public string CertAlgName { get; set; }

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
    }
}
