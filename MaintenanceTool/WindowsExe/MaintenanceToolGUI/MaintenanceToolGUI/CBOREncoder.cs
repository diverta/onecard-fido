using MaintenanceToolCommon;
using PeterO.Cbor;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;

namespace MaintenanceToolGUI
{
    internal class CBOREncoder
    {
        public CBOREncoder()
        {
        }

        // 管理ツール側で発行した新規公開鍵を保持
        public KeyAgreement AgreementPublicKey { get; set; }

        // pinHashEnc: 
        //   Encrypted first 16 bytes of SHA-256 hash of curPin 
        //   using sharedSecret
        //   AES256-CBC(sharedSecret, IV= 0, LEFT(SHA-256(curPin),16))
        private byte[] PinHashEnc;

        // newPinEnc: Encrypted newPin using sharedSecret
        //   AES256-CBC(sharedSecret, IV=0, newPin)
        private byte[] NewPinEnc;

        // pinAuth
        //   setPIN: LEFT(HMAC-SHA-256(sharedSecret, newPinEnc), 16)
        //   changePIN: LEFT(HMAC-SHA-256(sharedSecret, newPinEnc || pinHashEnc), 16)
        private byte[] PinAuth;

        // 共通鍵を保持
        public byte[] SharedSecretKey { get; set; }

        // hmac-secret機能で使用するsaltを保持
        private Random random = new Random();
        private byte[] Salt = new byte[64];

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

        public byte[] SetPIN(byte cborCommand, byte subCommand, string pinNew, string pinOld, byte[] agreementKeyCBOR)
        {
            // 公開鍵から共通鍵を生成
            if (CreateSharedSecretKey(agreementKeyCBOR) == false) {
                return null;
            }

            if (pinOld.Equals(string.Empty)) {
                PinHashEnc = null;
            } else {
                // pinHashEncを生成
                PinHashEnc = CreatePinHashEnc(pinOld, SharedSecretKey);
            }

            // newPinEnc、pinAuthを生成
            CreateNewPinEnc(pinNew);
            CreatePinAuth(NewPinEnc, PinHashEnc);

            // 送信データをCBORエンコードしたバイト配列を戻す
            return GenerateSetPinCbor(cborCommand, subCommand);
        }

        public byte[] GetPinToken(byte cborCommand, byte subCommand, string pinCur, byte[] agreementKeyCBOR)
        {
            // 公開鍵から共通鍵を生成
            if (CreateSharedSecretKey(agreementKeyCBOR) == false) {
                return null;
            }

            // pinHashEncを生成
            PinHashEnc = CreatePinHashEnc(pinCur, SharedSecretKey);

            // 送信データをCBORエンコードしたバイト配列を戻す
            return GenerateGetPinTokenCbor(cborCommand, subCommand);
        }

        private bool CreateSharedSecretKey(byte[] agreementKeyCBOR)
        {
            // CBORをデコードして公開鍵を抽出
            CBORDecoder cborDecoder = new CBORDecoder();
            KeyAgreement agreementKey = cborDecoder.GetKeyAgreement(agreementKeyCBOR);
            byte[] aG_x = agreementKey.X;
            byte[] aG_y = agreementKey.Y;

            // for debug
            // AppCommon.OutputLogToFile("Public key: ", true);
            // AppCommon.OutputLogToFile(AppCommon.DumpMessage(aG_x, aG_x.Length), false);
            // AppCommon.OutputLogToFile(AppCommon.DumpMessage(aG_y, aG_y.Length), false);

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

            // 公開鍵をモジュール内で退避
            AgreementPublicKey = new KeyAgreement(2, -7, 1, bG_x, bG_y);

            // ヘッダーと公開鍵のx, yを連結し、
            // CngKeyを作成するための公開鍵を生成
            byte[] header = new byte[] { 0x45, 0x43, 0x4b, 0x31, 0x20, 0x00, 0x00, 0x00 }; // 'ECK1'
            byte[] publicKeyforChgKey = header.Concat(aG_x).Concat(aG_y).ToArray();

            // 共通鍵を生成
            SharedSecretKey = ecdh.DeriveKeyMaterial(CngKey.Import(publicKeyforChgKey, CngKeyBlobFormat.EccPublicBlob));
            if (SharedSecretKey.Length != 32) {
                return false;
            }

            // for debug
            // AppCommon.OutputLogToFile("Shared secret key: ", true);
            // AppCommon.OutputLogToFile(AppCommon.DumpMessage(SharedSecretKey, SharedSecretKey.Length), false);
            
            return true;
        }

