using System;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    class ToolPIVCertDesc
    {
        // PIV設定情報を保持
        private ToolPIVSettingItem SettingItemRef;

        // 証明書バイナリーデータ格納領域
        byte[] CertDataBytes = null;

        // 証明書データ
        private string AlgName;
        private string NotAfter;
        private string Subject;
        private string Issuer;
        private string PrintableHash;

        // エラーメッセージを保持
        private string ErrorMessage = AppCommon.MSG_OCCUR_UNKNOWN_ERROR;

        public ToolPIVCertDesc(ToolPIVSettingItem item)
        {
            // PIV設定情報を保持
            SettingItemRef = item;
        }

        public string EditCertDescription(UInt32 objectId, string objectName)
        {
            string description = null;
            string CRLF = "\r\n";

            // 指定IDのPIVデータオブジェクトを抽出
            if (ExtractCertDescription(objectId)) {
                // データ種類名、アルゴリズム、有効期限、発行先／元、SHA-256ハッシュを表示
                description = string.Format("Slot for {0} ({1}, not after {2})", objectName, AlgName, NotAfter) + CRLF;
                description += string.Format("  Subject:  {0}", Subject) + CRLF;
                description += string.Format("  Issuer:   {0}", Issuer) + CRLF;
                description += string.Format("  SHA-256:  {0}", PrintableHash) + CRLF + CRLF;

            } else {
                // データ表示不可
                description = string.Format("Slot for {0} (Not available)", objectName) + CRLF;
            }

            return description;
        }

        private bool ExtractCertDescription(UInt32 objectId)
        {
            // 指定IDのPIVデータオブジェクトを抽出
            byte[] data = SettingItemRef.GetDataObject(objectId);
            if (data == null || data.Length == 0) {
                return false;
            }

            // PIVデータオブジェクトのデータ本体をTLVから抽出
            if (ExtractCertFromTLV(data) == false) {
                return false;
            }

            // 必要な属性をPIVデータオブジェクトから抽出
            if (ExtractCertDescriptions(CertDataBytes) == false) {
                AppUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_CERT_INFO_GET_FAILED, ErrorMessage));
                return false;
            }

            return true;
        }

        private bool ExtractCertFromTLV(byte[] certData)
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
            int offset = 1;
            int size = certData.Length;
            int tlv_size = size - 1;
            int offset_obj = tlv_get_length(certData, offset, tlv_size);
            if (offset_obj == 0) {
                // 不正なTLVの場合は終了
                ErrorMessage = "ExtractCertFromTLV: Invalid TLV (object length)";
                return false;
            }

            // 証明書データを抽出
            offset += offset_obj + 1;
            int offset_val = tlv_get_length(certData, offset, obj_len);
            if (offset_val == 0) {
                // 不正なTLVの場合は終了
                ErrorMessage = "ExtractCertFromTLV: Invalid TLV (object value length)";
                return false;
            }

            // 証明書本体の先頭位置
            offset += offset_val;

            // 領域を確保し、抽出した証明書データを保持
            CertDataBytes = new byte[obj_len];
            Array.Copy(certData, offset, CertDataBytes, 0, obj_len);
            return true;
        }

        int obj_len = 0;
        private int tlv_get_length(byte[] buffer, int offset, int size)
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

        private bool ExtractCertDescriptions(byte[] certData)
        {
            try {
                // 証明書のSHA-256ハッシュを取得
                SHA256 sha = new SHA256CryptoServiceProvider();
                byte[] h = sha.ComputeHash(certData);
                PrintableHash = PrintableHashString(h);

                // 証明書の発行者／発行先／期限を取得
                X509Certificate2 x509 = new X509Certificate2(certData);
                Subject = x509.Subject;
                Issuer = x509.Issuer;
                NotAfter = x509.GetExpirationDateString();

                // 証明書アルゴリズム名を取得
                string friendlyName = x509.PublicKey.Oid.FriendlyName;
                if (friendlyName.Equals("RSA")) {
                    AlgName = ToolPIVConst.ALG_NAME_RSA2048;
                }
                if (friendlyName.Equals("ECC")) {
                    AlgName = ToolPIVConst.ALG_NAME_ECCP256;
                }

                return true;

            } catch (Exception e) {
                ErrorMessage = e.Message;
                return false;
            }
        }

        public string PrintableHashString(byte[] data)
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
