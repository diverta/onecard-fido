using System;
using System.Linq;
using System.Security.Cryptography;
using ToolAppCommon;

namespace MaintenanceToolApp.DFU
{
    internal class DFUCommon
    {
        //
        // BLE SMPサービス関連
        //
        public static byte[] BuildSMPHeader(byte op, byte flags, ushort len, ushort group, byte seq, byte id_int)
        {
            byte[] header = {
                op,
                flags,
                (byte)(len >> 8),   (byte)(len & 0xff),
                (byte)(group >> 8), (byte)(group & 0xff),
                seq,
                id_int
            };
            return header;
        }

        public static int GetSMPResponseBodySize(byte[] responseData)
        {
            // レスポンスヘッダーの３・４バイト目からデータ長を抽出
            int totalSize = ((responseData[2] << 8) & 0xff00) + (responseData[3] & 0x00ff);
            return totalSize;
        }

        //
        // イメージ転送処理関連
        //
        public static byte[] GenerateBodyForRequestUploadImage(DFUParameter Parameter)
        {
            // リクエストデータ
            byte[] body = { 0xbf };

            // 転送元データ長
            uint bytesTotal = (uint)Parameter.UpdateImageData.NRF53AppBinSize;

            if (Parameter.ImageBytesSent == 0) {
                // 初回呼び出しの場合、イメージ長を設定
                body = body.Concat(DFUCommon.GenerateLenBytes(bytesTotal)).ToArray();

                // イメージのハッシュ値を設定
                body = body.Concat(DFUCommon.GenerateSHA256HashData(Parameter.UpdateImageData.NRF53AppBin)).ToArray();
            }

            // 転送済みバイト数を設定
            body = body.Concat(DFUCommon.GenerateOffBytes(Parameter.ImageBytesSent)).ToArray();

            // 転送イメージを連結（データ本体が240バイトに収まるよう、上限サイズを事前計算）
            int remainingSize = 240 - body.Length - 1;
            body = body.Concat(DFUCommon.GenerateDataBytes(Parameter.UpdateImageData.NRF53AppBin, Parameter.ImageBytesSent, remainingSize)).ToArray();

            // 終端文字を設定して戻す
            byte[] terminator = { 0xff };
            return body.Concat(terminator).ToArray();
        }

        private static byte[] GenerateLenBytes(uint bytesTotal)
        {
            // イメージ長を設定
            byte[] lenBytes = {
                0x63, 0x6c, 0x65, 0x6e, 0x1a, 0x00, 0x00, 0x00, 0x00
            };
            AppUtil.ConvertUint32ToBEBytes(bytesTotal, lenBytes, 5);
            return lenBytes;
        }

        private static byte[] GenerateSHA256HashData(byte[] data)
        {
            // イメージのハッシュ値を計算
            SHA256 sha = SHA256.Create();
            byte[] hash = sha.ComputeHash(data);

            // イメージのハッシュ値を設定
            byte[] shaBytes = {
                0x63, 0x73, 0x68, 0x61, 0x43, 0x00, 0x00, 0x00,
            };

            // 指定領域から３バイト分の領域に、SHA-256ハッシュの先頭３バイト分を設定
            for (int i = 0; i < 3; i++) {
                shaBytes[i + 5] = hash[i];
            }
            return shaBytes;
        }

        private static byte[] GenerateOffBytes(int bytesSent)
        {
            // 転送済みバイト数を設定
            byte[] offBytes = {
                0x63, 0x6f, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00
            };
            int len = offBytes.Length;
            if (bytesSent == 0) {
                len = 5;

            } else if (bytesSent < 0x100) {
                offBytes[4] = 0x18;
                offBytes[5] = (byte)bytesSent;
                len = 6;

            } else if (bytesSent < 0x10000) {
                offBytes[4] = 0x19;
                AppUtil.ConvertUint16ToBEBytes((UInt16)bytesSent, offBytes, 5);
                len = 7;

            } else {
                offBytes[4] = 0x1a;
                AppUtil.ConvertUint32ToBEBytes((UInt32)bytesSent, offBytes, 5);
            }

            // 不要な末尾バイトを除去して戻す
            byte[] offData = offBytes.Take(len).ToArray();
            return offData;
        }

        private static byte[] GenerateDataBytes(byte[] imageData, int bytesSent, int remaining)
        {
            // 転送バイト数を設定
            byte[] bodyBytes = {
                0x64, 0x64, 0x61, 0x74, 0x61, 0x58, 0x00
            };

            // 転送バイト数
            int bytesToSend = remaining - bodyBytes.Length;
            if (bytesToSend > imageData.Length - bytesSent) {
                bytesToSend = imageData.Length - bytesSent;
            }
            bodyBytes[6] = (byte)bytesToSend;

            // 転送イメージを抽出
            byte[] sendData = imageData.Skip(bytesSent).Take(bytesToSend).ToArray();

            // 転送イメージを連結して戻す
            return bodyBytes.Concat(sendData).ToArray();
        }

        public static byte[] GenerateBodyForRequestChangeImageUpdateMode(DFUParameter parameter, bool imageUpdateTestMode)
        {
            // リクエストデータ
            byte[] body = {
                0xbf, 0x67, 0x63, 0x6f, 0x6e, 0x66, 0x69, 0x72, 0x6d, 0x00,
                0x64, 0x68, 0x61, 0x73, 0x68, 0x58, 0x20
            };

            // イメージ反映モードを設定（confirm=false/true）
            if (imageUpdateTestMode) {
                body[9] = 0xf4;
            } else {
                body[9] = 0xf5;
            }

            // SHA-256ハッシュデータをイメージから抽出
            byte[] hashUpdate = parameter.UpdateImageData.SHA256Hash;

            // 本体にSHA-256ハッシュを連結
            body = body.Concat(hashUpdate).ToArray();

            // 終端文字を設定して戻す
            byte[] terminator = { 0xff };
            return body.Concat(terminator).ToArray();
        }
    }
}
