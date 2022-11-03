using System;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using ToolAppCommon;

namespace MaintenanceToolApp.PIV
{
    internal class PIVStatusCertData
    {
        // 証明書データ
        public string AlgName { get; set; }
        public string NotAfter { get; set; }
        public string Subject { get; set; }
        public string Issuer { get; set; }
        public string PrintableHash { get; set; }

        // 証明書バイナリーデータ格納領域
        public byte[] CertDataBytes { get; set; }
        public int CertDataLength { get; set; }

        public PIVStatusCertData()
        {
            CertDataBytes = Array.Empty<byte>();
            CertDataLength = 0;
            AlgName = String.Empty;
            NotAfter = String.Empty;
            Subject = String.Empty;
            Issuer = String.Empty;
            PrintableHash = String.Empty;
        }
    }

    internal class PIVStatusWindowUtility
    {
        public static string EditDescriptionString(PIVParameter parameter, string readerName)
        {
            // パラメーターから設定値を取得
            PIVSettingDataObjects settings = parameter.PIVSettings;
            byte retries = parameter.Retries;

            // 変数を初期化
            string StatusInfoString = "";
            string CRLF = "\r\n";
            PIVStatusCertData certData = new PIVStatusCertData();

            // 画面表示される文字列を編集
            StatusInfoString += string.Format("Device: {0}", readerName) + CRLF + CRLF;
            StatusInfoString += string.Format("CHUID:  {0}", PrintableCHUIDString(settings)) + CRLF;
            StatusInfoString += string.Format("CCC:    {0}", PrintableCCCString(settings)) + CRLF + CRLF;
            StatusInfoString += EditCertDescription(PIVConst.PIV_OBJ_AUTHENTICATION, "PIV authenticate", settings, certData);
            StatusInfoString += string.Format("PIN tries left: {0}", retries);
            return StatusInfoString;
        }

        private static string PrintableCHUIDString(PIVSettingDataObjects settings)
        {
            byte[] chuid = settings.Get(PIVConst.PIV_OBJ_CHUID);
            return PrintableObjectStringWithData(chuid);
        }

        private static string PrintableCCCString(PIVSettingDataObjects settings)
        {
            byte[] ccc = settings.Get(PIVConst.PIV_OBJ_CAPABILITY);
            return PrintableObjectStringWithData(ccc);
        }

        private static string PrintableObjectStringWithData(byte[] data)
        {
            // ブランクデータの場合
            if (data == null || data.Length == 0) {
                return "No data available";
            }

            // オブジェクトの先頭２バイト（＝TLVタグ）は不要なので削除
            int offset = 2;
            int size = data.Length - offset;

            // データオブジェクトを、表示可能なHEX文字列に変換
            string hex = "";
            for (int i = 0; i < size; i++) {
                hex += string.Format("{0:x2}", data[i + offset]);
            }
            return hex;
        }

        //
        // 証明書内容の表示
        // 
        private static string EditCertDescription(UInt32 objectId, string objectName, PIVSettingDataObjects settings, PIVStatusCertData certData)
        {
            string description;
            string CRLF = "\r\n";

            // 指定IDのPIVデータオブジェクトを抽出
            if (ExtractCertDescription(objectId, settings, certData)) {
                // データ種類名、アルゴリズム、有効期限、発行先／元、SHA-256ハッシュを表示
                description = string.Format("Slot for {0} ({1}, not after {2})", objectName, certData.AlgName, certData.NotAfter) + CRLF;
                description += string.Format("  Subject:  {0}", certData.Subject) + CRLF;
                description += string.Format("  Issuer:   {0}", certData.Issuer) + CRLF;
                description += string.Format("  SHA-256:  {0}", certData.PrintableHash) + CRLF + CRLF;

            } else {
                // データ表示不可
                description = string.Format("Slot for {0} (Not available)", objectName) + CRLF;
            }

            return description;
        }

        private static bool ExtractCertDescription(UInt32 objectId, PIVSettingDataObjects settings, PIVStatusCertData certData)
        {
            // 指定IDのPIVデータオブジェクトを抽出
            byte[] objectData = settings.Get(objectId);
            if (objectData == null || objectData.Length == 0) {
                return false;
            }

            // PIVデータオブジェクトのデータ本体をTLVから抽出
            string errorMessage;
            if (ExtractCertFromTLV(objectData, certData, out errorMessage) == false) {
                AppLogUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_CERT_INFO_GET_FAILED, errorMessage));
                return false;
            }

