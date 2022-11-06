using System;
using System.Text;
using ToolAppCommon;

namespace MaintenanceToolApp.PIV
{
    internal class PIVCCIDPinAuthProcess
    {
        // 処理実行のためのプロパティー
        private PIVParameter Parameter = null!;

        // CCID I/Fからデータ受信時のコールバックを保持
        public delegate void HandlerOnCommandResponse(bool success, string errorMessage);
        private HandlerOnCommandResponse OnCommandResponse = null!;

        //
        // PIV機能設定用関数
        // 
        public void DoPIVCcidCommand(PIVParameter parameter, HandlerOnCommandResponse handlerRef)
        {
            // 引き渡されたパラメーターを退避
            Parameter = parameter;

            // コールバックを保持
            OnCommandResponse = handlerRef;

            // PIN番号による認証を実行
            DoRequestPivPinVerify(Parameter.AuthPin);
        }

        private void DoRequestPivPinVerify(string pinCode)
        {
            // PINを設定（８文字に足りない場合は、足りない部分を0xffで埋める）
            byte[] apdu = GeneratePinBytes(pinCode);

            // コマンドを実行
            CCIDParameter param = new CCIDParameter(PIVCCIDConst.PIV_INS_VERIFY, 0x00, PIVCCIDConst.PIV_KEY_PIN, apdu, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponsePIVPinVerify);
        }

        private void DoResponsePIVPinVerify(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false) {
                OnCommandResponse(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            // ステータスワードをチェックし、PIN認証の成否を判定
            string errorMessage;
            if (CheckPIVPinVerifyResponseSW(responseSW, out errorMessage) == false) {
                // PIN認証が失敗した場合は処理終了
                OnCommandResponse(false, errorMessage);
                return;
            }

            // PIN認証成功
            OnCommandResponse(true, AppCommon.MSG_NONE);
        }

        //
        // PIN認証応答チェック処理
        //
        public static bool CheckPIVPinVerifyResponseSW(UInt16 responseSW, out string errorMessage)
        {
            return CheckPIVPinOrPukVerifyResponseSW(responseSW, true, out errorMessage);
        }

        public static bool CheckPIVPukVerifyResponseSW(UInt16 responseSW, out string errorMessage)
        {
            return CheckPIVPinOrPukVerifyResponseSW(responseSW, false, out errorMessage);
        }

        private static bool CheckPIVPinOrPukVerifyResponseSW(UInt16 responseSW, bool isPinAuth, out string errorMessage)
        {
            // エラーメッセージを初期化
            errorMessage = AppCommon.MSG_NONE;

            // ステータスワードをチェックし、エラーの種類を判定
            int retries = 3;
            bool isPinBlocked = false;
            if ((responseSW >> 8) == 0x63) {
                // リトライカウンターが戻された場合（入力PIN／PUKが不正時）
                retries = responseSW & 0x000f;
                if (retries < 1) {
                    isPinBlocked = true;
                }

            } else if (responseSW == CCIDProcessConst.SW_ERR_AUTH_BLOCKED) {
                // 入力PIN／PUKがすでにブロックされている場合
                isPinBlocked = true;

            } else if (responseSW != CCIDProcessConst.SW_SUCCESS) {
                // 不明なエラーが発生時
                errorMessage = string.Format(AppCommon.MSG_ERROR_PIV_UNKNOWN, responseSW);
            }
            // PINブロック or リトライカウンターの状態に応じメッセージを編集
            if (isPinBlocked) {
                errorMessage = isPinAuth ? AppCommon.MSG_ERROR_PIV_PIN_LOCKED : AppCommon.MSG_ERROR_PIV_PUK_LOCKED;

            } else if (retries < 3) {
                string name = isPinAuth ? "PIN" : "PUK";
                errorMessage = string.Format(AppCommon.MSG_ERROR_PIV_WRONG_PIN, name, name, retries);
            }

            return (responseSW == CCIDProcessConst.SW_SUCCESS);
        }

        //
        // ユーティリティー
        //
        public static byte[] GeneratePinBytes(string pinCode)
        {
            // バイト配列に、引数のPINを設定
            // ８文字に足りない場合は、足りない部分を0xffで埋める
            byte[] apdu = new byte[8];
            byte[] pinCodeBytes = Encoding.ASCII.GetBytes(pinCode);
            for (int i = 0; i < apdu.Length; i++) {
                if (i < pinCodeBytes.Length) {
                    apdu[i] = pinCodeBytes[i];
                } else {
                    apdu[i] = 0xff;
                }
            }
            return apdu;
        }
    }
}
