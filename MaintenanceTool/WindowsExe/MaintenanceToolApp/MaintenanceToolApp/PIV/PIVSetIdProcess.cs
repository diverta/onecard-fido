using System;
using ToolAppCommon;

namespace MaintenanceToolApp.PIV
{
    internal class PIVSetIdProcess
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

            // CCID I/F経由で、PIV管理機能認証を実行
            new PIVCCIDProcess().DoProcess(Parameter, DoResponseAdminAuth);
        }

        private void DoResponseAdminAuth(bool success, string errorMessage)
        {
            if (success == false) {
                DoCommandResponse(false, errorMessage);
                return;

            } else {
                // PIV管理機能認証が成功
                AppLogUtil.OutputLogInfo(AppCommon.MSG_PIV_ADMIN_AUTH_SUCCESS);
            }

            // CHUIDインポート処理を実行
            DoRequestPivSetChuId();
        }

        private void DoCommandResponse(bool success, string errorMessage)
        {
            // CCIDデバイスから切断し、上位クラスに制御を戻す
            CCIDProcess.DisconnectCCID();
            OnCommandResponse(success, errorMessage);
        }

        //
        // CHUIDインポート処理
        //
        private void DoRequestPivSetChuId()
        {
            // TODO: 仮の実装です。
            DoCommandResponse(true, AppCommon.MSG_NONE);
        }
    }
}
