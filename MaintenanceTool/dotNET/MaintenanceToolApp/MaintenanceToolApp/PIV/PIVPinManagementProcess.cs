using System;
using System.Linq;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.PIV
{
    internal class PIVPinManagementProcess
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

            // CCID I/F経由で、PIVアプレットselectを実行
            new PIVCCIDProcess().DoProcess(Parameter, DoResponsePIVSelectApplication);
        }

        private void DoResponsePIVSelectApplication(bool success, string errorMessage)
        {
            // PIN番号管理を実行
            DoRequestPinManagement();
        }

        private void DoCommandResponse(bool success, string errorMessage)
        {
            // CCIDデバイスから切断し、上位クラスに制御を戻す
            CCIDProcess.DisconnectCCID();
            OnCommandResponse(success, errorMessage);
        }

        //
        // PIN番号管理
        //
        private void DoRequestPinManagement()
        {
            // INS、P2を設定
            byte[] insP2 = GetPinManagementInsP2(Parameter.Command);
            byte ins = insP2[0];
            byte p2 = insP2[1];

            // コマンドAPDUを生成
            byte[] apdu = GeneratePinManagementAPDU(Parameter.CurrentPin, Parameter.NewPin);

            // PIN管理コマンドを実行
            CCIDParameter param = new CCIDParameter(ins, 0x00, p2, apdu, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponsePinManagement);
        }

        private void DoResponsePinManagement(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false) {
                DoCommandResponse(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            // ステータスワードをチェックし、PIN管理コマンドの成否を判定
            bool ret;
            string errorMessage;
            if (Parameter.Command == Command.COMMAND_CCID_PIV_CHANGE_PIN) {
                ret = PIVCCIDPinAuthProcess.CheckPIVPinVerifyResponseSW(responseSW, out errorMessage);
            } else {
                ret = PIVCCIDPinAuthProcess.CheckPIVPukVerifyResponseSW(responseSW, out errorMessage);
            }
            DoCommandResponse(ret, errorMessage);
        }

        private static byte[] GetPinManagementInsP2(Command command)
        {
            // INSとP2を配列で戻す
            byte[] insP2 = new byte[2];
            switch (command) {
            case Command.COMMAND_CCID_PIV_CHANGE_PIN:
                insP2[0] = PIVCCIDConst.PIV_INS_CHANGE_REFERENCE;
                insP2[1] = PIVConst.PIV_KEY_PIN;
                break;
            case Command.COMMAND_CCID_PIV_CHANGE_PUK:
                insP2[0] = PIVCCIDConst.PIV_INS_CHANGE_REFERENCE;
                insP2[1] = PIVConst.PIV_KEY_PUK;
                break;
            case Command.COMMAND_CCID_PIV_UNBLOCK_PIN:
                insP2[0] = PIVCCIDConst.PIV_INS_RESET_RETRY;
                insP2[1] = PIVConst.PIV_KEY_PIN;
                break;
            default:
                insP2[0] = 0;
                insP2[1] = 0;
                break;
            }
            return insP2;
        }

        private static byte[] GeneratePinManagementAPDU(string currentPin, string renewalPin)
        {
            // 認証用PINコード、更新用PINコードの順で配列にセット
            byte[] apdu = PIVCCIDPinAuthProcess.GeneratePinBytes(currentPin);
            apdu = apdu.Concat(PIVCCIDPinAuthProcess.GeneratePinBytes(renewalPin)).ToArray();
            return apdu;
        }
    }
}
