using System;

namespace MaintenanceToolApp.PIV
{
    internal class PIVImportKeyConst
    {
        public const string ALG_NAME_RSA2048 = "RSA2048";
        public const string ALG_NAME_ECCP256 = "ECCP256";
        public const byte CRYPTO_ALG_RSA2048 = 0x07;
        public const byte CRYPTO_ALG_ECCP256 = 0x11;
        public const int RSA2048_PQ_SIZE = 128;
        public const int ECCP256_KEY_SIZE = 32;
    }

    internal class PIVImportKeyUtility
    {
        //
        // APDU生成処理
        //
        public static bool GeneratePrivateKeyAPDU(PIVImportKeyParameter parameter)
        {
            if (parameter.PkeyAlgName == PIVImportKeyConst.ALG_NAME_RSA2048) {
                GenerateAPDUDataRsa2048(parameter);
            } else if (parameter.PkeyAlgName == PIVImportKeyConst.ALG_NAME_ECCP256) {
                GenerateAPDUDataEccp256(parameter);
            } else {
                return false;
            }

            return true;
        }

        private static void GenerateAPDUDataRsa2048(PIVImportKeyParameter parameter)
        {
            // 変数初期化
            int apduSize = (PIVImportKeyConst.RSA2048_PQ_SIZE + 3) * 5;
            byte[] PkeyAPDUBytes = new byte[apduSize];
            int offset = 0;

            // 項目長（128）を２バイトエンコード
            byte itemSizeTag = 0x81;
            byte itemSize = 0x80;

            // RSA秘密鍵データをTLV形式で設定
            // P
            PkeyAPDUBytes[offset++] = 0x01;
            PkeyAPDUBytes[offset++] = itemSizeTag;
            PkeyAPDUBytes[offset++] = itemSize;
            Array.Copy(parameter.RsaPBytes, 0, PkeyAPDUBytes, offset, PIVImportKeyConst.RSA2048_PQ_SIZE);
            offset += PIVImportKeyConst.RSA2048_PQ_SIZE;

            // Q
            PkeyAPDUBytes[offset++] = 0x02;
            PkeyAPDUBytes[offset++] = itemSizeTag;
            PkeyAPDUBytes[offset++] = itemSize;
            Array.Copy(parameter.RsaQBytes, 0, PkeyAPDUBytes, offset, PIVImportKeyConst.RSA2048_PQ_SIZE);
            offset += PIVImportKeyConst.RSA2048_PQ_SIZE;

            // DP
            PkeyAPDUBytes[offset++] = 0x03;
            PkeyAPDUBytes[offset++] = itemSizeTag;
            PkeyAPDUBytes[offset++] = itemSize;
            Array.Copy(parameter.RsaDpBytes, 0, PkeyAPDUBytes, offset, PIVImportKeyConst.RSA2048_PQ_SIZE);
            offset += PIVImportKeyConst.RSA2048_PQ_SIZE;

            // DQ
            PkeyAPDUBytes[offset++] = 0x04;
            PkeyAPDUBytes[offset++] = itemSizeTag;
            PkeyAPDUBytes[offset++] = itemSize;
            Array.Copy(parameter.RsaDqBytes, 0, PkeyAPDUBytes, offset, PIVImportKeyConst.RSA2048_PQ_SIZE);
            offset += PIVImportKeyConst.RSA2048_PQ_SIZE;

            // QINV
            PkeyAPDUBytes[offset++] = 0x05;
            PkeyAPDUBytes[offset++] = itemSizeTag;
            PkeyAPDUBytes[offset++] = itemSize;
            Array.Copy(parameter.RsaQinvBytes, 0, PkeyAPDUBytes, offset, PIVImportKeyConst.RSA2048_PQ_SIZE);
            offset += PIVImportKeyConst.RSA2048_PQ_SIZE;

            // 生成されたAPDUを、コマンドパラメーターに設定
            parameter.PkeyAPDUBytes = PkeyAPDUBytes;
        }