        private byte[] CreatePinHashEnc(string curPin, byte[] sharedSecret)
        {
            // curPin のハッシュを生成  SHA-256(curPin)
            byte[] pinbyte = Encoding.ASCII.GetBytes(curPin);
            SHA256 sha = new SHA256CryptoServiceProvider();
            byte[] pinsha = sha.ComputeHash(pinbyte);

            // 先頭16バイトを抽出  LEFT(SHA-256(curPin),16)
            byte[] pinsha16 = pinsha.ToList().Skip(0).Take(16).ToArray();

            // for debug
            // AppCommon.OutputLogToFile("pinHash: ", true);
            // AppCommon.OutputLogToFile(AppCommon.DumpMessage(pinsha16, pinsha16.Length), false);

            // AES256-CBCで暗号化  
            //   AES256-CBC(sharedSecret, IV=0, LEFT(SHA-256(curPin),16))
            byte[] pinHashEnc = AES256CBCEncrypt(sharedSecret, pinsha16);

            // for debug
            // AppCommon.OutputLogToFile("pinHashEnc: ", true);
            // AppCommon.OutputLogToFile(AppCommon.DumpMessage(pinHashEnc, pinHashEnc.Length), false);

            return pinHashEnc;
        }

        private void CreateNewPinEnc(string pinNew)
        {
            byte[] newPinBytes = PaddingPin64(pinNew);
            NewPinEnc = AES256CBCEncrypt(SharedSecretKey, newPinBytes);
        }

        private byte[] PaddingPin64(string pin)
        {
            // PINを64バイトの配列に転化する。
            // （64バイトに満たない部分は０埋め）
            byte[] pinBytes = new byte[64];
            byte[] pintmp = Encoding.ASCII.GetBytes(pin);
            for (int i = 0; i < pinBytes.Length; i++) {
                if (i < pintmp.Length) {
                    pinBytes[i] = pintmp[i];
                } else {
                    pinBytes[i] = 0;
                }
            }
            return pinBytes;
        }

        private byte[] AES256CBCEncrypt(byte[] key, byte[] data)
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

        private void CreatePinAuth(byte[] newPinEnc, byte[] pinHashEnc)
        {
            // ハッシュ化対象データを生成
            List<byte> dataForHash = new List<byte>();
            dataForHash.AddRange(newPinEnc.ToArray());
            if (pinHashEnc != null) {
                dataForHash.AddRange(pinHashEnc.ToArray());
            }

            // LEFT(HMAC-SHA-256(sharedSecret, newPinEnc), 16)
            using (var hmacsha256 = new HMACSHA256(SharedSecretKey)) {
                byte[] digest = hmacsha256.ComputeHash(dataForHash.ToArray());
                PinAuth = digest.ToList().Take(16).ToArray();
            }
        }

        private byte[] GenerateSetPinCbor(byte cborCommand, byte subCommand)
        {
            // 送信データを生成
            //   0x01: pinProtocol
            //   0x02: subCommand
            //   0x03: keyAgreement
            //   0x04: pinAuth
            //   0x05: newPinEnc
            //   0x06: pinHashEnc
            CBORObject cbor = CBORObject.NewMap();
            cbor.Add(0x01, 1);
            cbor.Add(0x02, subCommand);

            CBORObject keyParam = CBORObject.NewMap();
            keyParam.Add(1, AgreementPublicKey.Kty);
            keyParam.Add(3, AgreementPublicKey.Alg);
            keyParam.Add(-1, AgreementPublicKey.Crv);
            keyParam.Add(-2, AgreementPublicKey.X);
            keyParam.Add(-3, AgreementPublicKey.Y);
            cbor.Add(0x03, keyParam);

            cbor.Add(0x04, PinAuth);
            cbor.Add(0x05, NewPinEnc);

            if (PinHashEnc != null) {
                cbor.Add(0x06, PinHashEnc);
            }

            // エンコードを実行
            byte[] payload = cbor.EncodeToBytes();
            byte[] encoded = new byte[] { cborCommand }.Concat(payload).ToArray();

            // for debug
            // AppCommon.OutputLogToFile("Encoded CBOR request: ", true);
            // AppCommon.OutputLogToFile(AppCommon.DumpMessage(encoded, encoded.Length), false);

            // エンコードされたCBORバイト配列を戻す
            return encoded;
        }

