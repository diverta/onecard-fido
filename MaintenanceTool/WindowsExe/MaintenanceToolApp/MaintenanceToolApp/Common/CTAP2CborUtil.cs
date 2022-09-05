using PeterO.Cbor;
using System.Linq;
using ToolAppCommon;

namespace MaintenanceToolApp.Common
{
    internal class CBOREncoder
    {
        public static byte[] GenerateGetKeyAgreementCbor(byte cborCommand, byte subCommand)
        {
            // 送信データを生成
            //   0x01: pinProtocol = 1
            //   0x02: subCommand = 0x02(getKeyAgreement)
            CBORObject cbor = CBORObject.NewMap();
            cbor.Add(0x01, 1);
            cbor.Add(0x02, subCommand);
            byte[] payload = cbor.EncodeToBytes();
            byte[] encoded = new byte[] { cborCommand }.Concat(payload).ToArray();

            // エンコードされたCBORバイト配列を戻す
            return encoded;
        }

        public static byte[] GenerateGetPinTokenCbor(byte cborCommand, byte subCommand, KeyAgreement publicKey, byte[] pinHashEnc)
        {
            // 送信データを生成
            //   0x01: pinProtocol
            //   0x02: subCommand
            //   0x03: keyAgreement
            //   0x06: pinHashEnc
            CBORObject cbor = CBORObject.NewMap();
            cbor.Add(0x01, 1);
            cbor.Add(0x02, subCommand);

            CBORObject keyParam = CBORObject.NewMap();
            keyParam.Add(1, publicKey.Kty);
            keyParam.Add(3, publicKey.Alg);
            keyParam.Add(-1, publicKey.Crv);
            keyParam.Add(-2, publicKey.X);
            keyParam.Add(-3, publicKey.Y);
            cbor.Add(0x03, keyParam);
            cbor.Add(0x06, pinHashEnc);

            // エンコードを実行
            byte[] payload = cbor.EncodeToBytes();
            byte[] encoded = new byte[] { cborCommand }.Concat(payload).ToArray();

            // for debug
            AppLogUtil.OutputLogDebug(string.Format(
                "Encoded CBOR request: \n{0}", AppLogUtil.DumpMessage(encoded, encoded.Length)));

            // エンコードされたCBORバイト配列を戻す
            return encoded;
        }
    }

    internal class CBORDecoder
    {
        public CBORDecoder()
        {
        }

        public KeyAgreement GetKeyAgreement(byte[] cborBytes)
        {
            KeyAgreement AgreementKey = new KeyAgreement();
            CBORObject cbor = CBORObject.DecodeFromBytes(cborBytes, CBOREncodeOptions.Default);
            foreach (CBORObject key in cbor.Keys) {
                byte keyVal = key.AsByte();
                if (keyVal == 0x01) {
                    ParseCOSEkey(cbor[key], AgreementKey);
                }
            }
            return AgreementKey;
        }

        private void ParseCOSEkey(CBORObject cbor, KeyAgreement AgreementKey)
        {
            foreach (CBORObject key in cbor.Keys) {
                short keyVal = key.AsInt16();
                if (keyVal == 1) {
                    AgreementKey.Kty = cbor[key].AsInt16();
                } else if (keyVal == 3) {
                    AgreementKey.Alg = cbor[key].AsInt16();
                } else if (keyVal == -1) {
                    AgreementKey.Crv = cbor[key].AsInt16();
                } else if (keyVal == -2) {
                    AgreementKey.X = cbor[key].GetByteString();
                } else if (keyVal == -3) {
                    AgreementKey.Y = cbor[key].GetByteString();
                }
            }
        }
    }
}
