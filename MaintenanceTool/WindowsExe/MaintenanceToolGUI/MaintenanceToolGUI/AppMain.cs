using System;
using MaintenanceToolCommon;

namespace MaintenanceToolGUI
{
    internal class AppMain
    {
        private enum BLERequestType
        {   
            None = 0,
            TestRegister,
            TestAuthenticateCheck,
            TestAuthenticate,
            TestBLEPing,
            TestBLECTAP2,
        };
        private BLERequestType bleRequestType = BLERequestType.None;

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
        };

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

        // レスポンスデータ格納領域
        private byte[] BLEResponseData = new byte[1024];
        private int BLEResponseLength = 0;

        // メイン画面の参照を保持
        private MainForm mainForm;

        // BLEデバイス関連
        private BLEProcess bleProcess = new BLEProcess();

        // CTAP2共通処理
        private Ctap2 ctap2;

        public AppMain(MainForm f)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // BLEデバイス関連
            bleProcess.FIDOPeripheralPaired += new BLEProcess.FIDOPeripheralPairedEvent(OnPairedDevice);
            bleProcess.MessageTextEvent += new BLEProcess.MessageTextEventHandler(OnPrintMessageText);
            bleProcess.ReceiveBLEMessageEvent += new BLEProcess.ReceiveBLEMessageEventHandler(OnReceiveBLEMessage);

            AppCommon.OutputLogToFile(String.Format("{0}を起動しました", MainForm.MaintenanceToolTitle), true);