        private byte[] GenerateGetPinTokenCbor(byte cborCommand, byte subCommand)
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
            keyParam.Add(1, AgreementPublicKey.Kty);
            keyParam.Add(3, AgreementPublicKey.Alg);
            keyParam.Add(-1, AgreementPublicKey.Crv);
            keyParam.Add(-2, AgreementPublicKey.X);
            keyParam.Add(-3, AgreementPublicKey.Y);
            cbor.Add(0x03, keyParam);
            cbor.Add(0x06, PinHashEnc);

            // エンコードを実行
            byte[] payload = cbor.EncodeToBytes();
            byte[] encoded = new byte[] { cborCommand }.Concat(payload).ToArray();

            // for debug
            // AppCommon.OutputLogToFile("Encoded CBOR request: ", true);
            // AppCommon.OutputLogToFile(AppCommon.DumpMessage(encoded, encoded.Length), false);

            // エンコードされたCBORバイト配列を戻す
            return encoded;
        }

        //
        // ヘルスチェック実行用のテストデータ
        // 
        byte[] challenge = Encoding.ASCII.GetBytes("This is challenge");
        string rpid = "diverta.co.jp";
        string rpname = "Diverta inc.";
        byte[] userid = Encoding.ASCII.GetBytes("1234567890123456");
        string username = "username";
        string displayName = "User Name";

        public byte[] MakeCredential(byte cborCommand, string pinCur, byte[] pinTokenCBOR, byte[] sharedSecretKey)
        {
            // pinTokenを抽出
            byte[] pinToken = ExtractPinToken(pinTokenCBOR, sharedSecretKey);
            if (pinToken == null) {
                return null;
            }

            // clientDataHashを生成
            byte[] clientDataHash = CreateClientDataHash(challenge);

            // pinAuthを生成
            byte[] pinAuth = CreateClientPinAuth(pinToken, clientDataHash);

            // hmac-secret機能で使用する64バイトSaltを生成
            CreateRandomSalt();

            // 送信データをCBORエンコードしたバイト配列を戻す
            return GenerateMakeCredentialCbor(cborCommand, clientDataHash, pinAuth);
        }

        public byte[] GetAssertion(byte cborCommand, string pinCur, byte[] pinTokenCBOR, byte[] sharedSecretKey, CreateOrGetCommandResponse makeCredentialRes, KeyAgreement agreementPublicKey, bool testUserPresenceNeeded)
        {
            // pinTokenを抽出
            byte[] pinToken = ExtractPinToken(pinTokenCBOR, sharedSecretKey);
            if (pinToken == null) {
                return null;
            }

            // clientDataHashを生成
            byte[] clientDataHash = CreateClientDataHash(challenge);

            // pinAuthを生成
            byte[] pinAuth = CreateClientPinAuth(pinToken, clientDataHash);

            // saltEncを生成
            byte[] saltEnc = CreateSaltEnc(sharedSecretKey, Salt);

            // saltAuthを生成
            byte[] saltAuth = CreateSaltAuth(sharedSecretKey, saltEnc);

            // 送信データをCBORエンコードしたバイト配列を戻す
            byte[] credentialID = makeCredentialRes.CredentialId;
            return GenerateGetAssertionCbor(cborCommand, clientDataHash, credentialID, pinAuth, 
                agreementPublicKey, saltEnc, saltAuth, testUserPresenceNeeded);
        }

        private byte[] ExtractPinToken(byte[] pinTokenCBOR, byte[] sharedSecretKey)
        {
            // pinTokenをCBORから抽出
            CBORDecoder cborDecoder = new CBORDecoder();
            byte[] pinTokenEnc = cborDecoder.GetPinTokenEnc(pinTokenCBOR);
            if (pinTokenEnc == null) {
                AppCommon.OutputLogToFile("Extract encrypted pinToken fail", true);
                return null;
            }

            // pinTokenを共通鍵で復号化
            return AES256CBCDecrypt(sharedSecretKey, pinTokenEnc);
        }

        private byte[] CreateClientDataHash(byte[] challenge)
        {
            SHA256 sha = new SHA256CryptoServiceProvider();
            return sha.ComputeHash(challenge);
        }

        private byte[] CreateClientPinAuth(byte[] pinToken, byte[] clientDataHash)
        {
            // LEFT(HMAC-SHA-256(pinToken, clientDataHash), 16)
            byte[] pinAuth = null;
            using (var hmacsha256 = new HMACSHA256(pinToken)) {
                byte[] digest = hmacsha256.ComputeHash(clientDataHash);
                pinAuth = digest.ToList().Take(16).ToArray();
            }
            return pinAuth;
        }

        private byte[] AES256CBCDecrypt(byte[] key, byte[] data)
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

