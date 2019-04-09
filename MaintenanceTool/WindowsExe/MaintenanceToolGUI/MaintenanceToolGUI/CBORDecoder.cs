using PeterO.Cbor;

namespace MaintenanceToolGUI
{
    internal class KeyAgreement
    {
        public int Kty { get; set; }
        public int Alg { get; set; }
        public int Crv { get; set; }
        public byte[] X { get; set; }
        public byte[] Y { get; set; }
    }

    internal class CBORDecoder
    {
        public CBORDecoder()
        {
        }

        // 認証器から受信した公開鍵を保持
        private KeyAgreement AgreementKey = new KeyAgreement();

        public KeyAgreement GetKeyAgreement(byte[] cborBytes)
        {
            CBORObject cbor = CBORObject.DecodeFromBytes(cborBytes, CBOREncodeOptions.Default);
            foreach (CBORObject key in cbor.Keys) {
                byte keyVal = key.AsByte();
                if (keyVal == 0x01) {
                    ParseCOSEkey(cbor[key]);
                }
            }
            return AgreementKey;
        }

        private void ParseCOSEkey(CBORObject cbor)
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
