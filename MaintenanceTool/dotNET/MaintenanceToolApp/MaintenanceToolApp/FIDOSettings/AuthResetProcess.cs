﻿using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.FIDOSettings
{
    internal class AuthResetProcess
    {
        // 処理実行のためのプロパティー
        private readonly FIDOSettingsParameter Parameter;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(string commandTitle, string errorMessage, bool success);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;

        // HID／BLEからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        public AuthResetProcess(FIDOSettingsParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
        }

        //
        // 外部公開用
        //
        public void DoRequestAuthReset(HandlerOnNotifyCommandTerminated handler)
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
            case Command.COMMAND_AUTH_RESET:
                DoRequestCommandAuthReset();
                break;
            default:
                // メイン画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
                break;
            }
        }

        //
        // FIDO認証情報の消去
        //
        private void DoRequestCommandAuthReset()
        {
            // リクエスト転送の前に、
            // 基板上のボタンを押してもらうように促す
            // メッセージを画面表示
            CommandProcess.NotifyMessageToMainUI(AppCommon.MSG_CLEAR_PIN_CODE_COMMENT1);
            CommandProcess.NotifyMessageToMainUI(AppCommon.MSG_CLEAR_PIN_CODE_COMMENT2);
            CommandProcess.NotifyMessageToMainUI(AppCommon.MSG_CLEAR_PIN_CODE_COMMENT3);

            // authenticatorResetコマンドバイトを生成
            byte[] commandByte = { FIDODefine.CTAP2_CBORCMD_AUTH_RESET };

            // authenticatorResetコマンドを実行する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidCommand(HIDProcessConst.HID_CMD_CTAPHID_CBOR, commandByte);
        }

        private void DoResponseCommandAuthReset(byte[] responseData)
        {
            // ステータスバイトをチェック
            string errorMessage = AppCommon.MSG_NONE;
            bool success = (responseData[0] == 0x00);
            if (success == false) {
                errorMessage = AppCommon.MSG_AUTH_RESET_COMMAND_ERROR;
            }

            // 上位クラスに制御を戻す
            NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, success);
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
            case Command.COMMAND_AUTH_RESET:
                DoResponseCommandAuthReset(responseData);
                break;
            default:
                // メイン画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
                break;
            }
        }
    }
}