        public void CreateRandomSalt()
        {
            // 64バイト salt（ランダム値）を生成しておく
            random.NextBytes(Salt);
            // for debug
            AppCommon.OutputLogToFile("Generated Salt: ", true);
            AppCommon.OutputLogToFile(AppCommon.DumpMessage(Salt, Salt.Length), false);
        }

        private byte[] CreateSaltEnc(byte[] sharedSecret, byte[] salt)
        {
            // AES256-CBCで暗号化  
            //   AES256-CBC(sharedSecret, IV=0, salt)
            byte[] saltEnc = AES256CBCEncrypt(sharedSecret, salt);
            return saltEnc;
        }

        private byte[] CreateSaltAuth(byte[] sharedSecret, byte[] saltEnc)
        {
            // LEFT(HMAC-SHA-256(sharedSecret, newPinEnc), 16)
            var hmacsha256 = new HMACSHA256(sharedSecret);
            byte[] digest = hmacsha256.ComputeHash(saltEnc);
            return digest.ToList().Take(16).ToArray();
        }

        private byte[] GenerateMakeCredentialCbor(byte cborCommand, byte[] clientDataHash, byte[] pinAuth)
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
            cbor.Add(0x02, CBORObject.NewMap().Add("id", rpid).Add("name", rpname));

            var user = CBORObject.NewMap();
            user.Add("id", userid);
            user.Add("name", username);
            user.Add("displayName", displayName);
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
            // AppCommon.OutputLogToFile("Encoded CBOR request(MakeCredential): ", true);
            // AppCommon.OutputLogToFile(AppCommon.DumpMessage(encoded, encoded.Length), false);

            // エンコードされたCBORバイト配列を戻す
            return encoded;
        }

        private byte[] GenerateGetAssertionCbor(
            byte cborCommand, byte[] clientDataHash, byte[] credentialId, byte[] pinAuth, 
            KeyAgreement agreementPublicKey, byte[] saltEnc, byte[] saltAuth,
            bool testUserPresenceNeeded)
        {
            // 送信データを生成
            //   0x01: rpid
            //   0x02: clientDataHash
            //   0x03: allowList
            //   0x04: extensions
            //   0x05: options
            //   0x06: pinAuth
            //   0x07: pinProtocol
            CBORObject cbor = CBORObject.NewMap();
            cbor.Add(0x01, rpid);
            cbor.Add(0x02, clientDataHash);

            CBORObject pubKeyCredParams = CBORObject.NewMap();
            pubKeyCredParams.Add("type", "public-key");
            pubKeyCredParams.Add("id", credentialId);
            cbor.Add(0x03, CBORObject.NewArray().Add(pubKeyCredParams));

            CBORObject extensions = CreateExtensionsCBORObject(agreementPublicKey, saltEnc, saltAuth);
            cbor.Add(0x04, extensions);

            var opt = CBORObject.NewMap();
            opt.Add("rk", false);
            opt.Add("uv", false);
            opt.Add("up", testUserPresenceNeeded);
            cbor.Add(0x05, opt);

            if (pinAuth != null) {
                cbor.Add(0x06, pinAuth);
                cbor.Add(0x07, 1);
            }

            // エンコードを実行
            byte[] payload = cbor.EncodeToBytes();
            byte[] encoded = new byte[] { cborCommand }.Concat(payload).ToArray();

            // for debug
            // AppCommon.OutputLogToFile("Encoded CBOR request(GetAssertion): ", true);
            // AppCommon.OutputLogToFile(AppCommon.DumpMessage(encoded, encoded.Length), false);

            // エンコードされたCBORバイト配列を戻す
            return encoded;
        }

        CBORObject CreateExtensionsCBORObject(KeyAgreement agreementPublicKey, byte[] saltEnc, byte[] saltAuth)
        {
            CBORObject keyParam = CBORObject.NewMap();
            keyParam.Add(1, agreementPublicKey.Kty);
            keyParam.Add(3, agreementPublicKey.Alg);
            keyParam.Add(-1, agreementPublicKey.Crv);
            keyParam.Add(-2, agreementPublicKey.X);
            keyParam.Add(-3, agreementPublicKey.Y);

            CBORObject hmacSecret = CBORObject.NewMap();
            hmacSecret.Add(0x01, keyParam);
            hmacSecret.Add(0x02, saltEnc);
            hmacSecret.Add(0x03, saltAuth);

            CBORObject extensions = CBORObject.NewMap();
            extensions.Add("hmac-secret", hmacSecret);

            return extensions;
        }
    }
}
