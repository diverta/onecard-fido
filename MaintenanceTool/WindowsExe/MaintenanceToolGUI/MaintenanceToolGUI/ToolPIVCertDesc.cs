using System;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    class ToolPIVCertDesc
    {
        // PIV設定情報を保持
        private ToolPIVSettingItem SettingItemRef;

        // 証明書データ
        private byte[] extractedCertData = null;
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
            if (ExtractCertDescriptions(data) == false) {
                AppUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_CERT_INFO_GET_FAILED, ErrorMessage));
                return false;
            }

            return true;
        }

        // 証明書データ格納領域
        byte[] m_binary_data = new byte[3072];

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
            // 領域を初期化
            for (int i = 0; i < m_binary_data.Length; i++) {
                m_binary_data[i] = 0;
            }

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
            return true;
        }
    }
}
