using MaintenanceToolCommon;
using PeterO.Cbor;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;

namespace MaintenanceToolGUI
{
    internal class CBOREncoder
    {
        public CBOREncoder()
        {
        }

        // 管理ツール側で発行した新規公開鍵を保持
        KeyAgreement AgreementPublicKey;

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
            CreateSharedSecretKey(agreementKeyCBOR);

            // 仮の実装
            return null;
        }

        public byte[] ChangePIN(byte cborCommand, byte subCommand, string pinNew, string pinOld, byte[] agreementKeyCBOR)
        {
            // 公開鍵から共通鍵を生成
            CreateSharedSecretKey(agreementKeyCBOR);

            // 仮の実装
            return null;
        }

        private byte[] CreateSharedSecretKey(byte[] agreementKeyCBOR)
        {
            // CBORをデコードして公開鍵を抽出
            CBORDecoder cborDecoder = new CBORDecoder();
            KeyAgreement agreementKey = cborDecoder.GetKeyAgreement(agreementKeyCBOR);
            byte[] aG_x = agreementKey.X;
            byte[] aG_y = agreementKey.Y;

            // for debug
            AppCommon.OutputLogToFile("Public key: ", true);
            AppCommon.OutputLogToFile(AppCommon.DumpMessage(aG_x, aG_x.Length), false);
            AppCommon.OutputLogToFile(AppCommon.DumpMessage(aG_y, aG_y.Length), false);

            // 公開鍵から共通鍵を生成
            // COSE ES256 (ECDSA over P-256 with SHA-256) P-256
            ECDiffieHellmanCng ecdh = new ECDiffieHellmanCng(256);
            ecdh.KeyDerivationFunction = ECDiffieHellmanKeyDerivationFunction.Hash;
            // ECDHキーペアを新規作成
            ecdh.HashAlgorithm = CngAlgorithm.Sha256;
            byte[] newPublicKey = ecdh.PublicKey.ToByteArray();
            if (newPublicKey.Length != 72) {
                return null;
            }

            // ヘッダー（8バイト）を除去し公開鍵を抽出
            List<byte> pkey = newPublicKey.ToList().Skip(8).Take(64).ToList();
            byte[] bG_x = pkey.Take(32).ToArray();
            byte[] bG_y = pkey.Skip(32).Take(32).ToArray();

            // 公開鍵をモジュール内で退避
            AgreementPublicKey = new KeyAgreement(2, -7, 1, bG_x, bG_y);

            // ヘッダーと公開鍵のx, yを連結し、
            // CngKeyを作成するための公開鍵を生成
            byte[] header = new byte[] { 0x45, 0x43, 0x4b, 0x31, 0x20, 0x00, 0x00, 0x00 }; // 'ECK1'
            byte[] publicKeyforChgKey = header.Concat(aG_x).Concat(aG_y).ToArray();

            // 共通鍵を生成
            byte[] sharedSecretKey = ecdh.DeriveKeyMaterial(CngKey.Import(publicKeyforChgKey, CngKeyBlobFormat.EccPublicBlob));
            if (sharedSecretKey.Length != 32) {
                return null;
            }

            // for debug
            AppCommon.OutputLogToFile("Shared secret key: ", true);
            AppCommon.OutputLogToFile(AppCommon.DumpMessage(sharedSecretKey, sharedSecretKey.Length), false);
            
            return sharedSecretKey;
        }
    }
}
