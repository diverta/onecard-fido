using MaintenanceToolApp;
using System.Threading;

namespace VendorMaintenanceTool.CommonProcess
{
    internal class BootloaderModeProcess
    {
        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnNotifyCommandTerminated(bool success, string errorMessage);
        private HandlerOnNotifyCommandTerminated OnNotifyCommandTerminated = null!;

        //
        // 外部公開用
        //
        public BootloaderModeProcess()
        {
        }

        public void DoProcess(HandlerOnNotifyCommandTerminated handlerRef)
        {
            // コールバックを保持
            OnNotifyCommandTerminated = handlerRef;

            // TODO: 仮の実装です。
            Thread.Sleep(2000);
            OnNotifyCommandTerminated(true, AppCommon.MSG_NONE);
        }
    }
}
