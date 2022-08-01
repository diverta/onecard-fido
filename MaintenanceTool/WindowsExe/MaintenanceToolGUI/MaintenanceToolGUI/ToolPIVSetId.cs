using System;
using System.Text;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public class ToolPIVSetIdConst
    {
        // テンプレート内項目の設定位置、長さ
        public const int PIV_CARDID_OFFSET = 29;
        public const int PIV_CARDID_SIZE = 16;
        public const int PIV_CARDEXP_OFFSET = 47;
        public const int PIV_CARDEXP_SIZE = 8;
    }

    class ToolPIVSetId
    {
        //
        // CHUIDテンプレート
        //  0x30: FASC-N
        //  0x34: Card UUID / GUID
        //  0x35: Exp. Date
        //  0x3e: Signature
        //  0xfe: Error Detection Code
        //
        private byte[] ChuidTemplate = {
            0x30, 0x19, 0xd4, 0xe7, 0x39, 0xda, 0x73, 0x9c, 0xed, 0x39, 0xce, 0x73, 0x9d, 0x83, 0x68, 0x58, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0xc8, 0x42, 0x10, 0xc3, 0xeb,
            0x34, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x35, 0x08, 0x32, 0x30, 0x33, 0x30, 0x30, 0x31, 0x30, 0x31,
            0x3e, 0x00,
            0xfe, 0x00
        };

        // 乱数製造用
        private Random random = new Random();

        public ToolPIVSetId()
        {
        }

        public byte[] GenerateChuidAPDU()
        {
            // テンプレートをコピー
            //   先頭２バイトはTLVのタグ／データ長
            byte[] template = new byte[2 + ChuidTemplate.Length];
            Array.Copy(ChuidTemplate, 0, template, 2, ChuidTemplate.Length);

            // ランダムなIDを設定
            byte[] randomBytes = new byte[ToolPIVSetIdConst.PIV_CARDID_SIZE];
            GenerateRandomBytes(randomBytes);
            Array.Copy(randomBytes, 0, template, 2 + ToolPIVSetIdConst.PIV_CARDID_OFFSET, ToolPIVSetIdConst.PIV_CARDID_SIZE);

            // 有効期限を１年後に設定
            DateTime nowDate = DateTime.Now;
            DateTime expDate = nowDate.AddYears(1);
            byte[] expDateBytes = Encoding.ASCII.GetBytes(expDate.ToString("yyyyMMdd"));
            Array.Copy(expDateBytes, 0, template, 2 + ToolPIVSetIdConst.PIV_CARDEXP_OFFSET, ToolPIVSetIdConst.PIV_CARDEXP_SIZE);

            // タグとデータ長を設定
            template[0] = ToolPIVConst.TAG_DATA_OBJECT_VALUE;
            template[1] = (byte)ChuidTemplate.Length;
            return template;
        }

        public byte[] GenerateCccAPDU()
        {
            // TODO: 仮の実装です。
            return new byte[0];
        }

        private void GenerateRandomBytes(byte[] nonce)
        {
            // チャレンジにランダム値を設定
            random.NextBytes(nonce);
        }
    }
}
