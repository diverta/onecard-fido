using System;
using System.IO;
using System.Linq;
using System.Text;
using ToolAppCommon;
using static VendorMaintenanceTool.VendorAppCommon;

namespace VendorMaintenanceTool.FIDOSettings
{
    internal class FIDOAttestationUtil
    {
        //
        // 秘密鍵関連
        //
        public static bool ReadKeyPemFile(string keyPemPath, out byte[] keyPemBytes, out string errorMessage)
        {
            keyPemBytes = Array.Empty<byte>();
            errorMessage = string.Empty;

            // PEMファイルから秘密鍵データを読込
            string pemText;
            if (ReadTextFile(keyPemPath, out pemText) == false) {
                errorMessage = MSG_CANNOT_READ_SKEY_PEM_FILE;
                return false;
            }

            // PEMファイルから読込んだ秘密鍵データを配列に抽出
            if (DecodeB64EncodedString(pemText, out keyPemBytes) == false) {
                errorMessage = MSG_INVALID_SKEY_CONTENT_IN_PEM;
                return false;
            }

            return true;
        }

        public static byte[] ExtractPrivateKeyFromPemBytes(byte[] keyPemBytes)
        {
            // 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、
            // 先頭からビッグエンディアン形式で配置
            byte[] keyBytes = new byte[32];
            for (int i = 0; i < 32; i++) {
                keyBytes[i] = keyPemBytes[7 + i];
            }
            return keyBytes;
        }

        private static bool ReadTextFile(string keyFilePath, out string text)
        {
            bool ret = false;
            text = string.Empty;

            try {
                using (StreamReader sr = new StreamReader(keyFilePath, Encoding.GetEncoding("ASCII"))) {
                    bool concat = false;
                    string line;
                    while ((line = sr.ReadLine()!) != null) {
                        // ヘッダー／フッターは読み飛ばす
                        if (line.Equals("-----BEGIN EC PRIVATE KEY-----")) {
                            // これより後ろの行は連結します
                            concat = true;
                            continue;
                        }
                        if (line.Equals("-----END EC PRIVATE KEY-----")) {
                            break;
                        }
                        if (concat) {
                            text += line;
                        }
                    }
                    ret = (text.Length > 0);
                }

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("ReadLine failed: {0}", e.Message));
            }

            return ret;
        }

        private static bool DecodeB64EncodedString(string strMessage, out byte[] transferMessage)
        {
            bool ret = false;
            transferMessage = Array.Empty<byte>();

            try {
                // non web-safe形式に変換
                string encodedText = strMessage;
                encodedText = encodedText.Replace('_', '/');
                encodedText = encodedText.Replace('-', '+');

                // メッセージをbase64デコード
                transferMessage = Convert.FromBase64String(encodedText);
                ret = (transferMessage.Length > 0);

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("Convert.FromBase64String failed: {0}", e.Message));
            }

            return ret;
        }

        //
        // 証明書関連
        //
        public static bool ReadCertFile(string certFilePath, out byte[] certBytes)
        {
            bool ret = false;
            certBytes = Array.Empty<byte>();

            try {
                certBytes = File.ReadAllBytes(certFilePath);
                ret = (certBytes.Length > 0);

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("File.ReadAllBytes failed: {0}", e.Message));
            }

            return ret;
        }

        public static bool ValidateSkeyCert(byte[] PemBytes, byte[] CertBytes)
        {
            // 証明書から公開鍵を抽出
            byte[] pubkeyFromCert;
            if (ExtractPubkeyFromDer(CertBytes, out pubkeyFromCert) == false) {
                return false;
            }

            // 秘密鍵から公開鍵を生成
            byte[] pubkeyFromPrivkey;
            if (ExtractPubkeyFromDer(PemBytes, out pubkeyFromPrivkey) == false) {
                return false;
            }

            // for debug
            // AppLogUtil.OutputLogDebug("Public key from private key: ");
            // AppLogUtil.OutputLogText(AppLogUtil.DumpMessage(pubkeyFromPrivkey, pubkeyFromPrivkey.Length));
            // AppLogUtil.OutputLogDebug("Public key from certification: ");
            // AppLogUtil.OutputLogText(AppLogUtil.DumpMessage(pubkeyFromCert, pubkeyFromCert.Length));

            // 両者の公開鍵を比較し、同じでない場合はエラー
            return Enumerable.SequenceEqual(pubkeyFromCert, pubkeyFromPrivkey);
        }

        private static bool ExtractPubkeyFromDer(byte[] derBytes, out byte[] pubkeyBytes)
        {
            bool ret = false;
            pubkeyBytes = new byte[64];

            // 開始バイトが不正な場合は終了
            if (derBytes[0] != 0x30) {
                return ret;
            }

            for (int i = 3; i < derBytes.Length; i++) {
                if (derBytes[i - 3] == 0x03 && derBytes[i - 2] == 0x42 &&
                    derBytes[i - 1] == 0x00 && derBytes[i] == 0x04) {
                    // 03 42 00 04 というシーケンスが発見されたら、
                    // その後ろから64バイト分のデータをコピー
                    Array.Copy(derBytes, i + 1, pubkeyBytes, 0, 64);
                    ret = true;
                }
            }

            return ret;
        }
    }
}
