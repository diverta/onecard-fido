using System;
using System.IO;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public class ToolPIVPkeyCertConst
    {
        public const int RSA2048_PQ_SIZE = 128;
        public const int ECCP256_KEY_SIZE = 32;
    }

    public class ToolPIVPkeyCert
    {
        // 鍵・証明書のアルゴリズムを保持
        public string PkeyAlgName { get; set; }
        public string CertAlgName { get; set; }

        // 秘密鍵インポート用APDU
        public byte[] PkeyAPDUBytes = null;

        // RSA鍵のバイナリーイメージ
        public byte[] RsaEBytes = null;
        public byte[] RsaPBytes = null;
        public byte[] RsaQBytes = null;
        public byte[] RsaDpBytes = null;
        public byte[] RsaDqBytes = null;
        public byte[] RsaQinvBytes = null;

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
                if (ParseRSAPrivateKey(privateKeyPem) == false) {
                    return false;
                }
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

        private bool ParseRSAPrivateKey(string privateKeyPem)
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
                AppUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_PKEY_PEM_LOAD_FAILED, "RSA private key not found in PEM file"));
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
                            RsaEBytes = itemBytes;
                            break;
                        case 3:
                            RsaPBytes = itemBytes;
                            break;
                        case 4:
                            RsaQBytes = itemBytes;
                            break;
                        case 5:
                            RsaDpBytes = itemBytes;
                            break;
                        case 6:
                            RsaDqBytes = itemBytes;
                            break;
                        case 7:
                            RsaQinvBytes = itemBytes;
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
                AppUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_PKEY_PEM_LOAD_FAILED, e.Message));
                return false;
            }
        }

        private bool ParseECPrivateKey(string privateKeyPem)
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

        //
        // APDU生成処理
        //
        public bool GeneratePrivateKeyAPDU()
        {
            if (PkeyAlgName == "RSA2048") {
                GenerateAPDUDataRsa2048();
            } else if (PkeyAlgName == "ECCP256") {
                GenerateAPDUDataEccp256();
            } else {
                return false;
            }
            return true;
        }

        private void GenerateAPDUDataRsa2048()
        {
            // 変数初期化
            int apduSize = (ToolPIVPkeyCertConst.RSA2048_PQ_SIZE + 3) * 5; 
            PkeyAPDUBytes = new byte[apduSize];
            int offset = 0;

            // 項目長（128）を２バイトエンコード
            byte itemSizeTag = 0x81;
            byte itemSize = 0x80;

            // RSA秘密鍵データをTLV形式で設定
            // P
            PkeyAPDUBytes[offset++] = 0x01;
            PkeyAPDUBytes[offset++] = itemSizeTag;
            PkeyAPDUBytes[offset++] = itemSize;
            Array.Copy(RsaPBytes, 0, PkeyAPDUBytes, offset, ToolPIVPkeyCertConst.RSA2048_PQ_SIZE);
            offset += ToolPIVPkeyCertConst.RSA2048_PQ_SIZE;

            // Q
            PkeyAPDUBytes[offset++] = 0x02;
            PkeyAPDUBytes[offset++] = itemSizeTag;
            PkeyAPDUBytes[offset++] = itemSize;
            Array.Copy(RsaQBytes, 0, PkeyAPDUBytes, offset, ToolPIVPkeyCertConst.RSA2048_PQ_SIZE);
            offset += ToolPIVPkeyCertConst.RSA2048_PQ_SIZE;

            // DP
            PkeyAPDUBytes[offset++] = 0x03;
            PkeyAPDUBytes[offset++] = itemSizeTag;
            PkeyAPDUBytes[offset++] = itemSize;
            Array.Copy(RsaDpBytes, 0, PkeyAPDUBytes, offset, ToolPIVPkeyCertConst.RSA2048_PQ_SIZE);
            offset += ToolPIVPkeyCertConst.RSA2048_PQ_SIZE;

            // DQ
            PkeyAPDUBytes[offset++] = 0x04;
            PkeyAPDUBytes[offset++] = itemSizeTag;
            PkeyAPDUBytes[offset++] = itemSize;
            Array.Copy(RsaDqBytes, 0, PkeyAPDUBytes, offset, ToolPIVPkeyCertConst.RSA2048_PQ_SIZE);
            offset += ToolPIVPkeyCertConst.RSA2048_PQ_SIZE;

            // QINV
            PkeyAPDUBytes[offset++] = 0x05;
            PkeyAPDUBytes[offset++] = itemSizeTag;
            PkeyAPDUBytes[offset++] = itemSize;
            Array.Copy(RsaQinvBytes, 0, PkeyAPDUBytes, offset, ToolPIVPkeyCertConst.RSA2048_PQ_SIZE);
            offset += ToolPIVPkeyCertConst.RSA2048_PQ_SIZE;
        }

        private void GenerateAPDUDataEccp256()
        {
            // 変数初期化
            PkeyAPDUBytes = new byte[ToolPIVPkeyCertConst.ECCP256_KEY_SIZE];
            int offset = 0;

            // EC秘密鍵データをTLV形式で設定
            PkeyAPDUBytes[offset++] = 0x06;
            PkeyAPDUBytes[offset++] = ToolPIVPkeyCertConst.ECCP256_KEY_SIZE;
            Array.Copy(ECPrivKeyBytes, 0, PkeyAPDUBytes, offset, ToolPIVPkeyCertConst.ECCP256_KEY_SIZE);
            offset += ToolPIVPkeyCertConst.ECCP256_KEY_SIZE;
        }
    }
}
