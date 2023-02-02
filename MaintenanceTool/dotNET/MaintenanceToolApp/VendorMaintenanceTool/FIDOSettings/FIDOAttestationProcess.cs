using MaintenanceToolApp;
using System.Threading;
using ToolAppCommon;
using VendorMaintenanceTool.VendorFunction;

namespace VendorMaintenanceTool.FIDOSettings
{
    internal class FIDOAttestationProcess
    {
        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnNotifyCommandTerminated(bool success, string errorMessage);
        private HandlerOnNotifyCommandTerminated OnNotifyCommandTerminated = null!;

        // 処理実行のためのプロパティー
        private readonly VendorFunctionParameter Parameter = null!;

        //
        // 外部公開用
        //
        public FIDOAttestationProcess(VendorFunctionParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
        }

        public void DoProcess(HandlerOnNotifyCommandTerminated handlerRef)
        {
            // コールバックを保持
            OnNotifyCommandTerminated = handlerRef;

            // TODO: 仮の実装です。
            AppLogUtil.OutputLogInfo(Parameter.KeyPath);
            AppLogUtil.OutputLogInfo(Parameter.CertPath);
            Thread.Sleep(2000);
            OnNotifyCommandTerminated(true, AppCommon.MSG_NONE);
        }
    }
}
