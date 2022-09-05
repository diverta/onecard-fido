using PeterO.Cbor;
using System.Linq;

namespace MaintenanceToolApp.Common
{
    internal class CBOREncoder
    {
        public CBOREncoder()
        {
        }

        public byte[] GetKeyAgreement(byte cborCommand, byte subCommand)
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

    }
}
