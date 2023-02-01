using MaintenanceToolApp;
using System.Threading;
using ToolAppCommon;

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
            // TODO: 仮の実装です。
            Thread.Sleep(2000);
            OnNotifyCommandTerminated(true, AppCommon.MSG_NONE);
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
        }
    }
}
