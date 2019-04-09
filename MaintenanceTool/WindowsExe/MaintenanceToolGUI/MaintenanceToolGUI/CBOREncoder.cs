using MaintenanceToolCommon;
using PeterO.Cbor;
using System;
using System.Linq;

namespace MaintenanceToolGUI
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

        public byte[] SetPIN(byte cborCommand, byte subCommand, string pinNew, byte[] agreementKeyCBOR)
        {
            // 公開鍵から共通鍵を生成
            ExtractSharedPrivateKey(agreementKeyCBOR);

            // 仮の実装
            return null;
        }

        public byte[] ChangePIN(byte cborCommand, byte subCommand, string pinNew, string pinOld, byte[] agreementKeyCBOR)
        {
            // 公開鍵から共通鍵を生成
            ExtractSharedPrivateKey(agreementKeyCBOR);

            // 仮の実装
            return null;
        }

        private void ExtractSharedPrivateKey(byte[] agreementKeyCBOR)
        {
            // CBORをデコードして公開鍵を抽出
            CBORDecoder cborDecoder = new CBORDecoder();
            KeyAgreement agreementKey = cborDecoder.GetKeyAgreement(agreementKeyCBOR);
            string aG_x = BitConverter.ToString(agreementKey.X).Replace("-", string.Empty);
            string aG_y = BitConverter.ToString(agreementKey.Y).Replace("-", string.Empty);

            // for debug
            AppCommon.OutputLogToFile(aG_x, true);
            AppCommon.OutputLogToFile(aG_y, true);

            // TODO: 公開鍵から共通鍵を生成
        }
    }
}
