using MaintenanceToolCommon;
using System;

namespace MaintenanceToolGUI
{
    class U2f
    {
        internal static class Const
        {
            public const int MSG_HEADER_LEN = 3;
            public const int U2F_INS_REGISTER = 0x01;
            public const int U2F_INS_AUTHENTICATE = 0x02;
            public const int U2F_INS_VERSION = 0x03;
            public const int U2F_AUTH_ENFORCE = 0x03;
            public const int U2F_AUTH_CHECK_ONLY = 0x07;
            public const int U2F_APPID_SIZE = 32;
            public const int U2F_NONCE_SIZE = 32;
            public const int U2F_KEYHANDLE_SIZE = 64;

            // BLEコマンドバイトに関する定義
            public const byte BLE_CMD_MSG = 0x83;
        };

        // トランスポート種別を保持
        private byte transportType;

        // トランスポート別処理の参照を保持
        private HIDMain hidMain;
        private BLEMain bleMain;

        // メイン画面の参照を保持
        private MainForm mainForm;

        // 実行機能を保持
        public enum RequestType
        {
            None = 0,
            TestRegister,
            TestAuthenticateCheck,
            TestAuthenticate,
        };
        private RequestType requestType;

        // リクエストデータ格納領域
        private byte[] U2FRequestData = new byte[1024];

        // U2Fキーハンドルデータを保持
        // (ヘルスチェック処理で使用)
        private byte[] u2FKeyhandleData = new byte[128];

        // 生成されたランダムなチャレンジ、AppIDを保持
        // (ヘルスチェック処理で使用)
        private byte[] nonce = new byte[Const.U2F_NONCE_SIZE];
        private byte[] appid = new byte[Const.U2F_APPID_SIZE];
        private Random random = new Random();

        public U2f(MainForm m, byte transportType_)
        {
            mainForm = m;
            transportType = transportType_;
        }

        public void SetHidMain(HIDMain p)
        {
            hidMain = p;
        }

        public void SetBleMain(BLEMain a)
        {
            bleMain = a;
        }

        public void SetRequestType(RequestType t)
        {
            requestType = t;
        }

