using PeterO.Cbor;
using System;

namespace MaintenanceToolGUI
{
    internal class BLESMPResultInfo
    {
        public byte Rc { get; set; }
        public UInt32 Off { get; set; }

        public BLESMPResultInfo()
        {
        }
    }

    internal class BLESMPSlotInfo
    {
        public byte SlotNo { get; set; }
        public byte[] Hash { get; set; }
        public bool Active { get; set; }

        public BLESMPSlotInfo()
        {
        }
    }

    internal class BLESMPCBORDecoder
    {
        public BLESMPResultInfo ResultInfo { get; set; }
        public BLESMPSlotInfo[] SlotInfos { get; set; }

        public BLESMPCBORDecoder()
        {
            // スロット照会情報を初期化
            SlotInfos = new BLESMPSlotInfo[2];
            for (int i = 0; i < SlotInfos.Length; i++) {
                SlotInfos[i] = new BLESMPSlotInfo();
            }

            // 転送結果情報を初期化
            ResultInfo = new BLESMPResultInfo();
        }

        public bool DecodeSlotInfo(byte[] cborBytes)
        {
            // ルートのMapを抽出
            CBORObject slotInfoMap = CBORObject.DecodeFromBytes(cborBytes, CBOREncodeOptions.Default);

            // Map内を探索
            foreach (CBORObject slotInfoKey in slotInfoMap.Keys) {
                string keyStr = slotInfoKey.AsString();
                if (keyStr.Equals("images")) {
                    // "images"エントリーを抽出（配列）
                    if (ParseImageArray(slotInfoMap, slotInfoKey) == false) {
                        return false;
                    }
                }

                if (keyStr.Equals("rc")) {
                    // "images"がない場合は、代わりに"rc"を抽出
                    byte rc = 0;
                    if (ParseByteValue(slotInfoMap, slotInfoKey, ref rc) == false) {
                        return false;
                    }
                    ResultInfo.Rc = rc;
                }
            }
            return true;
        }

        public bool DecodeUploadResultInfo(byte[] cborBytes)
        {
            // ルートのMapを抽出
            CBORObject uploadResultInfoMap = CBORObject.DecodeFromBytes(cborBytes, CBOREncodeOptions.Default);

            // Map内を探索
            foreach (CBORObject uploadResultInfoKey in uploadResultInfoMap.Keys) {
                string keyStr = uploadResultInfoKey.AsString();
                if (keyStr.Equals("rc")) {
                    // "rc"エントリーを抽出（数値）
                    byte rc = 0;
                    if (ParseByteValue(uploadResultInfoMap, uploadResultInfoKey, ref rc) == false) {
                        return false;
                    }
                    ResultInfo.Rc = rc;
                }
                if (keyStr.Equals("off")) {
                    // "off"エントリーを抽出（数値）
                    UInt32 off = 0;
                    if (ParseUInt32Value(uploadResultInfoMap, uploadResultInfoKey, ref off) == false) {
                        return false;
                    }
                    ResultInfo.Off = off;
                }
            }
            return true;
        }

        private bool ParseImageArray(CBORObject map, CBORObject key)
        {
            // Mapから指定キーのエントリーを抽出
            CBORObject imageArray = map[key];
            if (imageArray == null) {
                AppCommon.OutputLogError(string.Format("ParseImageArray: {0} is null", key.AsString()));
                return false;
            }

            // 型をチェック
            if (imageArray.Type != CBORType.Array) {
                AppCommon.OutputLogError(string.Format("ParseImageArray: {0} is not CBORType.Array", key.AsString()));
                return false;
            }

            // 配列内を探索
            int idx = 0;
            foreach (CBORObject imageMap in imageArray.AsList()) {
                // 型をチェック
                if (imageMap.Type != CBORType.Map) {
                    AppCommon.OutputLogError(string.Format("ParseImageArray: idx[{0}] is not CBORType.Map", idx));
                    return false;
                }

                // 抽出する値を格納
                byte slotNo = 0;
                byte[] hash = null;
                bool active = false;

                // Map内を探索
                foreach (CBORObject imageKey in imageMap.Keys) {
                    string imageKeyStr = imageKey.AsString();
                    if (imageKeyStr.Equals("slot")) {
                        // "slot"エントリーを抽出（数値）
                        if (ParseByteValue(imageMap, imageKey, ref slotNo) == false) {
                            return false;
                        }
                        SlotInfos[slotNo].SlotNo = slotNo;
                    }

                    if (imageKeyStr.Equals("hash")) {
                        // "hash"エントリーを抽出（バイト配列）
                        if (ParseFixedBytesValue(imageMap, imageKey, ref hash) == false) {
                            return false;
                        }
                        SlotInfos[slotNo].Hash = hash;
                    }

                    if (imageKeyStr.Equals("active")) {
                        // "active"エントリーを抽出（bool）
                        if (ParseBooleanValue(imageMap, imageKey, ref active) == false) {
                            return false;
                        }
                        SlotInfos[slotNo].Active = active;
                    }
                }
                idx++;
            }

            return true;
        }

        private bool ParseByteValue(CBORObject map, CBORObject key, ref byte b)
        {
            // Mapから指定キーのエントリーを抽出
            CBORObject value = map[key];
            if (value == null) {
                AppCommon.OutputLogError(string.Format("ParseByteValue: {0} is null", key.AsString()));
                return false;
            }

            // 型をチェック
            if (value.Type != CBORType.Integer) {
                AppCommon.OutputLogError(string.Format("ParseByteValue: {0} is not CBORType.Number", key.AsString()));
                return false;
            }

            // 値を抽出
            b = value.AsByte();
            return true;
        }

        private bool ParseFixedBytesValue(CBORObject map, CBORObject key, ref byte[] b)
        {
            // Mapから指定キーのエントリーを抽出
            CBORObject value = map[key];
            if (value == null) {
                AppCommon.OutputLogError(string.Format("ParseFixedBytesValue: {0} is null", key.AsString()));
                return false;
            }

            // 型をチェック
            if (value.Type != CBORType.ByteString) {
                AppCommon.OutputLogError(string.Format("ParseFixedBytesValue: {0} is not CBORType.ByteString", key.AsString()));
                return false;
            }

            // 値を抽出
            b = value.GetByteString();
            return true;
        }

        private bool ParseBooleanValue(CBORObject map, CBORObject key, ref bool b)
        {
            // Mapから指定キーのエントリーを抽出
            CBORObject value = map[key];
            if (value == null) {
                AppCommon.OutputLogError(string.Format("ParseBooleanValue: {0} is null", key.AsString()));
                return false;
            }

            // 型をチェック
            if (value.Type != CBORType.Boolean) {
                AppCommon.OutputLogError(string.Format("ParseBooleanValue: {0} is not CBORType.Boolean", key.AsString()));
                return false;
            }

            // 値を抽出
            b = value.AsBoolean();
            return true;
        }

        private bool ParseUInt32Value(CBORObject map, CBORObject key, ref UInt32 ui)
        {
            // Mapから指定キーのエントリーを抽出
            CBORObject value = map[key];
            if (value == null) {
                AppCommon.OutputLogError(string.Format("ParseUInt32Value: {0} is null", key.AsString()));
                return false;
            }

            // 型をチェック
            if (value.Type != CBORType.Integer) {
                AppCommon.OutputLogError(string.Format("ParseUInt32Value: {0} is not CBORType.Number", key.AsString()));
                return false;
            }

            // 値を抽出
            ui = value.AsUInt32();
            return true;
        }
    }
}
