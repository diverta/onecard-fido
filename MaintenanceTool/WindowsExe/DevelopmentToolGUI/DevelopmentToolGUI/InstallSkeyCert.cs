using System;
using System.IO;
using System.Linq;
using System.Text;
using ToolGUICommon;

namespace DevelopmentToolGUI
{
    internal class InstallSkeyCert
    {
        // 秘密鍵格納領域
        private byte[] SkeyBytes = new byte[32];
        // 証明書格納領域
        private byte[] CertBytes = null;
        // PEMファイルから読込んだ秘密鍵データの格納領域
        private byte[] PemBytes = null;

        public bool ReadPemFile(string skeyFilePath)
        {
            string pemText = ReadTextFile(skeyFilePath);
            if (pemText.Length == 0) {
                return false;
            }

            // PEMファイルから読込んだ秘密鍵データを保持
            PemBytes = DecodeB64EncodedString(pemText);
            if (PemBytes == null) {
                return false;
            }

            // 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、
            // 先頭からビッグエンディアン形式で配置
            for (int i = 0; i < 32; i++) {
                SkeyBytes[i] = PemBytes[7 + i];
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
                AppUtil.OutputLogError(string.Format("ReadLine failed: {0}", e.Message));
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
                AppUtil.OutputLogError(string.Format("Convert.FromBase64String failed: {0}", e.Message));
            }
            return null;
        }

        public bool ReadCertFile(string certFilePath)
        {
            try {
                CertBytes = File.ReadAllBytes(certFilePath);
                return true;

            } catch (Exception e) {
                AppUtil.OutputLogError(string.Format("File.ReadAllBytes failed: {0}", e.Message));
            }

            return false;
        }

        public bool ValidateSkeyCert()
        {
            // 証明書から公開鍵を抽出
            byte[] pubkeyFromCert = ExtractPubkeyFromDer(CertBytes);
            if (pubkeyFromCert == null) {
                return false;
            }

            // 秘密鍵から公開鍵を生成
            byte[] pubkeyFromPrivkey = ExtractPubkeyFromDer(PemBytes);
            if (pubkeyFromPrivkey == null) {
                return false;
            }

            // for debug
            // AppUtil.OutputLogDebug("Public key from certification: ");
            // AppUtil.OutputLogText(AppCommon.DumpMessage(pubkeyFromCert, pubkeyFromCert.Length));
            // AppUtil.OutputLogDebug("Public key from private key: ");
            // AppUtil.OutputLogText(AppCommon.DumpMessage(pubkeyFromPrivkey, pubkeyFromPrivkey.Length));

            // 両者の公開鍵を比較し、同じでない場合はエラー
            if (Enumerable.SequenceEqual(pubkeyFromCert, pubkeyFromPrivkey) == false) {
                return false;
            }

            return true;
        }

        private byte[] ExtractPubkeyFromDer(byte[] derBytes)
        {
            // 開始バイトが不正な場合は終了
            if (derBytes[0] != 0x30) {
                return null;
            }

            for (int i = 3; i < derBytes.Length; i++) {
                if (derBytes[i - 3] == 0x03 && derBytes[i - 2] == 0x42 &&
                    derBytes[i - 1] == 0x00 && derBytes[i] == 0x04) {
                    // 03 42 00 04 というシーケンスが発見されたら、
                    // その後ろから64バイト分のデータをコピー
                    byte[] pubkeyBytes = new byte[64];
                    Array.Copy(derBytes, i + 1, pubkeyBytes, 0, 64);
                    return pubkeyBytes;
                }
            }

            return null;
        }

        public byte[] GenerateInstallSkeyCertBytes()
        {
            // 配列サイズ
            int skeyCertBytesSize = SkeyBytes.Length + CertBytes.Length;

            // 初期化
            int pos = 0;
            byte[] skeyCertBytes = new byte[skeyCertBytesSize];

            // 鍵・証明書バイナリーデータを、１本のバイトデータにマージ
            //   秘密鍵（32バイト）
            Array.Copy(SkeyBytes, 0, skeyCertBytes, pos, SkeyBytes.Length);
            pos += SkeyBytes.Length;
            //   証明書（ファイルサイズと同じ）
            Array.Copy(CertBytes, 0, skeyCertBytes, pos, CertBytes.Length);
            pos += CertBytes.Length;

            // マージされたバイトデータを戻す
            return skeyCertBytes;
        }
    }
}
