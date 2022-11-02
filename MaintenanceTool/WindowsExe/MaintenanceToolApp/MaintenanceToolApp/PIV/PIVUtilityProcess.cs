using System;
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
            // カードリセットコマンドを実行
            byte[] apdu = Array.Empty<byte>();
            CCIDParameter param = new CCIDParameter(PIVCCIDConst.YKPIV_INS_RESET, 0x00, 0x00, apdu, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponseCardReset);
        }

        private void DoResponseCardReset(bool success, byte[] responseData, UInt16 responseSW)
        {
            // ステータスワードの内容に応じて分岐
            if (responseSW == CCIDProcessConst.SW_SEC_STATUS_NOT_SATISFIED) {
                // PIN／PUKがまだブロックされていない場合
                DoCommandResponse(false, AppCommon.MSG_ERROR_PIV_RESET_FAIL);

            } else if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                // 不明なエラーが発生時
                DoCommandResponse(false, string.Format(AppCommon.MSG_ERROR_PIV_UNKNOWN, responseSW));

            } else {
                // 正常終了
                DoCommandResponse(true, AppCommon.MSG_NONE);
            }
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
