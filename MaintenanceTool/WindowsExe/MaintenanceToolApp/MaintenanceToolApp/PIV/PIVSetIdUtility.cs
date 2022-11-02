using System;
using System.Text;

namespace MaintenanceToolApp.PIV
{
    internal class PIVSetIdConst
    {
        // テンプレート内項目の設定位置、長さ
        public const int PIV_CARDID_OFFSET = 29;
        public const int PIV_CARDID_SIZE = 16;
        public const int PIV_CARDEXP_OFFSET = 47;
        public const int PIV_CARDEXP_SIZE = 8;
        public const int PIV_CCCID_OFFSET = 9;
        public const int PIV_CCCID_SIZE = 14;
    }

    internal class PIVSetIdUtility
    {
        // このクラスのインスタンス
        private static readonly PIVSetIdUtility Instance = new PIVSetIdUtility();

        // 乱数製造用
        private readonly Random Randomizer = new Random();

        //
        // CHUIDテンプレート
        //  0x30: FASC-N
        //  0x34: Card UUID / GUID
        //  0x35: Exp. Date
        //  0x3e: Signature
        //  0xfe: Error Detection Code
        //
        private static readonly byte[] ChuidTemplate = {
            0x30, 0x19, 0xd4, 0xe7, 0x39, 0xda, 0x73, 0x9c, 0xed, 0x39, 0xce, 0x73, 0x9d, 0x83, 0x68, 0x58, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0xc8, 0x42, 0x10, 0xc3, 0xeb,
            0x34, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x35, 0x08, 0x32, 0x30, 0x33, 0x30, 0x30, 0x31, 0x30, 0x31,
            0x3e, 0x00,
            0xfe, 0x00
        };

        //
        // CHUIDインポート用リクエストデータ生成
        //
        public static byte[] GenerateChuidAPDU()
        {
            // テンプレートをコピー
            //   先頭５バイトはオブジェクトID
            //   先頭２バイトはTLVのタグ／データ長
            int leadingOffset = 5 + 2;
            byte[] template = new byte[leadingOffset + ChuidTemplate.Length];
            Array.Copy(ChuidTemplate, 0, template, leadingOffset, ChuidTemplate.Length);

            // ランダムなIDを設定
            byte[] randomBytes = new byte[PIVSetIdConst.PIV_CARDID_SIZE];
            Instance.Randomizer.NextBytes(randomBytes);
            Array.Copy(randomBytes, 0, template, leadingOffset + PIVSetIdConst.PIV_CARDID_OFFSET, PIVSetIdConst.PIV_CARDID_SIZE);

            // 有効期限を10年後に設定
            DateTime nowDate = DateTime.Now;
            DateTime expDate = nowDate.AddYears(10);
            byte[] expDateBytes = Encoding.ASCII.GetBytes(expDate.ToString("yyyyMMdd"));
            Array.Copy(expDateBytes, 0, template, leadingOffset + PIVSetIdConst.PIV_CARDEXP_OFFSET, PIVSetIdConst.PIV_CARDEXP_SIZE);

            // オブジェクトIDを設定
            template[0] = PIVConst.TAG_DATA_OBJECT;
            template[1] = 3;
            UInt32 objectId = PIVConst.PIV_OBJ_CHUID;
            template[2] = (byte)((objectId >> 16) & 0xff);
            template[3] = (byte)((objectId >> 8) & 0xff);
            template[4] = (byte)(objectId & 0xff);

            // タグとデータ長を設定
            template[5] = PIVConst.TAG_DATA_OBJECT_VALUE;
            template[6] = (byte)ChuidTemplate.Length;
            return template;
        }
    }
}
