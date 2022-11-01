using System;
using System.IO;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using ToolAppCommon;

namespace MaintenanceToolApp.PIV
{
    internal class PIVImportKeyLoader
    {
        public static bool LoadPrivateKey(string pkeyPath, PIVImportKeyParameter parameter)
        {
            // 変数初期化
            byte PkeyAlgorithm;
            string PkeyAlgName;

            // PEMファイルから秘密鍵を読込
            string privateKeyPem = File.ReadAllText(pkeyPath);

            // 秘密鍵アルゴリズム名を取得
            if (privateKeyPem.Contains("RSA PRIVATE KEY")) {
                PkeyAlgName = PIVImportKeyConst.ALG_NAME_RSA2048;
                if (ParseRSAPrivateKey(privateKeyPem, parameter) == false) {
                    return false;
                }
            } else if (privateKeyPem.Contains("EC PRIVATE KEY")) {
                PkeyAlgName = PIVImportKeyConst.ALG_NAME_ECCP256;
                if (ParseECPrivateKey(privateKeyPem, parameter) == false) {
                    return false;
                }
            } else {
                return false;
            }

            // 鍵アルゴリズムを取得
            if (PkeyAlgName.Equals(PIVImportKeyConst.ALG_NAME_RSA2048)) {
                PkeyAlgorithm = PIVImportKeyConst.CRYPTO_ALG_RSA2048;
            } else if (PkeyAlgName.Equals(PIVImportKeyConst.ALG_NAME_ECCP256)) {
                PkeyAlgorithm = PIVImportKeyConst.CRYPTO_ALG_ECCP256;
            } else {
                return false;
            }

            parameter.PkeyAlgName = PkeyAlgName;
            parameter.PkeyAlgorithm = PkeyAlgorithm;
            return true;
        }

        private static bool ParseRSAPrivateKey(string privateKeyPem, PIVImportKeyParameter parameter)
        {
            // 秘密鍵を抽出（Base64エンコード文字列）
            string rsaKeyPem = "";
            bool found = false;
            foreach (string line in privateKeyPem.Split('\n')) {
                if (found) {
                    if (line.Contains("-----END RSA PRIVATE KEY-----")) {
                        break;
                    } else {
                        rsaKeyPem += line;
                    }
                } else if (line.Contains("-----BEGIN RSA PRIVATE KEY-----")) {
                    found = true;
                }
            }

            // 秘密鍵が抽出できなかった場合はエラー
            if (rsaKeyPem.Length == 0) {
                AppLogUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_PKEY_PEM_LOAD_FAILED, "RSA private key not found in PEM file"));
                return false;
            }

            try {
                // 秘密鍵情報をデコード
                byte[] decodedBytes = Convert.FromBase64String(rsaKeyPem);

                // 先頭から解析
                //   スキップする７バイト＝ 30 82 04 a3 02 01 00
                int offset = 7;
                byte tag = 0;
                int length = 0;
                int itemNo = 0;
                while (offset < decodedBytes.Length) {
                    if (length > 0) {
                        if (decodedBytes[offset] == 0x00) {
                            // 項目の先頭が0x00であれば無視
                            offset++;
                            length -= 1;
                        }

                        // 項目のバイト配列を抽出
                        byte[] itemBytes = decodedBytes.Skip(offset).Take(length).ToArray();

                        // 該当する項目に設定
                        switch (itemNo) {
                        case 1:
                            parameter.RsaEBytes = itemBytes;
                            break;
                        case 3:
                            parameter.RsaPBytes = itemBytes;
                            break;
                        case 4:
                            parameter.RsaQBytes = itemBytes;
                            break;
                        case 5:
                            parameter.RsaDpBytes = itemBytes;
                            break;
                        case 6:
                            parameter.RsaDqBytes = itemBytes;
                            break;
                        case 7:
                            parameter.RsaQinvBytes = itemBytes;
                            break;
                        default:
                            break;
                        }

                        // 次の項目に進む
                        offset += length;
                        tag = 0;
                        length = 0;
                        itemNo++;

                    } else if (tag > 0) {
                        // 項目の長さを取得
                        if (decodedBytes[offset] == 0x82) {
                            length = ((decodedBytes[offset + 1] << 8) & 0xff00) + (decodedBytes[offset + 2] & 0x00ff);
                            offset += 3;
                        } else if (decodedBytes[offset] == 0x81) {
                            length = decodedBytes[offset + 1];
                            offset += 2;
                        } else {
                            length = decodedBytes[offset];
                            offset++;
                        }

                    } else if (decodedBytes[offset] == 0x02) {
                        // 項目の種類を取得
                        tag = decodedBytes[offset];
                        offset++;

                    } else {
                        // 次のバイトに進む
                        offset++;
                    }
                }
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_PKEY_PEM_LOAD_FAILED, e.Message));
                return false;
            }
        }

        private static bool ParseECPrivateKey(string privateKeyPem, PIVImportKeyParameter parameter)
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
                AppLogUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_PKEY_PEM_LOAD_FAILED, "EC private key not found in PEM file"));
                return false;
            }

            try {
                // 秘密鍵情報をデコード
                byte[] decodedBytes = Convert.FromBase64String(ecKeyPem);

                // デコードされた秘密鍵情報の８バイト目から32バイトを抽出
                //   スキップする７バイト＝ 30 77 02 01 01 04 20
                parameter.ECPrivKeyBytes = decodedBytes.Skip(7).Take(32).ToArray();
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_PKEY_PEM_LOAD_FAILED, e.Message));
                return false;
            }
        }

        public static bool LoadCertificate(string certPath, PIVImportKeyParameter parameter)
        {
            // 変数初期化
            byte CertAlgorithm;
            string CertAlgName;

            // PEMファイルから証明書を読込
            X509Certificate2 x509 = new X509Certificate2(certPath);

            // 証明書アルゴリズム名を取得
            string? friendlyName = x509.PublicKey.Oid.FriendlyName;
            if (friendlyName == null) {
                return false;
            } else if (friendlyName.Equals("RSA")) {
                CertAlgName = PIVImportKeyConst.ALG_NAME_RSA2048;
            } else if (friendlyName.Equals("ECC")) {
                CertAlgName = PIVImportKeyConst.ALG_NAME_ECCP256;
            } else {
                return false;
            }

            // 証明書アルゴリズムを、コマンドパラメーターに設定
            if (CertAlgName.Equals(PIVImportKeyConst.ALG_NAME_RSA2048)) {
                CertAlgorithm = PIVImportKeyConst.CRYPTO_ALG_RSA2048;
            } else if (CertAlgName.Equals(PIVImportKeyConst.ALG_NAME_ECCP256)) {
                CertAlgorithm = PIVImportKeyConst.CRYPTO_ALG_ECCP256;
            } else {
                return false;
            }

            // 証明書のバイナリーイメージを抽出
            parameter.CertBytes = x509.GetRawCertData();
            parameter.CertAlgName = CertAlgName;
            parameter.CertAlgorithm = CertAlgorithm;
            return true;
        }
    }
}
