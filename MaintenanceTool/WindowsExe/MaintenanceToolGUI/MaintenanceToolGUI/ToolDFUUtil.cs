using System;
using System.Linq;

namespace MaintenanceToolGUI
{
    class ToolDFUUtil
    {
        // DFUで使用するパラメーター
        private int MTU;

        public ToolDFUUtil(int size)
        {
            // MTUは４の倍数とする
            MTU = (size / 4) * 4;
        }

        //
        // フレーム分割送信処理
        //
        private byte[] PrivateObjectBuf;
        private int PrivateObjectLen;
        private int PrivateObjectIx;
        private int PrivateObjectOffset;
        private UInt32 PrivateObjectCrc;

        // 生成されたフレームを保持
        private byte[] PrivateRequestBuf = new byte[2560];
        private int PrivateRequestLen;

        public void DFUObjectFrameInit(byte[] objectData, int objectSize, int offset)
        {
            PrivateObjectBuf = objectData;
            PrivateObjectLen = objectSize;
            PrivateObjectIx = 0;
            PrivateObjectOffset = offset;
        }

        public byte[] GetPreparedFrame()
        {
            // 生成した送信フレームを戻す
            byte[] b = PrivateRequestBuf.Take(PrivateRequestLen).ToArray();
            return b;
        }

        public bool DFUObjectFramePrepare(byte objectType)
        {
            if (PrivateObjectIx < PrivateObjectLen) {
                // 送信フレーム格納領域にバイトデータを設定
                CreateObjectFrameToWrite(NRFDfuConst.NRF_DFU_OP_OBJECT_WRITE);
                return true;

            } else {
                // これ以上送信すべきデータがない場合はfalse
                return false;
            }
        }

        private void CreateObjectFrameToWrite(byte requestCode)
        {
            // フレームサイズを計算
            int remaining = PrivateObjectLen - PrivateObjectIx;
            if (remaining > MTU) {
                remaining = MTU;
            }

            // 先頭にコマンド種別をセット
            int p = 0;
            PrivateRequestBuf[p++] = requestCode;
            PrivateRequestLen = 1;

            // 送信データを設定
            for (int i = 0; i < remaining; i++) {
                byte b = PrivateObjectBuf[PrivateObjectOffset + PrivateObjectIx];
                if (b == 0xC0 || b == 0xDB) {
                    // C0->DBDC, DB->DBDD に変換
                    PrivateRequestBuf[p++] = 0xDB;
                    PrivateRequestBuf[p++] = (byte)((b == 0xC0) ? 0xDC : 0xDD);
                    PrivateRequestLen += 2;
                } else {
                    PrivateRequestBuf[p++] = b;
                    PrivateRequestLen++;
                }
                UpdateCRC(b);
                PrivateObjectIx++;
            }

            // 終端バイトをセット
            PrivateRequestBuf[p] = NRFDfuConst.NRF_DFU_BYTE_EOM;
            PrivateRequestLen++;
        }

        private void UpdateCRC(byte b)
        {
            int i;
            UInt32 crc = PrivateObjectCrc;
            crc ^= b;
            for (i = 0; i < 8; i++) {
                UInt32 o = crc & 0x00000001;
                UInt32 m = (o == 0) ? 0x00000000 : 0xffffffff;
                crc = (crc >> 1) ^ (0xedb88320u & m);
            }
            PrivateObjectCrc = crc;
        }

        public UInt32 DFUObjectChecksumGet()
        {
            // CRC（内部保持値を反転したもの）を戻す
            return ~PrivateObjectCrc;
        }

        public void DFUObjectChecksumReset()
        {
            // CRCを初期設定
            PrivateObjectCrc = 0xffffffff;
        }

        //
        // ユーティリティー
        //
        public static byte[] UnescapeResponseData(byte[] response)
        {
            int size = response.Length;
            byte[] unescaped = new byte[0];
            int cnt = 0;

            byte c;
            bool escapeChar = false;
            for (int i = 0; i < size; i++) {
                c = response[i];
                if (c == 0xdb) {
                    escapeChar = true;
                } else {
                    if (escapeChar) {
                        escapeChar = false;
                        // DBDC->C0, DBDD->DB に変換
                        if (c == 0xdc) {
                            c = 0xc0;
                        } else if (c == 0xdd) {
                            c = 0xdb;
                        }
                    }
                    // 配列サイズを変更
                    Array.Resize(ref unescaped, ++cnt);
                    unescaped[cnt - 1] = c;
                }
            }
            return unescaped;
        }
    }
}
