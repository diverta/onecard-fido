using System;
using System.IO;
using System.Text;
using MaintenanceToolCommon;

namespace MaintenanceToolGUI
{
    internal class InstallSkeyCert
    {
        // 秘密鍵格納領域
        private byte[] SkeyBytes = new byte[32];
        // 証明書格納領域
        private byte[] CertBytes = null;

        public bool ReadPemFile(string skeyFilePath)
        {
            string pemText = ReadTextFile(skeyFilePath);
            if (pemText.Length == 0) {
                return false;
            }

            byte[] pemBytes = DecodeB64EncodedString(pemText);
            if (pemBytes == null) {
                return false;
            }

            // 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、
            // 先頭からビッグエンディアン形式で配置
            for (int i = 0; i < 32; i++) {
                SkeyBytes[i] = pemBytes[7 + i];
            }

            return true;
        }

        private string ReadTextFile(string skeyFilePath)
        {
            bool concat = false;
            string text = "";
            try {
                string line;
                using (StreamReader sr = new StreamReader(
                    skeyFilePath, Encoding.GetEncoding("Shift_JIS"))) {
                    while ((line = sr.ReadLine()) != null) {
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
                }
            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("ReadLine failed: {0}", e.Message));
            }
            return text;
        }

        private byte[] DecodeB64EncodedString(string strMessage)
        {
            try {
                // non web-safe形式に変換
                string encodedText = strMessage;
                encodedText = encodedText.Replace('_', '/');
                encodedText = encodedText.Replace('-', '+');

                // メッセージをbase64デコード
                byte[] transferMessage = Convert.FromBase64String(encodedText);
                return transferMessage;

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("Convert.FromBase64String failed: {0}", e.Message));
            }
            return null;
        }

        public bool ReadCertFile(string certFilePath)
        {
            try {
                CertBytes = File.ReadAllBytes(certFilePath);
                return true;

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("File.ReadAllBytes failed: {0}", e.Message));
            }

            return false;
        }

        public int GenerateInstallSkeyCertBytes(byte[] RequestData)
        {
            int pos = 0;

            // リクエストデータを配列にセット
            //   秘密鍵（32バイト）
            Array.Copy(SkeyBytes, 0, RequestData, pos, SkeyBytes.Length);
            pos += SkeyBytes.Length;
            //   証明書（ファイルサイズと同じ）
            Array.Copy(CertBytes, 0, RequestData, pos, CertBytes.Length);
            pos += CertBytes.Length;

            // 配列にセットされたバイト数を戻す
            return pos;
        }
    }
}
