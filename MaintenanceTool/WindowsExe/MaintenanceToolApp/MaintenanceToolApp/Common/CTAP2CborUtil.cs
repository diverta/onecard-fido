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

        public static byte[] GenerateMakeCredentialCbor(byte cborCommand, MakeCredentialParameter parameter, byte[] clientDataHash, byte[] pinAuth)
        {
            // 送信データを生成
            //   0x01: clientDataHash
            //   0x02: rp
            //   0x03: user
            //   0x04: pubKeyCredParams
            //   0x06: extensions
            //   0x07: options
            //   0x08: pinAuth
            //   0x09: pinProtocol
            CBORObject cbor = CBORObject.NewMap();
            cbor.Add(0x01, clientDataHash);
            cbor.Add(0x02, CBORObject.NewMap().Add("id", parameter.RpId).Add("name", parameter.RpName));

            var user = CBORObject.NewMap();
            user.Add("id", parameter.UserId);
            user.Add("name", parameter.UserName );
            user.Add("displayName", parameter.DisplayName);
            cbor.Add(0x03, user);

            var pubKeyCredParams = CBORObject.NewMap();
            pubKeyCredParams.Add("alg", -7);
            pubKeyCredParams.Add("type", "public-key");
            cbor.Add(0x04, CBORObject.NewArray().Add(pubKeyCredParams));

            var extensions = CBORObject.NewMap();
            extensions.Add("hmac-secret", true);
            cbor.Add(0x06, extensions);

            var opt = CBORObject.NewMap();
            opt.Add("rk", false);
            opt.Add("uv", false);
            opt.Add("up", false);
            cbor.Add(0x07, opt);

            if (pinAuth != null) {
                cbor.Add(0x08, pinAuth);
                cbor.Add(0x09, 1);
            }

            // エンコードを実行
            byte[] payload = cbor.EncodeToBytes();
            byte[] encoded = new byte[] { cborCommand }.Concat(payload).ToArray();

            // for debug
            AppLogUtil.OutputLogDebug(string.Format(
                "Encoded CBOR request(MakeCredential): \n{0}", AppLogUtil.DumpMessage(encoded, encoded.Length)));

            // エンコードされたCBORバイト配列を戻す
            return encoded;
        }
    }

    internal class CBORDecoder
    {
        public static KeyAgreement GetKeyAgreement(byte[] cborBytes)
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

        private static void ParseCOSEkey(CBORObject cbor, KeyAgreement AgreementKey)
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

        public static byte[] GetPinTokenEnc(byte[] cborBytes)
        {
            // 暗号化されているpinTokenを抽出
            byte[] pinTokenEnc = new byte[0];
            CBORObject cbor = CBORObject.DecodeFromBytes(cborBytes, CBOREncodeOptions.Default);
            foreach (CBORObject key in cbor.Keys) {
                byte keyVal = key.AsByte();
                if (keyVal == 0x02) {
                    pinTokenEnc = cbor[key].GetByteString();
                }
            }
            return pinTokenEnc;
        }
    }
}
