using MaintenanceToolApp;
using ToolAppCommon;
using static MaintenanceToolApp.FIDODefine;

namespace VendorMaintenanceTool.CommonProcess
{
    internal class BootloaderModeProcess
    {
        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnNotifyCommandTerminated(bool success, string errorMessage);
        private HandlerOnNotifyCommandTerminated OnNotifyCommandTerminated = null!;

        // HIDからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        // USB接続／切断時のコールバック参照
        private readonly HIDProcess.HandlerOnConnectHIDDevice OnConnectHIDDeviceRef;

        // 待機フラグ
        private bool WaitingToBoot = false;

        //
        // 外部公開用
        //
        public BootloaderModeProcess()
        {
            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
            OnConnectHIDDeviceRef = new HIDProcess.HandlerOnConnectHIDDevice(OnConnectHIDDevice);
        }

        public void DoProcess(HandlerOnNotifyCommandTerminated handlerRef)
        {
            // コールバックを保持
            OnNotifyCommandTerminated = handlerRef;

            // 待機フラグをリセット
            WaitingToBoot = false;

            // CTAPHID_INITから実行
            DoRequestCtapHidInit();
        }

        //
        // 内部処理
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
            DoRequestBootloaderModeCommand();
        }

        private void DoRequestBootloaderModeCommand()
        {
            // ブートローダーモード遷移コマンドを実行する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidCommand(0x80 | MNT_COMMAND_BASE, new byte[] { MNT_COMMAND_BOOTLOADER_MODE });
        }

        private void DoResponseBootloaderModeCommand(byte[] responseData)
        {
            // レスポンスメッセージの１バイト目（ステータスコード）を確認
            if (responseData[0] != 0x00) {
                string msg = string.Format(AppCommon.MSG_OCCUR_UNKNOWN_ERROR_ST, responseData[0]);
                OnNotifyCommandTerminated(false, msg);

            } else {
                // 接続断まで待機
                WaitingToBoot = true;
                HIDProcess.RegisterHandlerOnConnectHIDDevice(OnConnectHIDDeviceRef);
            }
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
                OnNotifyCommandTerminated(success, errorMessage);
                return;
            }

            // INITからの戻りの場合
            if (CMD == HIDProcessConst.HID_CMD_CTAPHID_INIT) {
                DoResponseCtapHidInit();
                return;
            }

            if (CMD == HIDProcessConst.HID_CMD_UNKNOWN_ERROR) {
                // ブートローダーモード遷移コマンド失敗時は、即時で制御を戻す
                OnNotifyCommandTerminated(success, AppCommon.MSG_DFU_TARGET_NOT_BOOTLOADER_MODE);

            } else {
                // 実行コマンドからの戻り
                DoResponseBootloaderModeCommand(responseData);
            }
        }

        //
        // USB接続／切断時の処理
        //
        private void OnConnectHIDDevice(bool connected)
        {
            // イベントを解除
            HIDProcess.UnregisterHandlerOnConnectHIDDevice(OnConnectHIDDeviceRef);

            // 待機フラグ設定中でない場合は無視
            if (WaitingToBoot == false) {
                return;
            }

            if (connected == false) {
                // 切断時は、待機フラグをリセット
                WaitingToBoot = false;

                // ブートローダーモード遷移完了
                AppLogUtil.OutputLogDebug(AppCommon.MSG_DFU_TARGET_NORMAL_MODE);
                OnNotifyCommandTerminated(true, AppCommon.MSG_NONE);
            }
        }
    }
}
