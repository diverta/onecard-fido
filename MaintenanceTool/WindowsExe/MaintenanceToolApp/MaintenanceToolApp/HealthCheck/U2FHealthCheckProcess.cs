using System;
using System.Linq;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.HealthCheck
{
    internal static class U2FProcessConst
    {
        // U2F処理に関する定義
        public const int U2F_INS_REGISTER = 0x01;
        public const int U2F_INS_AUTHENTICATE = 0x02;
        public const int U2F_INS_VERSION = 0x03;
        public const int U2F_AUTH_ENFORCE = 0x03;
        public const int U2F_AUTH_CHECK_ONLY = 0x07;
        public const int U2F_APPID_SIZE = 32;
        public const int U2F_NONCE_SIZE = 32;

        // U2Fコマンドバイトに関する定義
        public const byte U2F_CMD_MSG = 0x83;
    }

    internal class U2FHealthCheckProcess
    {
        // 処理実行のためのプロパティー
        private readonly HealthCheckParameter Parameter;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(string commandTitle, string errorMessage, bool success);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;

        // HIDインターフェースからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        public U2FHealthCheckProcess(HealthCheckParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
        }

        //
        // 外部公開用
        //
        public void DoRequestBleU2fHealthCheck(HandlerOnNotifyCommandTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyCommandTerminated = handler;

            // TODO: 仮の実装です。
            NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
        }

        public void DoRequestHidU2fHealthCheck(HandlerOnNotifyCommandTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyCommandTerminated = handler;

            // CTAPHID_INITから実行
            DoRequestCtapHidInit();
        }

        public void DoRequestBlePingTest(HandlerOnNotifyCommandTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyCommandTerminated = handler;

            // PING処理を実行
            DoRequestPing();
        }

        public void DoRequestHidPingTest(HandlerOnNotifyCommandTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyCommandTerminated = handler;

            // CTAPHID_INITから実行
            DoRequestCtapHidInit();
        }

        //
        // INITコマンド関連処理
        //
        private void DoRequestCtapHidInit()
        {
            // INITコマンドを実行し、nonce を送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidInit();
        }

        private void DoResponseCtapHidInit()
        {
            // CTAPHID_INIT応答後の処理を実行
            switch (Parameter.Command) {
            case Command.COMMAND_HID_U2F_HCHECK:
                DoRequestCommandRegister();
                break;
            case Command.COMMAND_TEST_CTAPHID_PING:
                DoRequestPing();
                break;
            default:
                // メイン画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
                break;
            }
        }

        //
        // U2F Registerコマンド関連処理
        //
        // リクエストデータ格納領域
        private readonly byte[] U2FRequestData = new byte[1024];

        // 生成されたランダムなチャレンジ、AppIDを保持
        // (ヘルスチェック処理で使用)
        private readonly byte[] NonceBytes = new byte[U2FProcessConst.U2F_NONCE_SIZE];
        private readonly byte[] AppidBytes = new byte[U2FProcessConst.U2F_APPID_SIZE];
        private readonly Random RandomInst = new Random();

        // U2Fキーハンドルデータを保持
        // (ヘルスチェック処理で使用)
        private byte[] U2FKeyhandleData = new byte[128];
        private int U2FKeyhandleSize;

        private void DoRequestCommandRegister()
        {
            // チャレンジにランダム値を設定
            RandomInst.NextBytes(NonceBytes);

            // AppIDにランダム値を設定
            RandomInst.NextBytes(AppidBytes);

            // リクエストデータ（APDU）を編集しリクエストデータに格納
            int length = GenerateU2FRegisterBytes(U2FRequestData);
            byte[] requestBytes = U2FRequestData.Take(length).ToArray();

            // U2F Registerコマンドを実行
            Parameter.Command = Command.COMMAND_TEST_REGISTER;
            switch (Parameter.Transport) {
            case Transport.TRANSPORT_HID:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestCtapHidCommand(U2FProcessConst.U2F_CMD_MSG, requestBytes);
                break;
            case Transport.TRANSPORT_BLE:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestBleCommand(U2FProcessConst.U2F_CMD_MSG, requestBytes);
                break;
            default:
                break;
            }
        }

        private int GenerateU2FRegisterBytes(byte[] u2fRequestData)
        {
            int pos;

            // リクエストデータを配列にセット
            u2fRequestData[0] = 0x00;
            u2fRequestData[1] = U2FProcessConst.U2F_INS_REGISTER;
            u2fRequestData[2] = 0x00;
            u2fRequestData[3] = 0x00;
            u2fRequestData[4] = 0x00;
            u2fRequestData[5] = 0x00;
            u2fRequestData[6] = U2FProcessConst.U2F_NONCE_SIZE + U2FProcessConst.U2F_APPID_SIZE;

            // challengeを設定
            pos = 7;
            Array.Copy(NonceBytes, 0, u2fRequestData, pos, U2FProcessConst.U2F_NONCE_SIZE);
            pos += U2FProcessConst.U2F_NONCE_SIZE;

            // appIdを設定
            Array.Copy(AppidBytes, 0, u2fRequestData, pos, U2FProcessConst.U2F_APPID_SIZE);
            pos += U2FProcessConst.U2F_APPID_SIZE;

            // Leを設定
            u2fRequestData[pos++] = 0x00;
            u2fRequestData[pos++] = 0x00;

            return pos;
        }

        private void DoResponseRegister(byte[] message)
        {
            string errorMessage;
            if (CheckStatusWord(message, out errorMessage) == false) {
                // 処理結果が不正の場合は画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, false);
                return;
            }

            // Registerレスポンスからキーハンドル長(67バイト目)を取得
            U2FKeyhandleSize = message[66];

            // Registerレスポンスからキーハンドル
            // (68バイト目以降)を切り出して保持
            Array.Copy(message, 67, U2FKeyhandleData, 0, U2FKeyhandleSize);

            // ヘルスチェックの進捗をメイン画面に表示させる
            CommandProcess.NotifyMessageToMainUI(AppCommon.MSG_HCHK_U2F_REGISTER_SUCCESS);

            // TODO: 仮の実装です。 
            NotifyCommandTerminated(Parameter.CommandTitle, "TODO: 仮の実装です。", false);
        }

        private bool CheckStatusWord(byte[] receivedMessage, out string errorMessage)
        {
            //
            // U2F関連コマンドの場合は
            // ステータスワードチェックを行う。
            //
            int receivedLen = receivedMessage.Length;
            byte[] statusBytes = new byte[2];
            Array.Copy(receivedMessage, receivedLen - 2, statusBytes, 0, 2);
            if (BitConverter.IsLittleEndian) {
                Array.Reverse(statusBytes);
            }

            ushort statusWord = BitConverter.ToUInt16(statusBytes, 0);
            errorMessage = "";

            if (statusWord == 0x6985) {
                // キーハンドルチェックの場合は成功とみなす
                return true;
            }
            if (statusWord == 0x6a80) {
                // invalid keyhandleエラーである場合はその旨を通知
                errorMessage = AppCommon.MSG_OCCUR_KEYHANDLE_ERROR;
                return false;
            }
            if (statusWord == 0x9402) {
                // 鍵・証明書がインストールされていない旨のエラーである場合はその旨を通知
                errorMessage = AppCommon.MSG_OCCUR_SKEYNOEXIST_ERROR;
                return false;
            }
            if (statusWord == 0x9601) {
                // ペアリングモード時はペアリング以外の機能を実行できない旨を通知
                errorMessage = AppCommon.MSG_OCCUR_PAIRINGMODE_ERROR;
                return false;
            }
            if (statusWord != 0x9000) {
                // U2Fサービスの戻りコマンドが不正の場合はエラー
                errorMessage = AppCommon.MSG_OCCUR_UNKNOWN_ERROR;
                return false;
            }
            return true;
        }

        //
        // PINGコマンド関連処理
        //
        // PINGバイトを保持
        private readonly byte[] PingBytes = new byte[100];

        private void DoRequestPing()
        {
            // 100バイトのランダムデータを生成
            RandomInst.NextBytes(PingBytes);

            // PINGコマンドを実行する
            switch (Parameter.Transport) {
            case Transport.TRANSPORT_HID:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestCtapHidCommand(HIDProcessConst.HID_CMD_CTAPHID_PING, PingBytes);
                break;
            case Transport.TRANSPORT_BLE:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestBleCommand(HIDProcessConst.HID_CMD_CTAPHID_PING, PingBytes);
                break;
            default:
                break;
            }
        }

        private void DoResponsePing(byte[] responseData)
        {
            // レスポンスデータのチェック
            if (responseData == null || responseData.Length == 0) {
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_CMDTST_INVALID_PING, false);
                return;
            }

            // PINGバイトの一致チェック
            for (int i = 0; i < PingBytes.Length; i++) {
                if (PingBytes[i] != responseData[i]) {
                    // 画面のテキストエリアにメッセージを表示
                    NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_CMDTST_INVALID_PING, false);
                    return;
                }
            }

            // 画面に制御を戻す
            NotifyCommandTerminated(Parameter.CommandTitle, "", true);
        }

        //
        // HID／BLEからのレスポンス振分け処理
        //
        private void OnCommandResponse(byte CMD, byte[] responseData, bool success, string errorMessage)
        {
            // イベントを解除
            CommandProcess.UnregisterHandlerOnCommandResponse(OnCommandResponseRef);

            // 即時でアプリケーションに制御を戻す
            if (success == false) {
                NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, success);
                return;
            }

            // INITからの戻りの場合
            if (CMD == HIDProcessConst.HID_CMD_CTAPHID_INIT) {
                DoResponseCtapHidInit();
                return;
            }

            // 実行コマンドにより処理分岐
            switch (Parameter.Command) {
            case Command.COMMAND_TEST_REGISTER:
                DoResponseRegister(responseData);
                break;
            case Command.COMMAND_TEST_CTAPHID_PING:
            case Command.COMMAND_TEST_BLE_PING:
                DoResponsePing(responseData);
                break;
            default:
                // メイン画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
                break;
            }
        }
    }
}
