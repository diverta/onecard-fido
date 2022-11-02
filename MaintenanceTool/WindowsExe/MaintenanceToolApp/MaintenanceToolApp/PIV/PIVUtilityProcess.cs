using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.PIV
{
    internal class PIVUtilityProcess
    {
        // 処理実行のためのプロパティー
        private PIVParameter Parameter = null!;

        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnCommandResponse(bool success, string errorMessage);
        private HandlerOnCommandResponse OnCommandResponse = null!;

        public void DoProcess(PIVParameter parameterRef, HandlerOnCommandResponse handlerRef)
        {
            // パラメーターを保持
            Parameter = parameterRef;

            // コールバックを保持
            OnCommandResponse = handlerRef;

            // CCIDインタフェース経由で認証器に接続
            if (CCIDProcess.ConnectCCID() == false) {
                // PIV機能を認識できなかった旨のエラーメッセージを設定し
                // 上位クラスに制御を戻す
                OnCommandResponse(false, AppCommon.MSG_ERROR_PIV_APPLET_SELECT_FAILED);
                return;
            }

            if (Parameter.Command == Command.COMMAND_CCID_PIV_RESET) {
                // 設定情報消去を実行
                DoRequestCardReset();

            } else {
                // 設定情報照会を実行
                DoRequestCardStatus();
            }
        }

        private void DoCommandResponse(bool success, string errorMessage)
        {
            // CCIDデバイスから切断し、上位クラスに制御を戻す
            CCIDProcess.DisconnectCCID();
            OnCommandResponse(success, errorMessage);
        }

        //
        // 設定情報消去
        //
        private void DoRequestCardReset()
        {
            // TODO: 仮の実装です。
            DoCommandResponse(true, AppCommon.MSG_NONE);
        }

        //
        // 設定情報照会
        //
        private void DoRequestCardStatus()
        {
            // TODO: 仮の実装です。
            DoCommandResponse(true, AppCommon.MSG_NONE);
        }
    }
}
