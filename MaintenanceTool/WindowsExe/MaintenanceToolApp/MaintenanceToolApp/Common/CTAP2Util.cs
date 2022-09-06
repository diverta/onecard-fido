﻿using MaintenanceToolApp.HealthCheck;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using ToolAppCommon;
using static MaintenanceToolApp.FIDODefine;

namespace MaintenanceToolApp.Common
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
            X = new byte[0];
            Y = new byte[0];
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

    internal class MakeCredentialParameter
    {
        public string RpId { get; set; }
        public string RpName { get; set; }
        public byte[] UserId { get; set; }
        public string UserName { get; set; }
        public string DisplayName { get; set; }

        public MakeCredentialParameter(string rpId, string rpName, byte[] userId, string userName, string displayName)
        {
            RpId = rpId;
            RpName = rpName;
            UserId = userId;
            UserName = userName;
            DisplayName = displayName;
        }
    }

    internal class CTAP2Util
    {
        //
        // PinToken生成関連
        // 
        public static bool GenerateSharedSecretKey(CTAP2HealthCheckParameter parameter)
        {
            // 公開鍵を抽出
            KeyAgreement agreementKey = parameter.AgreementPublicKey;
            byte[] aG_x = agreementKey.X;
            byte[] aG_y = agreementKey.Y;

            // for debug
            AppLogUtil.OutputLogDebug(string.Format(
                "Public key: \n{0}\n{1}", 
                AppLogUtil.DumpMessage(aG_x, aG_x.Length), 
                AppLogUtil.DumpMessage(aG_y, aG_y.Length)));

            // ECDHキーペアを新規作成
            // COSE ES256 (ECDSA over P-256 with SHA-256) P-256
            ECDiffieHellmanCng ecdh = new ECDiffieHellmanCng(256) {
                KeyDerivationFunction = ECDiffieHellmanKeyDerivationFunction.Hash,
                HashAlgorithm = CngAlgorithm.Sha256
            };

            // 公開鍵を生成
            byte[] newPublicKey = ecdh.PublicKey.ToByteArray();
            if (newPublicKey.Length != 72) {
                return false;
            }

            // ヘッダー（8バイト）を除去し公開鍵を抽出
            List<byte> pkey = newPublicKey.ToList().Skip(8).Take(64).ToList();
            byte[] bG_x = pkey.Take(32).ToArray();
            byte[] bG_y = pkey.Skip(32).Take(32).ToArray();

            // 公開鍵をパラメーターに保持
            parameter.AgreementPublicKey = new KeyAgreement(2, -7, 1, bG_x, bG_y);

            // ヘッダーと公開鍵のx, yを連結し、
            // CngKeyを作成するための公開鍵を生成
            byte[] header = new byte[] { 0x45, 0x43, 0x4b, 0x31, 0x20, 0x00, 0x00, 0x00 }; // 'ECK1'
            byte[] publicKeyforChgKey = header.Concat(aG_x).Concat(aG_y).ToArray();

            // 共通鍵を生成し、パラメーターに保持
            byte[] sharedSecretKey = ecdh.DeriveKeyMaterial(CngKey.Import(publicKeyforChgKey, CngKeyBlobFormat.EccPublicBlob));
            if (sharedSecretKey.Length != 32) {
                return false;
            }
            parameter.SharedSecretKey = sharedSecretKey;
            return true;
        }

        public static void GeneratePinHashEnc(string curPin, CTAP2HealthCheckParameter parameter)
        {
            // curPin のハッシュを生成  SHA-256(curPin)
            byte[] pinbyte = Encoding.ASCII.GetBytes(curPin);
            SHA256 sha = new SHA256CryptoServiceProvider();
            byte[] pinsha = sha.ComputeHash(pinbyte);

            // 先頭16バイトを抽出  LEFT(SHA-256(curPin),16)
            byte[] pinsha16 = pinsha.ToList().Skip(0).Take(16).ToArray();

            // for debug
            AppLogUtil.OutputLogDebug(string.Format(
                "pinHash: \n{0}", AppLogUtil.DumpMessage(pinsha16, pinsha16.Length)));

            // AES256-CBCで暗号化  
            //   AES256-CBC(sharedSecret, IV=0, LEFT(SHA-256(curPin),16))
            byte[] pinHashEnc = AES256CBCEncrypt(parameter.SharedSecretKey, pinsha16);

            // for debug
            AppLogUtil.OutputLogDebug(string.Format(
                "pinHashEnc: \n{0}", AppLogUtil.DumpMessage(pinHashEnc, pinHashEnc.Length)));

            // 生成されたPinHashEncを、パラメーターに保持
            parameter.PinHashEnc = pinHashEnc;
        }

        //
        // MakeCredential／GetAssertion関連
        //
        public static byte[] ComputeClientDataHash(byte[] challenge)
        {
            SHA256 sha = new SHA256CryptoServiceProvider();
            return sha.ComputeHash(challenge);
        }

        public static byte[] GenerateClientPinAuth(byte[] pinToken, byte[] clientDataHash)
        {
            // LEFT(HMAC-SHA-256(pinToken, clientDataHash), 16)
            byte[] pinAuth = new byte[0];
            using (var hmacsha256 = new HMACSHA256(pinToken)) {
                byte[] digest = hmacsha256.ComputeHash(clientDataHash);
                pinAuth = digest.ToList().Take(16).ToArray();
            }
            return pinAuth;
        }

        public static bool CheckStatusByte(byte[] receivedMessage, out string errorMessage)
        {
            errorMessage = "";
            switch (receivedMessage[0]) {
            case CTAP1_ERR_SUCCESS:
                return true;
            case CTAP2_ERR_PIN_INVALID:
            case CTAP2_ERR_PIN_AUTH_INVALID:
                errorMessage = AppCommon.MSG_CTAP2_ERR_PIN_INVALID;
                break;
            case CTAP2_ERR_PIN_BLOCKED:
                errorMessage = AppCommon.MSG_CTAP2_ERR_PIN_BLOCKED;
                break;
            case CTAP2_ERR_PIN_AUTH_BLOCKED:
                errorMessage = AppCommon.MSG_CTAP2_ERR_PIN_AUTH_BLOCKED;
                break;
            case CTAP2_ERR_PIN_NOT_SET:
                errorMessage = AppCommon.MSG_CTAP2_ERR_PIN_NOT_SET;
                break;
            case CTAP2_ERR_VENDOR_KEY_CRT_NOT_EXIST: // CTAP2_ERR_VENDOR_FIRST+0x0e
                errorMessage = AppCommon.MSG_OCCUR_SKEYNOEXIST_ERROR;
                break;
            default:
                errorMessage = string.Format("不明なステータスにより処理が失敗しました: 0x{0:x2}", receivedMessage[0]);
                break;
            }

            // エラーログ出力
            AppLogUtil.OutputLogError(errorMessage);
            return false;
        }

        //
        // 共通関数
        //
        public static byte[] AES256CBCEncrypt(byte[] key, byte[] data)
        {
            // AES256-CBCにより暗号化
            //   鍵の長さ: 256（32バイト）
            //   ブロックサイズ: 128（16バイト）
            //   暗号利用モード: CBC
            //   初期化ベクター: 0
            AesManaged aes = new AesManaged {
                KeySize = 256,
                BlockSize = 128,
                Mode = CipherMode.CBC,
                IV = new byte[] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                Key = key,
                Padding = PaddingMode.None
            };
            return aes.CreateEncryptor().TransformFinalBlock(data, 0, data.Length);
        }

        public static byte[] AES256CBCDecrypt(byte[] key, byte[] data)
        {
            // AES256-CBCにより復号化
            //   鍵の長さ: 256（32バイト）
            //   ブロックサイズ: 128（16バイト）
            //   暗号利用モード: CBC
            //   初期化ベクター: 0
            AesManaged aes = new AesManaged {
                KeySize = 256,
                BlockSize = 128,
                Mode = CipherMode.CBC,
                IV = new byte[] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                Key = key,
                Padding = PaddingMode.None
            };
            return aes.CreateDecryptor().TransformFinalBlock(data, 0, data.Length);
        }
    }
}
