﻿using System;
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
        public const int PIV_CCCID_OFFSET = 9;
        public const int PIV_CCCID_SIZE = 14;
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

        //
        // CCCテンプレート
        //  f0: Card Identifier
        //      0xa000000116 == GSC-IS RID
        //      0xff == Manufacturer ID (dummy)
        //      0x02 == Card type (javaCard)
        //      next 14 bytes: card ID
        private byte[] CccTemplate = {
            0xf0, 0x15,
            0xa0, 0x00, 0x00, 0x01, 0x16,
            0xff,
            0x02,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xf1, 0x01, 0x21,
            0xf2, 0x01, 0x21,
            0xf3, 0x00,
            0xf4, 0x01, 0x00,
            0xf5, 0x01, 0x10,
            0xf6, 0x00,
            0xf7, 0x00,
            0xfa, 0x00,
            0xfb, 0x00,
            0xfc, 0x00,
            0xfd, 0x00,
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
            //   先頭５バイトはオブジェクトID
            //   先頭２バイトはTLVのタグ／データ長
            int leadingOffset = 5 + 2;
            byte[] template = new byte[leadingOffset + ChuidTemplate.Length];
            Array.Copy(ChuidTemplate, 0, template, leadingOffset, ChuidTemplate.Length);

            // ランダムなIDを設定
            byte[] randomBytes = new byte[ToolPIVSetIdConst.PIV_CARDID_SIZE];
            GenerateRandomBytes(randomBytes);
            Array.Copy(randomBytes, 0, template, leadingOffset + ToolPIVSetIdConst.PIV_CARDID_OFFSET, ToolPIVSetIdConst.PIV_CARDID_SIZE);

            // 有効期限を１年後に設定
            DateTime nowDate = DateTime.Now;
            DateTime expDate = nowDate.AddYears(1);
            byte[] expDateBytes = Encoding.ASCII.GetBytes(expDate.ToString("yyyyMMdd"));
            Array.Copy(expDateBytes, 0, template, leadingOffset + ToolPIVSetIdConst.PIV_CARDEXP_OFFSET, ToolPIVSetIdConst.PIV_CARDEXP_SIZE);

            // オブジェクトIDを設定
            template[0] = ToolPIVConst.TAG_DATA_OBJECT;
            template[1] = 3;
            UInt32 objectId = ToolPIVConst.PIV_OBJ_CHUID;
            template[2] = (byte)((objectId >> 16) & 0xff);
            template[3] = (byte)((objectId >> 8) & 0xff);
            template[4] = (byte)(objectId & 0xff);

            // タグとデータ長を設定
            template[5] = ToolPIVConst.TAG_DATA_OBJECT_VALUE;
            template[6] = (byte)ChuidTemplate.Length;
            return template;
        }

        public byte[] GenerateCccAPDU()
        {
            // テンプレートをコピー
            //   先頭５バイトはオブジェクトID
            //   先頭２バイトはTLVのタグ／データ長
            int leadingOffset = 5 + 2;
            byte[] template = new byte[leadingOffset + CccTemplate.Length];
            Array.Copy(CccTemplate, 0, template, leadingOffset, CccTemplate.Length);

            // ランダムなIDを設定
            byte[] randomBytes = new byte[ToolPIVSetIdConst.PIV_CCCID_SIZE];
            GenerateRandomBytes(randomBytes);
            Array.Copy(randomBytes, 0, template, leadingOffset + ToolPIVSetIdConst.PIV_CCCID_OFFSET, ToolPIVSetIdConst.PIV_CCCID_SIZE);

            // オブジェクトIDを設定
            template[0] = ToolPIVConst.TAG_DATA_OBJECT;
            template[1] = 3;
            UInt32 objectId = ToolPIVConst.PIV_OBJ_CAPABILITY;
            template[2] = (byte)((objectId >> 16) & 0xff);
            template[3] = (byte)((objectId >> 8) & 0xff);
            template[4] = (byte)(objectId & 0xff);

            // タグとデータ長を設定
            template[5] = ToolPIVConst.TAG_DATA_OBJECT_VALUE;
            template[6] = (byte)CccTemplate.Length;
            return template;
        }

        private void GenerateRandomBytes(byte[] nonce)
        {
            // チャレンジにランダム値を設定
            random.NextBytes(nonce);
        }
    }
}
