using System;
using System.IO;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    class ToolPIVPkeyCert
    {
        // 鍵・証明書のアルゴリズムを保持
        public string PkeyAlgName { get; set; }
        public string CertAlgName { get; set; }

        // EC鍵のバイナリーイメージ
        public byte[] ECPrivKeyBytes = null;

        // 証明書のバイナリーイメージ
        public byte[] CertBytes = null;

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
                if (ParseECPrivateKey(privateKeyPem) == false) {
                    return false;
                }
            } else {
                PkeyAlgName = "";
                return false;
            }

            return true;
        }

        public bool ParseECPrivateKey(string privateKeyPem)
        {
            // 秘密鍵を抽出（Base64エンコード文字列）
            string ecKeyPem = "";
            bool found = false;
            foreach (string line in privateKeyPem.Split('\n')) {
                if (found) {
                    if (line.Contains("-----END EC PRIVATE KEY-----")) {
                        break;
                    } else {
                        ecKeyPem += line;
                    }
                } else if (line.Contains("-----BEGIN EC PRIVATE KEY-----")) {
                    found = true;
                }
            }

            // 秘密鍵が抽出できなかった場合はエラー
            if (ecKeyPem.Length == 0) {
                AppUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_PKEY_PEM_LOAD_FAILED, "EC private key not found in PEM file"));
                return false;
            }

            try {
                // 秘密鍵情報をデコード
                byte[] decodedBytes = Convert.FromBase64String(ecKeyPem);

                // デコードされた秘密鍵情報の８バイト目から32バイトを抽出
                //   スキップする７バイト＝ 30 77 02 01 01 04 20
                ECPrivKeyBytes = decodedBytes.Skip(7).Take(32).ToArray();
                return true;

            } catch (Exception e) {
                AppUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_PKEY_PEM_LOAD_FAILED, e.Message));
                return false;
            }
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

            // 証明書のバイナリーイメージを抽出
            CertBytes = x509.GetRawCertData();
            return true;
        }
    }
}
