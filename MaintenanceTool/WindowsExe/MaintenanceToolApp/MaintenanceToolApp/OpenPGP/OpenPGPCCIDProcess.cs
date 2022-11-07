using System;
using System.Text;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.OpenPGP
{
    internal class OpenPGPCCIDConst
    {
        public const byte OPENPGP_INS_SELECT = 0xA4;
        public const byte OPENPGP_INS_VERIFY = 0x20;
    }

    internal class OpenPGPCCIDProcess
    {
        // 処理実行のためのプロパティー
        private OpenPGPParameter Parameter = null!;

        // CCID I/Fからデータ受信時のコールバックを保持
        public delegate void HandlerOnCommandResponse(bool success, string errorMessage);
        private HandlerOnCommandResponse OnCommandResponse = null!;

        //
        // OpenPGP機能設定用関数
        // 
        public void DoOpenPGPCcidCommand(OpenPGPParameter parameter, HandlerOnCommandResponse handlerRef)
        {
            // 引き渡されたパラメーターを退避
            Parameter = parameter;

            // コールバックを保持
            OnCommandResponse = handlerRef;

            // CCIDインタフェース経由で認証器に接続
            if (CCIDProcess.ConnectCCID() == false) {
                // OpenPGP機能を認識できなかった旨のエラーメッセージを設定し
                // 上位クラスに制御を戻す
                OnCommandResponse(false, AppCommon.MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL);
                return;
            }

            // 機能実行に先立ち、PIVアプレットをSELECT
            DoRequestOpenPGPInsSelectApplication();
        }

        private void DoCommandResponse(bool success, string errorMessage)
        {
            // CCIDデバイスから切断し、上位クラスに制御を戻す
            CCIDProcess.DisconnectCCID();
            OnCommandResponse(success, errorMessage);
        }

        //
        // CCID I/Fコマンド実行関数
        //
        private void DoRequestOpenPGPInsSelectApplication()
        {
            // OpenPGP appletを選択
            byte[] aidBytes = new byte[] { 0xD2, 0x76, 0x00, 0x01, 0x24, 0x01 };
            CCIDParameter param = new CCIDParameter(OpenPGPCCIDConst.OPENPGP_INS_SELECT, 0x04, 0x00, aidBytes, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponseOpenPGPInsSelectApplication);
        }

        private void DoResponseOpenPGPInsSelectApplication(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                DoCommandResponse(false, AppCommon.MSG_ERROR_OPENPGP_APPLET_SELECT_FAILED);
                return;
            }

            // コマンドに応じ、以下の処理に分岐
            switch (Parameter.Command) {
            case Command.COMMAND_OPENPGP_INSTALL_KEYS:
                DoRequestOpenPGPInsVerify();
                break;
            case Command.COMMAND_OPENPGP_CHANGE_PIN:
            case Command.COMMAND_OPENPGP_CHANGE_ADMIN_PIN:
            case Command.COMMAND_OPENPGP_UNBLOCK_PIN:
            case Command.COMMAND_OPENPGP_SET_RESET_CODE:
            case Command.COMMAND_OPENPGP_UNBLOCK:
                DoRequestPinManagement();
                break;
            default:
                // 上位クラスに制御を戻す
                DoCommandResponse(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                break;
            }
        }

        //
        // 管理用PIN番号による認証
        //
        private void DoRequestOpenPGPInsVerify()
        {
            // パラメーターの管理用PIN番号を使用し、PIN認証を実行
            string pin = Parameter.Passphrase;
            byte[] pinBytes = Encoding.ASCII.GetBytes(pin);
            CCIDParameter param = new CCIDParameter(OpenPGPCCIDConst.OPENPGP_INS_VERIFY, 0x00, 0x83, pinBytes, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponseOpenPGPInsVerify);
        }

        private void DoResponseOpenPGPInsVerify(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                string errMsg;
                if ((responseSW & 0xfff0) == 0x63c0) {
                    // 入力PINが不正の場合はその旨のメッセージを出力
                    int retries = responseSW & 0x000f;
                    errMsg = string.Format(AppCommon.MSG_FORMAT_OPENPGP_PIN_VERIFY_ERR, AppCommon.MSG_LABEL_ITEM_PGP_ADMIN_PIN, retries);

                } else {
                    errMsg = string.Format(AppCommon.MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_ERR, AppCommon.MSG_LABEL_COMMAND_OPENPGP_ADMIN_PIN_VERIFY);
                }
                DoCommandResponse(false, errMsg);
                return;
            }

            // 上位クラスに制御を戻す
            DoCommandResponse(true, AppCommon.MSG_NONE);
        }

        //
        // PIN番号管理
        //
        private void DoRequestPinManagement()
        {
            // CCID I/F経由でPIN番号管理コマンドを実行
            new OpenPGPPinManagementProcess().DoProcess(Parameter, DoResponsePinManagement);
        }

        private void DoResponsePinManagement(bool success, string errorMessage)
        {
            // 上位クラスに制御を戻す
            DoCommandResponse(success, errorMessage);
        }
    }
}