        // 
        // レスポンスの後続処理判定
        //
        public void DoResponse(byte[] receivedMessage, int receivedLen)
        {
            if (CheckStatusWord(receivedMessage, receivedLen) == false) {
                // 処理結果が不正の場合は画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                return;
            }

            switch (requestType) {
            case RequestType.TestRegister:
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_U2F_REGISTER_SUCCESS);
                DoRequestAuthenticate(receivedMessage, receivedLen, RequestType.TestAuthenticateCheck);
                break;
            case RequestType.TestAuthenticateCheck:
                DoRequestAuthenticate(receivedMessage, receivedLen, RequestType.TestAuthenticate);
                break;
            case RequestType.TestAuthenticate:
                // 画面に制御を戻す
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_U2F_AUTHENTICATE_SUCCESS);
                mainForm.OnAppMainProcessExited(true);
                break;
            default:
                break;
            }
        }

        private bool CheckStatusWord(byte[] receivedMessage, int receivedLen)
        {
            //
            // U2F関連コマンドの場合は
            // ステータスワードチェックを行う。
            //
            byte[] statusBytes = new byte[2];
            Array.Copy(receivedMessage, receivedLen - 2, statusBytes, 0, 2);
            if (BitConverter.IsLittleEndian) {
                Array.Reverse(statusBytes);
            }
            ushort statusWord = BitConverter.ToUInt16(statusBytes, 0);

            if (statusWord == 0x6985) {
                // キーハンドルチェックの場合は成功とみなす
                return true;
            }
            if (statusWord == 0x6a80) {
                // invalid keyhandleエラーである場合はその旨を通知
                mainForm.OnPrintMessageText(AppCommon.MSG_OCCUR_KEYHANDLE_ERROR);
                return false;
            }
            if (statusWord == 0x9402) {
                // 鍵・証明書がインストールされていない旨のエラーである場合はその旨を通知
                mainForm.OnPrintMessageText(AppCommon.MSG_OCCUR_SKEYNOEXIST_ERROR);
                return false;
            }
            if (statusWord == 0x9601) {
                // ペアリングモード時はペアリング以外の機能を実行できない旨を通知
                mainForm.OnPrintMessageText(AppCommon.MSG_OCCUR_PAIRINGMODE_ERROR);
                return false;
            }
            if (statusWord != 0x9000) {
                // U2Fサービスの戻りコマンドが不正の場合はエラー
                mainForm.OnPrintMessageText(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return false;
            }

            return true;
        }

        //
        // U2F Registerコマンド関連処理
        //
        public void DoRequestRegister()
        {
            // ランダムなチャレンジデータを生成
            GenerateNonceBytes();
            GenerateAppIDBytes();

            // リクエストデータ（APDU）を編集しリクエストデータに格納
            int length = GenerateU2FRegisterBytes(U2FRequestData);

            // リクエスト種別を設定
            requestType = RequestType.TestRegister;

            // U2F Registerコマンドを実行
            switch (transportType) {
            case AppCommon.TRANSPORT_HID:
                hidMain.SendHIDMessage(Const.BLE_CMD_MSG, U2FRequestData, length);
                break;
            case AppCommon.TRANSPORT_BLE:
                bleMain.SendBLEMessage(Const.BLE_CMD_MSG, U2FRequestData, length);
                break;
            default:
                break;
            }
        }

        private void GenerateNonceBytes()
        {
            // チャレンジにランダム値を設定
            random.NextBytes(nonce);

            // 生成されたランダム値をダンプ
            string dumpNonce = AppCommon.DumpMessage(nonce, nonce.Length);
            AppCommon.OutputLogToFile(string.Format("doHealthCheck: Challenge\r\n{0}", dumpNonce), true);
        }

        private void GenerateAppIDBytes()
        {
            // AppIDにランダム値を設定
            random.NextBytes(appid);

            // 生成されたランダム値をダンプ
            string dumpAppid = AppCommon.DumpMessage(appid, appid.Length);
            AppCommon.OutputLogToFile(string.Format("doHealthCheck: AppId\r\n{0}", dumpAppid), true);
        }

        private int GenerateU2FRegisterBytes(byte[] u2fRequestData)
        {
            int pos;

            // リクエストデータを配列にセット
            u2fRequestData[0] = 0x00;
            u2fRequestData[1] = Const.U2F_INS_REGISTER;
            u2fRequestData[2] = 0x00;
            u2fRequestData[3] = 0x00;
            u2fRequestData[4] = 0x00;
            u2fRequestData[5] = 0x00;
            u2fRequestData[6] = Const.U2F_NONCE_SIZE + Const.U2F_APPID_SIZE;

            // challengeを設定
            pos = 7;
            Array.Copy(nonce, 0, u2fRequestData, pos, Const.U2F_NONCE_SIZE);
            pos += Const.U2F_NONCE_SIZE;

            // appIdを設定
            Array.Copy(appid, 0, u2fRequestData, pos, Const.U2F_APPID_SIZE);
            pos += Const.U2F_APPID_SIZE;

            // Leを設定
            u2fRequestData[pos++] = 0x00;
            u2fRequestData[pos++] = 0x00;

            return pos;
        }

        //
        // U2F Authenticateコマンド関連処理
        //
        private void DoRequestAuthenticate(byte[] receivedMessage, int receivedLen, RequestType type)
        {
            // Registerレスポンスからキーハンドル
            // (68バイト目から64バイト)を切り出して保持
            Array.Copy(receivedMessage, 67, u2FKeyhandleData, 0, Const.U2F_KEYHANDLE_SIZE);

            // ランダムなチャレンジデータを生成
            GenerateNonceBytes();

            // リクエストデータ（APDU）を編集しリクエストデータに格納
            int length = GenerateU2FAuthenticateBytes(U2FRequestData, getAuthOption(type));

            if (type == RequestType.TestAuthenticate) {
                // BLE U2Fリクエスト転送の前に、
                // FIDO認証器のMAIN SWを押してもらうように促す
                // メッセージを画面表示
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_U2F_AUTHENTICATE_START);
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_U2F_AUTHENTICATE_COMMENT1);
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_U2F_AUTHENTICATE_COMMENT2);
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_U2F_AUTHENTICATE_COMMENT3);
            }

            // リクエスト種別を設定
            requestType = type;

            // U2F Authenticateコマンドを実行
            switch (transportType) {
            case AppCommon.TRANSPORT_HID:
                hidMain.SendHIDMessage(Const.BLE_CMD_MSG, U2FRequestData, length);
                break;
            case AppCommon.TRANSPORT_BLE:
                bleMain.SendBLEMessage(Const.BLE_CMD_MSG, U2FRequestData, length);
                break;
            default:
                break;
            }
        }

        private int GenerateU2FAuthenticateBytes(byte[] u2fRequestData, byte authOption)
        {
            int pos;

            // リクエストデータを配列にセット
            u2fRequestData[0] = 0x00;
            u2fRequestData[1] = Const.U2F_INS_AUTHENTICATE;
            u2fRequestData[2] = authOption;
            u2fRequestData[3] = 0x00;
            u2fRequestData[4] = 0x00;
            u2fRequestData[5] = 0x00;
            u2fRequestData[6] = Const.U2F_NONCE_SIZE + Const.U2F_APPID_SIZE + Const.U2F_KEYHANDLE_SIZE + 1;

            // challengeを設定
            pos = 7;
            Array.Copy(nonce, 0, u2fRequestData, pos, Const.U2F_NONCE_SIZE);
            pos += Const.U2F_NONCE_SIZE;

            // appIdを設定
            Array.Copy(appid, 0, u2fRequestData, pos, Const.U2F_APPID_SIZE);
            pos += Const.U2F_APPID_SIZE;

            // キーハンドル長を設定
            u2fRequestData[pos++] = Const.U2F_KEYHANDLE_SIZE;

            // キーハンドルを設定
            Array.Copy(u2FKeyhandleData, 0, u2fRequestData, pos, Const.U2F_KEYHANDLE_SIZE);
            pos += Const.U2F_KEYHANDLE_SIZE;

            // Leを設定
            u2fRequestData[pos++] = 0x00;
            u2fRequestData[pos++] = 0x00;

            return pos;
        }

        private byte getAuthOption(RequestType type)
        {
            // 処理区分からオプションを設定
            if (type == RequestType.TestAuthenticateCheck) {
                return Const.U2F_AUTH_CHECK_ONLY;
            } else {
                return Const.U2F_AUTH_ENFORCE;
            }
        }
    }
}
