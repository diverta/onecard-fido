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
                AppUtil.OutputLogError(string.Format(AppCommon.MSG_ERROR_PIV_CERT_INFO_GET_FAILED, ""));
                return false;
            }

            return true;
        }

        private bool ExtractCertFromTLV(byte[] certData)
        {
            return true;
        }

        private bool ExtractCertDescriptions(byte[] certData)
        {
            return true;
        }
    }
}