        private static void GenerateAPDUDataEccp256(PIVImportKeyParameter parameter)
        {
            // 変数初期化
            parameter.PkeyAPDUBytes = new byte[PIVImportKeyConst.ECCP256_KEY_SIZE + 2];
            int offset = 0;

            // EC秘密鍵データをTLV形式で設定
            parameter.PkeyAPDUBytes[offset++] = 0x06;
            parameter.PkeyAPDUBytes[offset++] = PIVImportKeyConst.ECCP256_KEY_SIZE;
            Array.Copy(parameter.ECPrivKeyBytes, 0, parameter.PkeyAPDUBytes, offset, PIVImportKeyConst.ECCP256_KEY_SIZE);
            offset += PIVImportKeyConst.ECCP256_KEY_SIZE;
        }

        public static bool GenerateCertificateAPDU(PIVImportKeyParameter parameter)
        {
            // 変数初期化
            int certBytesSize = parameter.CertBytes.Length;
            byte[] CertAPDUBytes = new byte[certBytesSize + 18];
            int offset = 0;

            // スロットIDからオブジェクトIDを取得
            UInt32 objectId = GetObjectIdFromSlotId(parameter.PkeySlotId);
            if (objectId == 0) {
                return false;
            }

            //
            // APDUヘッダー部（９バイト）
            //
            // オブジェクトID（５バイト）
            CertAPDUBytes[offset++] = PIVConst.TAG_DATA_OBJECT;
            CertAPDUBytes[offset++] = 3;
            CertAPDUBytes[offset++] = (byte)((objectId >> 16) & 0xff);
            CertAPDUBytes[offset++] = (byte)((objectId >> 8) & 0xff);
            CertAPDUBytes[offset++] = (byte)(objectId & 0xff);

            // オブジェクト長（certBytesSize + 9）を３バイトエンコード
            int objectSize = certBytesSize + 9;
            byte itemSizeTag = 0x82;
            byte itemSizeHigh = (byte)((objectSize >> 8) & 0xff);
            byte itemSizeLow = (byte)(objectSize & 0xff);

            // オブジェクト長（４バイト）
            CertAPDUBytes[offset++] = PIVConst.TAG_DATA_OBJECT_VALUE;
            CertAPDUBytes[offset++] = itemSizeTag;
            CertAPDUBytes[offset++] = itemSizeHigh;
            CertAPDUBytes[offset++] = itemSizeLow;

            // 項目長（certBytesSize）を３バイトエンコード
            itemSizeTag = 0x82;
            itemSizeHigh = (byte)((certBytesSize >> 8) & 0xff);
            itemSizeLow = (byte)(certBytesSize & 0xff);

            //
            // データオブジェクト部（証明書生データ長＋９バイト）
            //
            // ヘッダー（４バイト）
            CertAPDUBytes[offset++] = PIVConst.TAG_CERT;
            CertAPDUBytes[offset++] = itemSizeTag;
            CertAPDUBytes[offset++] = itemSizeHigh;
            CertAPDUBytes[offset++] = itemSizeLow;

            // 証明書の生データをコピー
            Array.Copy(parameter.CertBytes, 0, CertAPDUBytes, offset, certBytesSize);
            offset += certBytesSize;

            // フッター（５バイト）
            // compression info & LRC trailer
            CertAPDUBytes[offset++] = PIVConst.TAG_CERT_COMPRESS;
            CertAPDUBytes[offset++] = 0x01;
            CertAPDUBytes[offset++] = 0x00;
            CertAPDUBytes[offset++] = PIVConst.TAG_CERT_LRC;
            CertAPDUBytes[offset++] = 0x00;

            // 生成されたAPDUを、コマンドパラメーターに設定
            parameter.CertAPDUBytes = CertAPDUBytes;
            return true;
        }

        private static UInt32 GetObjectIdFromSlotId(byte slotId)
        {
            // スロットIDからオブジェクトIDを取得
            UInt32 objectId;
            switch (slotId) {
            case PIVConst.PIV_KEY_AUTHENTICATION:
                objectId = PIVConst.PIV_OBJ_AUTHENTICATION;
                break;
            case PIVConst.PIV_KEY_SIGNATURE:
                objectId = PIVConst.PIV_OBJ_SIGNATURE;
                break;
            case PIVConst.PIV_KEY_KEYMGM:
                objectId = PIVConst.PIV_OBJ_KEY_MANAGEMENT;
                break;
            default:
                objectId = 0;
                break;
            }
            return objectId;
        }
    }
}
