using MaintenanceToolCommon;
using PeterO.Cbor;
using System.Linq;

namespace MaintenanceToolGUI
{
    internal class KeyAgreement
    {
        public int Kty { get; set; }
        public int Alg { get; set; }
        public int Crv { get; set; }
        public byte[] X { get; set; }
        public byte[] Y { get; set; }

        public KeyAgreement()
        {
        }

        public KeyAgreement(int kty, int alg, int crv, byte[] x, byte[] y)
        {
            Kty = kty;
            Alg = alg;
            Crv = crv;
            X = new byte[32];
            Y = new byte[32];
            x.CopyTo(X, 0);
            y.CopyTo(Y, 0);
        }
    }

    internal class MakeCredentialResponse
    {
        // データ項目
        public string Fmt { get; set; }
        public byte[] RpIdHash { get; set; }
        public byte Flags { get; set; }
        public int SignCount;
        public byte[] Aaguid { get; set; }
        public int CredentialIdLength;
        public byte[] CredentialId { get; set; }
        public byte[] CredentialPublicKeyByte { get; set; }

        // データ長
        public static int RpIdHashSize = 32;
        public static int SignCountSize = 4;
        public static int AaguidSize = 16;
        public static int CredentialIdLengthSize = 2;

        public MakeCredentialResponse()
        {
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

        public byte[] GetPinTokenEnc(byte[] cborBytes)
        {
            // 暗号化されているpinTokenを抽出
            byte[] pinTokenEnc = null;
            CBORObject cbor = CBORObject.DecodeFromBytes(cborBytes, CBOREncodeOptions.Default);
            foreach (CBORObject key in cbor.Keys) {
                byte keyVal = key.AsByte();
                if (keyVal == 0x02) {
                    pinTokenEnc = cbor[key].GetByteString();
                }
            }
            return pinTokenEnc;
        }

        public MakeCredentialResponse MakeCredential(byte[] cborBytes)
        {
            MakeCredentialResponse MakeCredentialRes = new MakeCredentialResponse();
            CBORObject cbor = CBORObject.DecodeFromBytes(cborBytes, CBOREncodeOptions.Default);
            foreach (CBORObject key in cbor.Keys) {
                var keyVal = key.AsByte();
                if (keyVal == 0x01) {
                    // fmt
                    MakeCredentialRes.Fmt = cbor[key].AsString();
                } else if (keyVal == 0x02) {
                    // authData
                    parseAuthData(cbor[key].GetByteString(), MakeCredentialRes);
                }
            }
            return MakeCredentialRes;
        }

        private void parseAuthData(byte[] data, MakeCredentialResponse MakeCredentialRes)
        {
            int index = 0;
            int size;

            // rpIdHash
            size = MakeCredentialResponse.RpIdHashSize;
            MakeCredentialRes.RpIdHash = data.Skip(index).Take(size).ToArray();
            index += size;

            // for debug
            // AppCommon.OutputLogToFile("rpIdHash: ", true);
            // AppCommon.OutputLogToFile(AppCommon.DumpMessage(MakeCredentialRes.RpIdHash, size), false);

            // flags
            MakeCredentialRes.Flags = data[index];
            index++;

            // signCount（エンディアン変換が必要）
            MakeCredentialRes.SignCount = AppCommon.ToInt32(data, index, true);
            index += MakeCredentialResponse.SignCountSize;

            // aaguid
            size = MakeCredentialResponse.AaguidSize;
            MakeCredentialRes.Aaguid = data.Skip(index).Take(16).ToArray();
            index += size;

            // credentialIdLength（エンディアン変換が必要）
            MakeCredentialRes.CredentialIdLength = AppCommon.ToInt16(data, index, true);
            index += MakeCredentialResponse.CredentialIdLengthSize;

            // CredentialId
            size = MakeCredentialRes.CredentialIdLength;
            MakeCredentialRes.CredentialId = data.Skip(index).Take(size).ToArray();
            index += size;

            // for debug
            // AppCommon.OutputLogToFile("CredentialId: ", true);
            // AppCommon.OutputLogToFile(AppCommon.DumpMessage(MakeCredentialRes.CredentialId, size), false);

            // credentialPublicKey
            MakeCredentialRes.CredentialPublicKeyByte = data.Skip(index).ToArray();
        }
    }
}