            // CTAP2共通処理に各種参照を引き渡す
            ctap2 = new Ctap2(mainForm, AppCommon.TRANSPORT_BLE);
            ctap2.SetBleMain(this);
        }

        private void OnPrintMessageText(string message)
        {
            // メッセージを画面表示させる
            mainForm.OnPrintMessageText(message);
        }

        public void doPairing()
        {
            bleProcess.PairWithFIDOPeripheral();
        }

        public void OnPairedDevice(bool success, string messageOnFail)
        {
            if (success == false) {
                // BLEペアリングエラー時は、メッセージを画面表示させる
                mainForm.OnPrintMessageText(messageOnFail);
            }
            // メイン画面の参照を経由し、コマンド実行完了時の処理を実行
            mainForm.OnAppMainProcessExited(success);
        }

        private void OnReceiveBLEMessage(bool ret, byte[] receivedMessage, int receivedLen)
        {
            if (CheckStatusWord(receivedMessage, receivedLen) == false) {
                // 処理結果が不正の場合は画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                return;
            }

            switch (bleRequestType) {
            case BLERequestType.TestRegister:
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_U2F_REGISTER_SUCCESS);
                DoTestAuthenticate(ret, receivedMessage, receivedLen, BLERequestType.TestAuthenticateCheck);
                break;
            case BLERequestType.TestAuthenticateCheck:
                DoTestAuthenticate(ret, receivedMessage, receivedLen, BLERequestType.TestAuthenticate);
                break;
            case BLERequestType.TestAuthenticate:
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_U2F_AUTHENTICATE_SUCCESS);
                DoResponse(ret, receivedMessage, receivedLen);
                break;
            case BLERequestType.TestBLEPing:
                DoResponseBLEPing(receivedMessage, receivedLen);
                break;
            case BLERequestType.TestBLECTAP2:
                DoResponseBLECTAP2(receivedMessage, receivedLen);
                break;
            default:
                break;
            }
        }

        private bool CheckStatusWord(byte[] receivedMessage, int receivedLen)
        {
            if (bleRequestType == BLERequestType.TestRegister &&
                bleRequestType == BLERequestType.TestAuthenticateCheck &&
                bleRequestType == BLERequestType.TestAuthenticate) {
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
                    OnPrintMessageText(AppCommon.MSG_OCCUR_KEYHANDLE_ERROR);
                    return false;
                }
                if (statusWord == 0x9402) {
                    // 鍵・証明書がインストールされていない旨のエラーである場合はその旨を通知
                    OnPrintMessageText(AppCommon.MSG_OCCUR_SKEYNOEXIST_ERROR);
                    return false;
                }
                if (statusWord == 0x9601) {
                    // ペアリングモード時はペアリング以外の機能を実行できない旨を通知
                    OnPrintMessageText(AppCommon.MSG_OCCUR_PAIRINGMODE_ERROR);
                    return false;
                }
                if (statusWord != 0x9000) {
                    // U2Fサービスの戻りコマンドが不正の場合はエラー
                    OnPrintMessageText(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                    return false;
                }
            }
            return true;
        }

        private void DoRequest(byte[] requestBytes, int requestLen, BLERequestType type)
        {
            // BLE処理を実行し、メッセージを転送
            bleRequestType = type;
            bleProcess.DoXferMessage(requestBytes, requestLen);
        }

        private void DoResponse(bool ret, byte[] receivedMessage, int receivedLen)
        {
            // BLEメッセージが返送されて来たら
            // 画面に制御を戻す
            mainForm.OnAppMainProcessExited(ret);
        }

        public void SendBLEMessage(byte cmd, byte[] message)
        {
            // リクエストデータ（APDU）を編集しリクエストデータに格納
            int length = GenerateBLERequestBytes(U2FRequestData, cmd, message);

            // BLE処理を実行し、メッセージを転送
            bleProcess.DoXferMessage(U2FRequestData, length);
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

            // ヘッダーにコマンドをセット
            u2fRequestData[0] = 0x83;

            // リクエストデータを配列にセット
            u2fRequestData[Const.MSG_HEADER_LEN + 0] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 1] = Const.U2F_INS_REGISTER;
            u2fRequestData[Const.MSG_HEADER_LEN + 2] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 3] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 4] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 5] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 6] = Const.U2F_NONCE_SIZE + Const.U2F_APPID_SIZE;

            // challengeを設定
            pos = 7;
            Array.Copy(nonce, 0, u2fRequestData, Const.MSG_HEADER_LEN + pos, Const.U2F_NONCE_SIZE);
            pos += Const.U2F_NONCE_SIZE;

            // appIdを設定
            Array.Copy(appid, 0, u2fRequestData, Const.MSG_HEADER_LEN + pos, Const.U2F_APPID_SIZE);
            pos += Const.U2F_APPID_SIZE;

            // Leを設定
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = 0x00;

            // ヘッダーにデータ長をセット
            u2fRequestData[1] = (byte)(pos / 256);
            u2fRequestData[2] = (byte)(pos % 256);

            return Const.MSG_HEADER_LEN + pos;
        }

        public void doHealthCheck()
        {
            // ランダムなチャレンジデータを生成
            GenerateNonceBytes();
            GenerateAppIDBytes();

            // リクエストデータ（APDU）を編集しリクエストデータに格納
            int length = GenerateU2FRegisterBytes(U2FRequestData);

            // BLE処理を実行し、メッセージを転送
            DoRequest(U2FRequestData, length, BLERequestType.TestRegister);
        }

        private int GenerateU2FAuthenticateBytes(byte[] u2fRequestData, byte authOption)
        {
            int pos;

            // ヘッダーにコマンドをセット
            u2fRequestData[0] = 0x83;

            // リクエストデータを配列にセット
            u2fRequestData[Const.MSG_HEADER_LEN + 0] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 1] = Const.U2F_INS_AUTHENTICATE;
            u2fRequestData[Const.MSG_HEADER_LEN + 2] = authOption;
            u2fRequestData[Const.MSG_HEADER_LEN + 3] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 4] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 5] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + 6] = Const.U2F_NONCE_SIZE + Const.U2F_APPID_SIZE + Const.U2F_KEYHANDLE_SIZE + 1;

            // challengeを設定
            pos = 7;
            Array.Copy(nonce, 0, u2fRequestData, Const.MSG_HEADER_LEN + pos, Const.U2F_NONCE_SIZE);
            pos += Const.U2F_NONCE_SIZE;

            // appIdを設定
            Array.Copy(appid, 0, u2fRequestData, Const.MSG_HEADER_LEN + pos, Const.U2F_APPID_SIZE);
            pos += Const.U2F_APPID_SIZE;

            // キーハンドル長を設定
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = Const.U2F_KEYHANDLE_SIZE;

            // キーハンドルを設定
            Array.Copy(u2FKeyhandleData, 0, u2fRequestData, Const.MSG_HEADER_LEN + pos, Const.U2F_KEYHANDLE_SIZE);
            pos += Const.U2F_KEYHANDLE_SIZE;

            // Leを設定
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = 0x00;
            u2fRequestData[Const.MSG_HEADER_LEN + pos++] = 0x00;

            // ヘッダーにデータ長をセット
            u2fRequestData[1] = (byte)(pos / 256);
            u2fRequestData[2] = (byte)(pos % 256);

            return Const.MSG_HEADER_LEN + pos;
        }

        private byte getAuthOption(BLERequestType type)
        {
            // 処理区分からオプションを設定
            if (type == BLERequestType.TestAuthenticateCheck) {
                return Const.U2F_AUTH_CHECK_ONLY;
            } else {
                return Const.U2F_AUTH_ENFORCE;
            }
        }

        private void DoTestAuthenticate(bool ret, byte[] receivedMessage, int receivedLen, BLERequestType type)
        {
            // 先行のRegister処理が失敗時は以降の処理を行わない
            if (ret == false) {
                mainForm.OnAppMainProcessExited(ret);
            }

            // Registerレスポンスからキーハンドル
            // (71バイト目から64バイト)を切り出して保持
            Array.Copy(receivedMessage, 70, u2FKeyhandleData, 0, Const.U2F_KEYHANDLE_SIZE);

            // ランダムなチャレンジデータを生成
            GenerateNonceBytes();

            // リクエストデータ（APDU）を編集しリクエストデータに格納
            int length = GenerateU2FAuthenticateBytes(U2FRequestData, getAuthOption(type));

            if (type == BLERequestType.TestAuthenticate) {
                // BLE U2Fリクエスト転送の前に、
                // FIDO認証器のMAIN SWを押してもらうように促す
                // メッセージを画面表示
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_U2F_AUTHENTICATE_START);
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_U2F_AUTHENTICATE_COMMENT1);
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_U2F_AUTHENTICATE_COMMENT2);
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_U2F_AUTHENTICATE_COMMENT3);
            }

            // BLE処理を実行し、メッセージを転送
            DoRequest(U2FRequestData, length, type);
        }

        public int GenerateBLERequestBytes(byte[] u2fRequestData, byte cmd, byte[] requestBytes)
        {
            // ヘッダーにコマンドをセット
            u2fRequestData[0] = cmd;

            // ヘッダーにデータ長をセット
            u2fRequestData[1] = (byte)(requestBytes.Length / 256);
            u2fRequestData[2] = (byte)(requestBytes.Length % 256);

            // リクエストデータを配列にセット
            Array.Copy(requestBytes, 0, u2fRequestData, Const.MSG_HEADER_LEN, requestBytes.Length);

            return Const.MSG_HEADER_LEN + requestBytes.Length;
        }

        //
        // BLE PINGコマンド
        //
        public void DoTestBLEPing()
        {
            // PINGリクエスト処理を実行
            bleRequestType = BLERequestType.TestBLEPing;
            ctap2.DoRequestPing();
        }

        private void DoResponseBLEPing(byte[] receivedMessage, int receivedLen)
        {
            // BLEヘッダーを除去し、PINGレスポンス処理を実行
            ExtractResponseData(receivedMessage, receivedLen);
            ctap2.DoResponsePing(BLEResponseData, BLEResponseLength);
        }

        //
        // BLE CTAP2ヘルスチェック
        //
        public void DoCtap2Healthcheck(string pin)
        {
            // 実行するコマンドと引数を退避
            //   認証器からPINトークンを取得するため、
            //   ClientPINコマンド（getKeyAgreement）を
            //   事前実行する必要あり
            bleRequestType = BLERequestType.TestBLECTAP2;
            ctap2.SetClientPin(pin);
            ctap2.SetRequestType(Ctap2.RequestType.TestMakeCredential);
            ctap2.DoGetKeyAgreement();
        }

        private void DoResponseBLECTAP2(byte[] receivedMessage, int receivedLen)
        {
            // BLEヘッダーを除去し、PINGレスポンス処理を実行
            ExtractResponseData(receivedMessage, receivedLen);
            ctap2.DoResponseCtapHidCbor(BLEResponseData, BLEResponseLength);
        }

        private void ExtractResponseData(byte[] receivedMessage, int receivedLen)
        {
            // CBORバイト配列を抽出
            //   receivedMessage の先頭には、
            //   BLEヘッダー（3バイト）が含まれているので
            //   それをスキップして新しい配列にコピー
            byte cnth = receivedMessage[1];
            byte cntl = receivedMessage[2];
            BLEResponseLength = cnth * 256 + cntl;
            Array.Copy(receivedMessage, 3, BLEResponseData, 0, receivedLen - 3);
        }

        public void doExit()
        {
            bleProcess.DisconnectBLE();
            System.Windows.Forms.Application.Exit();
            AppCommon.OutputLogToFile(String.Format("{0}を終了しました", MainForm.MaintenanceToolTitle), true);
        }
    }
}
