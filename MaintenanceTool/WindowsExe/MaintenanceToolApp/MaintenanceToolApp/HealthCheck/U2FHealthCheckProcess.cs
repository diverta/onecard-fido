using System;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.HealthCheck
{
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
        // PINGコマンド関連処理
        //
        // PINGバイトを保持
        private readonly byte[] PingBytes = new byte[100];

        public void DoRequestPing()
        {
            // 100バイトのランダムデータを生成
            new Random().NextBytes(PingBytes);

            // PINGコマンドを実行する
            switch (Parameter.Transport) {
            case Transport.TRANSPORT_HID:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestCtapHidCommand(HIDProcessConst.HID_CMD_CTAPHID_PING, PingBytes);
                break;
            default:
                break;
            }
        }

        public void DoResponsePing(byte[] responseData)
        {
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
        // HIDからのレスポンス振分け処理
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
            case Command.COMMAND_TEST_CTAPHID_PING:
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
