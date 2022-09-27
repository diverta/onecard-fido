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

        public USBDFUProcess(Window parentWindowRef, DFUParameter parameterRef)
        {
            // 親ウィンドウの参照を保持
            ParentWindow = parentWindowRef;

            // パラメーターの参照を保持
            Parameter = parameterRef;
        }

        public void StartUSBDFU()
        {
            // USB接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(ParentWindow)) {
                return;
            }

            // TODO: 仮の実装です。
            Parameter.ErrorMessage = AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED;
            Parameter.Success = false;

            // メイン画面に制御を移す
            CommandProcess.NotifyCommandTerminated(AppCommon.PROCESS_NAME_BLE_DFU, Parameter.ErrorMessage, Parameter.Success, ParentWindow);
        }
    }
}
