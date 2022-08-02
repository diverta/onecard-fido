using System.IO;
using System.Security.Cryptography.X509Certificates;

namespace MaintenanceToolGUI
{
    class ToolPIVPkeyCert
    {
        // 鍵・証明書のアルゴリズムを保持
        public string PkeyAlgName { get; set; }
        public string CertAlgName { get; set; }

        public ToolPIVPkeyCert()
        {
        }

        public bool LoadPrivateKey(string pkeyPath)
        {
            // PEMファイルから秘密鍵を読込
            string privateKeyPem = File.ReadAllText(pkeyPath);

            // 秘密鍵アルゴリズム名を取得
            if (privateKeyPem.Contains("RSA PRIVATE KEY")) {
                PkeyAlgName = "RSA2048";
            } else if (privateKeyPem.Contains("EC PRIVATE KEY")) {
                PkeyAlgName = "ECCP256";
            } else {
                PkeyAlgName = "";
                return false;
            }

            return true;
        }

        public bool LoadCertificate(string certPath)
        {
            // PEMファイルから証明書を読込
            X509Certificate2 x509 = new X509Certificate2(certPath);

            // 証明書アルゴリズム名を取得
            string friendlyName = x509.PublicKey.Oid.FriendlyName;
            if (friendlyName.Equals("RSA")) {
                CertAlgName = "RSA2048";
            } else if (friendlyName.Equals("ECC")) {
                CertAlgName = "ECCP256";
            } else {
                CertAlgName = "";
                return false;
            }

            return true;
        }
    }
}
