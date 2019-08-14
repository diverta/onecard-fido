﻿using System;
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

        // PINGバイトを保持
        private byte[] pingBytes = new byte[100];

        // メイン画面の参照を保持
        private MainForm mainForm;

        // BLEデバイス関連
        private BLEProcess bleProcess = new BLEProcess();

        public AppMain(MainForm f)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // BLEデバイス関連
            bleProcess.OneCardPeripheralPaired += new BLEProcess.oneCardPeripheralPairedEvent(OnPairedDevice);
            bleProcess.MessageTextEvent += new BLEProcess.MessageTextEventHandler(OnPrintMessageText);
            bleProcess.ReceiveBLEMessageEvent += new BLEProcess.ReceiveBLEMessageEventHandler(OnReceiveBLEMessage);

            AppCommon.OutputLogToFile(String.Format("{0}を起動しました", MainForm.MaintenanceToolTitle), true);
        }

        private void OnPrintMessageText(string message)
        {
            // メッセージを画面表示させる
            mainForm.OnPrintMessageText(message);
        }

        public void doPairing()
        {
            bleProcess.PairWithOneCardPeripheral();
        }

        public void OnPairedDevice(bool success)
        {
            // メイン画面の参照を経由し、コマンド実行完了時の処理を実行
            mainForm.OnAppMainProcessExited(success);
        }

        private void OnReceiveBLEMessage(bool ret, byte[] receivedMessage, int receivedLen)
        {
            if (ret == false) {
                // 処理結果が不正の場合は画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                return;
            }

            switch (bleRequestType) {
            case BLERequestType.TestRegister:
                DoTestAuthenticate(ret, receivedMessage, receivedLen, BLERequestType.TestAuthenticateCheck);
                break;
            case BLERequestType.TestAuthenticateCheck:
                DoTestAuthenticate(ret, receivedMessage, receivedLen, BLERequestType.TestAuthenticate);
                break;
            case BLERequestType.TestAuthenticate:
                DoResponse(ret, receivedMessage, receivedLen);
                break;
            case BLERequestType.TestBLEPing:
                DoResponseBLEPing(ret, receivedMessage, receivedLen);
                break;
            default:
                break;
            }
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
                // One CardのMAIN SWを押してもらうように促す
                // メッセージを画面表示
                mainForm.OnPrintMessageText("U2F Authenticateを開始します.");
                mainForm.OnPrintMessageText("  ユーザー所在確認が必要となりますので、");
                mainForm.OnPrintMessageText("  FIDO認証器上のユーザー所在確認LEDが点滅したら、");
                mainForm.OnPrintMessageText("  MAIN SWを１回押してください.");
            }

            // BLE処理を実行し、メッセージを転送
            DoRequest(U2FRequestData, length, type);
        }

        private int GeneratePingRequestBytes(byte[] u2fRequestData)
        {
            // ヘッダーにコマンドをセット
            u2fRequestData[0] = 0x81;

            // ヘッダーにデータ長をセット
            u2fRequestData[1] = (byte)(pingBytes.Length / 256);
            u2fRequestData[2] = (byte)(pingBytes.Length % 256);

            // リクエストデータを配列にセット
            Array.Copy(pingBytes, 0, u2fRequestData, Const.MSG_HEADER_LEN, pingBytes.Length);

            return Const.MSG_HEADER_LEN + pingBytes.Length;
        }

        public void DoTestBLEPing()
        {
            // ランダムデータを生成
            new Random().NextBytes(pingBytes);

            // リクエストデータ（APDU）を編集しリクエストデータに格納
            int length = GeneratePingRequestBytes(U2FRequestData);

            // BLE処理を実行し、メッセージを転送
            DoRequest(U2FRequestData, length, BLERequestType.TestBLEPing);
        }

        private void DoResponseBLEPing(bool ret, byte[] receivedMessage, int receivedLen)
        {
            // PINGバイトの一致チェック
            //   receivedMessage の先頭には、
            //   BLEヘッダー（3バイト）が含まれているので
            //   それをスキップしてチェック
            bool result = true;
            for (int i = 0; i < pingBytes.Length; i++) {
                if (pingBytes[i] != receivedMessage[BLEProcess.MSG_HEADER_LEN + i]) {
                    // 画面のテキストエリアにメッセージを表示
                    mainForm.OnPrintMessageText(AppCommon.MSG_BLE_INVALID_PING);
                    result = false;
                    break;
                }
            }
            // 画面に制御を戻す
            mainForm.OnAppMainProcessExited(result);
        }

        public void doExit()
        {
            bleProcess.DisconnectBLE();
            System.Windows.Forms.Application.Exit();
            AppCommon.OutputLogToFile(String.Format("{0}を終了しました", MainForm.MaintenanceToolTitle), true);
        }
    }
}
