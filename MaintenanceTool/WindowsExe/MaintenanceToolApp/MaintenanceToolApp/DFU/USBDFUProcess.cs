using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp.DFU
{
    internal class USBDFUProcess
    {
        // 処理実行のためのプロパティー
        private DFUParameter Parameter = null!;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow;

        // HIDからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        // USB接続／切断時のコールバック参照
        private readonly HIDProcess.HandlerOnConnectHIDDevice OnConnectHIDDeviceRef;

        public USBDFUProcess(Window parentWindowRef, DFUParameter parameterRef)
        {
            // 親ウィンドウの参照を保持
            ParentWindow = parentWindowRef;

            // パラメーターの参照を保持
            Parameter = parameterRef;

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
            OnConnectHIDDeviceRef = new HIDProcess.HandlerOnConnectHIDDevice(OnConnectHIDDevice);
        }

        public void StartUSBDFU()
        {
            // USB接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(ParentWindow)) {
                return;
            }

            // ブートローダーモード遷移コマンドを実行
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
            DoRequestCommandBootloaderMode();
        }

        //
        // ブートローダーモード遷移処理
        //
        private void DoRequestCommandBootloaderMode()
        {
            // コマンドバイトだけを送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidCommand(HIDProcessConst.HID_CMD_BOOTLOADER_MODE, System.Array.Empty<byte>());
        }

        public void DoResponseCommandBootloaderMode(byte CMD, byte[] responseData)
        {
            if (CMD == HIDProcessConst.HID_CMD_BOOTLOADER_MODE) {
                // ブートローダーモード遷移コマンド成功時
                HIDProcess.RegisterHandlerOnConnectHIDDevice(OnConnectHIDDeviceRef);

            } else {
                // ブートローダーモード遷移コマンド失敗時は、
                // 画面に制御を戻す
                NotifyCommandTerminated(AppCommon.MSG_DFU_TARGET_NOT_BOOTLOADER_MODE, false);
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
                NotifyCommandTerminated(AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
                return;
            }

            // INITからの戻りの場合
            if (CMD == HIDProcessConst.HID_CMD_CTAPHID_INIT) {
                DoResponseCtapHidInit();
                return;
            }

            // ブートローダーモード遷移コマンド実行後の処理
            DoResponseCommandBootloaderMode(CMD, responseData);
        }

        //
        // USB接続／切断時の処理
        //
        private void OnConnectHIDDevice(bool connected)
        {
            // イベントを解除
            HIDProcess.UnregisterHandlerOnConnectHIDDevice(OnConnectHIDDeviceRef);

            if (connected) {
                // 画面に制御を戻す
                NotifyCommandTerminated(AppCommon.MSG_DFU_TARGET_NOT_BOOTLOADER_MODE, false);
                return;
            }

            // ブートローダーモード遷移完了
            AppLogUtil.OutputLogDebug(AppCommon.MSG_DFU_TARGET_BOOTLOADER_MODE);

            //
            // TODO: 仮の実装です。
            NotifyCommandTerminated(AppCommon.MSG_NONE, true);
        }

        //
        // USB DFU処理の終了
        //
        private void NotifyCommandTerminated(string errorMessage, bool success)
        {
            // メイン画面に制御を戻す
            Parameter.ErrorMessage = errorMessage;
            Parameter.Success = success;
            CommandProcess.NotifyCommandTerminated(AppCommon.PROCESS_NAME_BLE_DFU, Parameter.ErrorMessage, Parameter.Success, ParentWindow);
        }
    }
}
