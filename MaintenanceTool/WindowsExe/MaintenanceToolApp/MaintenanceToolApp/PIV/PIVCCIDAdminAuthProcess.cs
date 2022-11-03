using System;
using System.Linq;
using System.Security.Cryptography;
using ToolAppCommon;

namespace MaintenanceToolApp.PIV
{
    internal class PIVCCIDAdminAuthProcess
    {
        // Challenge（往路／復路用）を保持
        private byte[] AuthChallenge = null!;

        // CCID I/Fからデータ受信時のコールバックを保持
        public delegate void HandlerOnCommandResponse(bool success, string errorMessage);
        private HandlerOnCommandResponse OnCommandResponse = null!;

        // 乱数製造用
        private readonly Random random = new Random();

        //
        // PIV機能設定用関数
        // 
        public void DoPIVCcidCommand(HandlerOnCommandResponse handlerRef)
        {
            // コールバックを保持
            OnCommandResponse = handlerRef;

            // PIV管理機能認証（往路）を実行
            DoRequestPivAdminAuth();
        }

        private void DoRequestPivAdminAuth()
        {
            // PIV管理機能認証（往路）のリクエストデータを生成
            byte[] apdu = { PIVConst.TAG_DYNAMIC_AUTH_TEMPLATE, 2, PIVConst.TAG_AUTH_WITNESS, 0 };
            AuthChallenge = Array.Empty<byte>();

            // コマンドを実行
            // 0x03: CRYPTO_ALG_3DES
            CCIDParameter param = new CCIDParameter(PIVCCIDConst.PIV_INS_AUTHENTICATE, 0x03, PIVConst.PIV_KEY_CARDMGM, apdu, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponsePIVAdminAuth);
        }

        private void DoRequestPivAdminAuthSecond(byte[] responseData)
        {
            // PIV管理機能認証（往路）のレスポンスから、暗号化された受信チャレンジを抽出（５バイト目から８バイト分）
            byte[] encrypted = new byte[8];
            Array.Copy(responseData, 4, encrypted, 0, 8);

            // PIV管理パスワードを使用し、受信チャレンジを復号化
            byte[] witness;
            string errorMessage;
            if (DecryptPivAdminAuthWitness(encrypted, out witness, out errorMessage) == false) {
                // エラーが発生時は制御を戻す
                OnCommandResponse(false, errorMessage);
                return;
            }

            // 8バイトのランダムベクターを送信チャレンジに設定
            byte[] PivAuthChallenge = new byte[8];
            random.NextBytes(PivAuthChallenge);

            // PIV管理機能認証（復路）のリクエストデータを生成
            byte[] apdu = new byte[22];
            int offset = 0;
            apdu[offset++] = PIVConst.TAG_DYNAMIC_AUTH_TEMPLATE;
            apdu[offset++] = 20;
            // copy witness
            apdu[offset++] = PIVConst.TAG_AUTH_WITNESS;
            apdu[offset++] = (byte)witness.Length;
            Array.Copy(witness, 0, apdu, offset, witness.Length);
            // copy challenge
            offset += witness.Length;
            apdu[offset++] = PIVConst.TAG_AUTH_CHALLENGE;
            apdu[offset++] = (byte)PivAuthChallenge.Length;
            Array.Copy(PivAuthChallenge, 0, apdu, offset, PivAuthChallenge.Length);

            // パラメーターに保持
            AuthChallenge = PivAuthChallenge;

            // コマンドを実行
            // 0x03: CRYPTO_ALG_3DES
            CCIDParameter param = new CCIDParameter(PIVCCIDConst.PIV_INS_AUTHENTICATE, 0x03, PIVConst.PIV_KEY_CARDMGM, apdu, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponsePIVAdminAuth);
        }

        private void DoResponsePIVAdminAuth(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (responseSW != CCIDProcessConst.SW_SUCCESS) {
                if (AuthChallenge == Array.Empty<byte>()) {
                    OnCommandResponse(false, AppCommon.MSG_ERROR_PIV_ADMIN_AUTH_REQ_FAILED);
                } else {
                    OnCommandResponse(false, AppCommon.MSG_ERROR_PIV_ADMIN_AUTH_RES_FAILED);
                }
                return;
            }

            if (AuthChallenge == Array.Empty<byte>()) {
                // PIV管理機能認証（復路）を実行
                DoRequestPivAdminAuthSecond(responseData);
                return;
            }

            // 送受信チャレンジの内容一致チェック
            // PIV管理機能認証（復路）のレスポンスから、暗号化された受信チャレンジを抽出（５バイト目から８バイト分）
            byte[] encrypted = new byte[8];
            Array.Copy(responseData, 4, encrypted, 0, 8);

            // PIV管理パスワードを使用し、受信チャレンジを復号化
            byte[] witness;
            string errorMessage;
            if (DecryptPivAdminAuthWitness(encrypted, out witness, out errorMessage) == false) {
                // エラーが発生時は制御を戻す
                OnCommandResponse(false, errorMessage);
                return;
            }

            // 送信チャレンジと受信チャレンジの内容が異なる場合はPIV管理認証失敗
            if (AuthChallenge.SequenceEqual(witness) == false) {
                OnCommandResponse(false, AppCommon.MSG_ERROR_PIV_ADMIN_AUTH_CHALLENGE_DIFF);
                return;
            }

            // 管理機能認証が成功
            OnCommandResponse(true, AppCommon.MSG_NONE);
        }

        //
        // Triple DES復号化関連
        //
        // デフォルトの3DES鍵
        private static byte[] Default3desKey = {
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
        };

        private static bool DecryptPivAdminAuthWitness(byte[] encrypted, out byte[] transformedBytes, out string errorMessage)
        {
            // エラーメッセージを初期化
            errorMessage = AppCommon.MSG_NONE;

            // Triple DESで復号化
            bool ret = false;
            try {
                DES des = DES.Create();
                des.Mode = CipherMode.ECB;
                des.Padding = PaddingMode.None;
                byte[] src = new byte[8];
                Array.Copy(encrypted, src, src.Length);

                des.Key = Default3desKey.Take(8).ToArray();
                ICryptoTransform ct1 = des.CreateDecryptor();
                transformedBytes = ct1.TransformFinalBlock(src, 0, src.Length);
                des.Clear();
                Array.Copy(transformedBytes, src, src.Length);

                des.Key = Default3desKey.Skip(8).Take(8).ToArray();
                ICryptoTransform ct2 = des.CreateEncryptor();
                transformedBytes = ct2.TransformFinalBlock(src, 0, src.Length);
                des.Clear();
                Array.Copy(transformedBytes, src, src.Length);

                des.Key = Default3desKey.Skip(16).Take(8).ToArray();
                ICryptoTransform ct3 = des.CreateDecryptor();
                transformedBytes = ct3.TransformFinalBlock(src, 0, src.Length);
                des.Clear();

                // 正常終了
                ret = true;

            } catch (Exception e) {
                // エラーメッセージを保持
                errorMessage = string.Format(AppCommon.MSG_ERROR_PIV_CERT_INFO_GET_FAILED, e.Message);
                transformedBytes = Array.Empty<byte>();
            }

            return ret;
        }
    }
}