            // 必要な属性をPIVデータオブジェクトから抽出
            if (ExtractCertDescriptions(certData, out errorMessage) == false) {
                AppLogUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_CERT_INFO_GET_FAILED, errorMessage));
                return false;
            }

            return true;
        }

        private static bool ExtractCertFromTLV(byte[] objectData, PIVStatusCertData certData, out string errorMessage)
        {
            //
            // 証明書データを格納しているTLVから、証明書データだけを抽出します。
            //   TLV: 538203957082038cXXXX...XXXX710100fe00 (921 bytes)
            //   --> TLV data: 7082038cXXXX...XXXX710100fe00
            //       TLV size: 0x0395 (917 bytes)
            //   --> cert data: XXXX...XXXX
            //       cert size: 0x038c (908 bytes)
            //
            // 証明書データを格納しているTLVを抽出
            int length;
            int offset = 1;
            int size = objectData.Length;
            int tlv_size = size - 1;
            int offset_obj = TlvGetLength(objectData, offset, tlv_size, out length);
            if (offset_obj == 0) {
                // 不正なTLVの場合は終了
                errorMessage = "ExtractCertFromTLV: Invalid TLV (object length)";
                return false;
            }

            // 証明書データを抽出
            offset += offset_obj + 1;
            int offset_val = TlvGetLength(objectData, offset, length, out length);
            if (offset_val == 0) {
                // 不正なTLVの場合は終了
                errorMessage = "ExtractCertFromTLV: Invalid TLV (object value length)";
                return false;
            }

            // 証明書本体の先頭位置
            offset += offset_val;

            // 領域を確保し、抽出した証明書データを保持
            certData.CertDataBytes = new byte[length];
            Array.Copy(objectData, offset, certData.CertDataBytes, 0, length);
            errorMessage = AppCommon.MSG_NONE;
            return true;
        }

        private static int TlvGetLength(byte[] buffer, int offset, int size, out int obj_len)
        {
            // 不正なTLVの場合、戻り値を０に設定します
            if (buffer[offset + 0] < 0x80 && 1 < size) {
                obj_len = buffer[offset + 0];
                return (1 + obj_len <= size) ? 1 : 0;
            } else if (buffer[offset + 0] == 0x81 && 2 < size) {
                obj_len = buffer[offset + 1];
                return (2 + obj_len <= size) ? 2 : 0;
            } else if (buffer[offset + 0] == 0x82 && 3 < size) {
                obj_len = ((buffer[offset + 1] << 8) & 0xff00) + (buffer[offset + 2] & 0x00ff);
                return (3 + obj_len <= size) ? 3 : 0;
            } else {
                obj_len = 0;
                return 0;
            }
        }

        private static bool ExtractCertDescriptions(PIVStatusCertData certData, out string errorMessage)
        {
            bool ret = false;
            errorMessage = AppCommon.MSG_NONE;
            try {
                // 証明書のSHA-256ハッシュを取得
                SHA256 sha = SHA256.Create();
                byte[] h = sha.ComputeHash(certData.CertDataBytes);
                certData.PrintableHash = PrintableHashString(h);

                // 証明書の発行者／発行先／期限を取得
                X509Certificate2 x509 = new X509Certificate2(certData.CertDataBytes);
                certData.Subject = x509.Subject;
                certData.Issuer = x509.Issuer;
                certData.NotAfter = x509.GetExpirationDateString();

                // 証明書アルゴリズム名を取得
                string friendlyName = x509.PublicKey.Oid.FriendlyName!;
                if (friendlyName != null) {
                    if (friendlyName.Equals("RSA")) {
                        certData.AlgName = PIVImportKeyConst.ALG_NAME_RSA2048;
                        ret = true;
                    } else if (friendlyName.Equals("ECC")) {
                        certData.AlgName = PIVImportKeyConst.ALG_NAME_ECCP256;
                        ret = true;
                    } else {
                        errorMessage = "Friendly name of public key is unknown";
                    }

                } else {
                    errorMessage = "Friendly name of public key is null";
                }

            } catch (Exception e) {
                errorMessage = e.Message;
            }

            return ret;
        }

        private static string PrintableHashString(byte[] data)
        {
            // データオブジェクトを、表示可能なHEX文字列に変換
            string hex = "";
            for (int i = 0; i < data.Length; i++) {
                hex += string.Format("{0:x2}", data[i]);
            }
            return hex;
        }
    }
}
