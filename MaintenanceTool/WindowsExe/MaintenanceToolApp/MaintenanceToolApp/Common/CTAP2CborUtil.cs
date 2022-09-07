using PeterO.Cbor;
using System.Linq;
using ToolAppCommon;

namespace MaintenanceToolApp.Common
{
    internal class CBOREncoder
    {
        // デバッグログ出力
        private static readonly bool OutputDebugLog = false;

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

            if (OutputDebugLog) {
                AppLogUtil.OutputLogDebug(string.Format(
                    "Encoded CBOR request: \n{0}", AppLogUtil.DumpMessage(encoded, encoded.Length)));
            }

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

            if (OutputDebugLog) {
                AppLogUtil.OutputLogDebug(string.Format(
                    "Encoded CBOR request(MakeCredential): \n{0}", AppLogUtil.DumpMessage(encoded, encoded.Length)));
            }

            // エンコードされたCBORバイト配列を戻す
            return encoded;
        }

        public static byte[] GenerateGetAssertionCbor(
            byte cborCommand, string rpid, byte[] clientDataHash, byte[] credentialId, byte[] pinAuth,
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

        private static CBORObject CreateExtensionsCBORObject(KeyAgreement agreementPublicKey, byte[] saltEnc, byte[] saltAuth)
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

        public static MakeOrGetCommandResponse ParseMakeOrGetCommandResponse(byte[] cborBytes, bool makeCredential)
        {
            MakeOrGetCommandResponse response = new MakeOrGetCommandResponse();
            CBORObject cbor = CBORObject.DecodeFromBytes(cborBytes, CBOREncodeOptions.Default);
            foreach (CBORObject key in cbor.Keys) {
                var keyVal = key.AsByte();
                if (keyVal == 0x02) {
                    // authData
                    ParseAuthData(cbor[key].GetByteString(), response, makeCredential);
                }
            }
            return response;
        }

        private static void ParseAuthData(byte[] data, MakeOrGetCommandResponse response, bool makeCredential)
        {
            int index = 0;
            int size;

            // rpIdHash
            size = MakeOrGetCommandResponse.RpIdHashSize;
            response.RpIdHash = data.Skip(index).Take(size).ToArray();
            index += size;

            // flags
            response.Flags = data[index];
            index++;

            // signCount（エンディアン変換が必要）
            response.SignCount = AppUtil.ToInt32(data, index, true);
            index += MakeOrGetCommandResponse.SignCountSize;

            if (makeCredential) {
                // aaguid
                size = MakeOrGetCommandResponse.AaguidSize;
                response.Aaguid = data.Skip(index).Take(16).ToArray();
                index += size;

                // credentialIdLength（エンディアン変換が必要）
                response.CredentialIdLength = AppUtil.ToInt16(data, index, true);
                index += MakeOrGetCommandResponse.CredentialIdLengthSize;

                // CredentialId
                size = response.CredentialIdLength;
                response.CredentialId = data.Skip(index).Take(size).ToArray();
                index += size;


                // credentialPublicKey
                size = MakeOrGetCommandResponse.CredentialPublicKeySize;
                response.CredentialPublicKeyByte = data.Skip(index).Take(size).ToArray();
                index += size;
            }

            // extensions CBORが付加されていない場合は終了
            size = makeCredential ?
                MakeOrGetCommandResponse.ExtensionsCBORForCreateSize :
                MakeOrGetCommandResponse.ExtensionsCBORForGetSize;
            if (data.Length - index < size) {
                response.HmacSecretRes = new ExtHmacSecretResponse();
                return;
            }

            // extensions CBOR
            byte[] extensionsCBORByte = data.Skip(index).Take(size).ToArray();
            ParseExtHmacSecretData(extensionsCBORByte, response);
        }

        public static void ParseExtHmacSecretData(byte[] cborBytes, MakeOrGetCommandResponse response)
        {
            // hmac-secret拡張データを初期化
            response.HmacSecretRes = new ExtHmacSecretResponse();
            response.HmacSecretRes.Flag = false;
            response.HmacSecretRes.Output = new byte[0];

            // hmac-secret拡張データを解析
            CBORObject cbor = CBORObject.DecodeFromBytes(cborBytes, CBOREncodeOptions.Default);
            foreach (CBORObject key in cbor.Keys) {
                string keyVal = key.AsString();
                if (keyVal == "hmac-secret") {
                    CBORObject value = cbor[key];
                    if (value.Type == CBORType.Boolean) {
                        response.HmacSecretRes.Flag = value.AsBoolean();
                    } else if (value.Type == CBORType.ByteString) {
                        response.HmacSecretRes.Output = value.GetByteString();
                    }
                }
            }
        }
    }
}
