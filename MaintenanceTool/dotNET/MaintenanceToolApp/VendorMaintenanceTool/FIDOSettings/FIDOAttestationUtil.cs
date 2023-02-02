using System;
using System.IO;
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
    }
}
