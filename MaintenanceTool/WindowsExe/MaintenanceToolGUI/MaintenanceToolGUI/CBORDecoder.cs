using PeterO.Cbor;
using System.Linq;
using ToolGUICommon;

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

    internal class ExtHmacSecretResponse
    {
        public bool Flag { get; set; }
        public byte[] Output { get; set; }
        public static int OutputSize = 64;

        public ExtHmacSecretResponse()
        {
        }
    }

    internal class CreateOrGetCommandResponse
    {
        // データ項目
        public byte[] RpIdHash { get; set; }
        public byte Flags { get; set; }
        public int SignCount;
        public byte[] Aaguid { get; set; }
        public int CredentialIdLength;
        public byte[] CredentialId { get; set; }
        public byte[] CredentialPublicKeyByte { get; set; }
        public ExtHmacSecretResponse HmacSecretRes { get; set; }

        // データ長
        public static int RpIdHashSize = 32;
        public static int SignCountSize = 4;
        public static int AaguidSize = 16;
        public static int CredentialIdLengthSize = 2;
        public static int CredentialPublicKeySize = 77;
        public static int ExtensionsCBORForCreateSize = 14;
        public static int ExtensionsCBORForGetSize = 79;

        // for debug
        public static bool OutputDebugLog = false;

        public CreateOrGetCommandResponse()
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

        public CreateOrGetCommandResponse CreateOrGetCommand(byte[] cborBytes, bool makeCredential)
        {
            CreateOrGetCommandResponse response = new CreateOrGetCommandResponse();
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

        private void ParseAuthData(byte[] data, CreateOrGetCommandResponse response, bool makeCredential)
        {
            int index = 0;
            int size;

            // rpIdHash
            size = CreateOrGetCommandResponse.RpIdHashSize;
            response.RpIdHash = data.Skip(index).Take(size).ToArray();
            index += size;

            if (CreateOrGetCommandResponse.OutputDebugLog) {
                string dump = AppUtil.DumpMessage(response.RpIdHash, size);
                AppUtil.OutputLogDebug(string.Format("rpIdHash:\r\n{0}", dump));
            }

            // flags
            response.Flags = data[index];
            index++;

            // signCount（エンディアン変換が必要）
            response.SignCount = AppUtil.ToInt32(data, index, true);
            index += CreateOrGetCommandResponse.SignCountSize;

            if (makeCredential) {
                // aaguid
                size = CreateOrGetCommandResponse.AaguidSize;
                response.Aaguid = data.Skip(index).Take(16).ToArray();
                index += size;

                // credentialIdLength（エンディアン変換が必要）
                response.CredentialIdLength = AppUtil.ToInt16(data, index, true);
                index += CreateOrGetCommandResponse.CredentialIdLengthSize;

                // CredentialId
                size = response.CredentialIdLength;
                response.CredentialId = data.Skip(index).Take(size).ToArray();
                index += size;

                if (CreateOrGetCommandResponse.OutputDebugLog) {
                    string dump = AppUtil.DumpMessage(response.CredentialId, size);
                    AppUtil.OutputLogDebug(string.Format("CredentialId:\r\n{0}", dump));
                }

                // credentialPublicKey
                size = CreateOrGetCommandResponse.CredentialPublicKeySize;
                response.CredentialPublicKeyByte = data.Skip(index).Take(size).ToArray();
                index += size;

                if (CreateOrGetCommandResponse.OutputDebugLog) {
                    string dump = AppUtil.DumpMessage(response.CredentialPublicKeyByte, size);
                    AppUtil.OutputLogDebug(string.Format("CredentialPublicKeyByte:\r\n{0}", dump));
                }
            }

            // extensions CBORが付加されていない場合は終了
            size = makeCredential ? 
                CreateOrGetCommandResponse.ExtensionsCBORForCreateSize : 
                CreateOrGetCommandResponse.ExtensionsCBORForGetSize;
            if (data.Length - index < size) {
                response.HmacSecretRes = null;
                return;
            }

            // extensions CBOR
            byte[] extensionsCBORByte = data.Skip(index).Take(size).ToArray();
            ParseExtHmacSecretData(extensionsCBORByte, response);
        }

        public void ParseExtHmacSecretData(byte[] cborBytes, CreateOrGetCommandResponse response)
        {
            // hmac-secret拡張データを初期化
            response.HmacSecretRes = new ExtHmacSecretResponse();
            response.HmacSecretRes.Flag = false;
            response.HmacSecretRes.Output = null;

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

            if (CreateOrGetCommandResponse.OutputDebugLog) {
                if (response.HmacSecretRes.Flag) {
                    AppUtil.OutputLogDebug(string.Format("HMAC Secret Extension available"));
                }
                if (response.HmacSecretRes.Output != null) {
                    string dump = AppUtil.DumpMessage(
                        response.HmacSecretRes.Output,
                        ExtHmacSecretResponse.OutputSize);
                    AppUtil.OutputLogDebug(string.Format("Extensions 'hmac-secret':\r\n{0}", dump));
                }
            }
        }
    }
}
