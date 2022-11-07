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
        public const byte OPENPGP_INS_CHANGE_REFERENCE_DATA = 0x24;
        public const byte OPENPGP_INS_RESET_RETRY_COUNTER = 0x2C;
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
            string errorMessage;
            if (success == false) {
                errorMessage = string.Format(AppCommon.MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_ERR, AppCommon.MSG_LABEL_COMMAND_OPENPGP_ADMIN_PIN_VERIFY);
                DoCommandResponse(false, errorMessage);
                return;
            }

            // 認証が失敗した場合は以降の処理を行わない
            if (CheckPinCommandResponseSW(Parameter.Command, responseSW, out errorMessage) == false) {
                DoCommandResponse(false, errorMessage);
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

        //
        // ユーティリティー
        //
        public static bool CheckPinCommandResponseSW(Command command, UInt16 responseSW, out string errorMessage)
        {
            // エラーメッセージを初期化
            errorMessage = AppCommon.MSG_NONE;

            // ラベルを初期化
            string pinName;
            switch (command) {
            case Command.COMMAND_OPENPGP_INSTALL_KEYS:
            case Command.COMMAND_OPENPGP_CHANGE_ADMIN_PIN:
            case Command.COMMAND_OPENPGP_UNBLOCK_PIN:
            case Command.COMMAND_OPENPGP_SET_RESET_CODE:
                pinName = AppCommon.MSG_LABEL_ITEM_PGP_ADMIN_PIN;
                break;
            case Command.COMMAND_OPENPGP_UNBLOCK:
                pinName = AppCommon.MSG_LABEL_ITEM_PGP_RESET_CODE;
                break;
            default:
                pinName = AppCommon.MSG_LABEL_ITEM_PGP_PIN;
                break;
            }

            // ステータスワードをチェックし、エラーの種類を判定
            int retries = 3;
            bool isPinBlocked = false;
            if ((responseSW >> 8) == 0x63) {
                // リトライカウンターが戻された場合（入力PIN／PUKが不正時）
                retries = responseSW & 0x000f;
                if (retries < 1) {
                    isPinBlocked = true;
                }

            } else if (responseSW == CCIDProcessConst.SW_ERR_AUTH_BLOCKED) {
                // 入力PIN／PUKがすでにブロックされている場合
                isPinBlocked = true;

            } else if (responseSW != CCIDProcessConst.SW_SUCCESS) {
                // 不明なエラーが発生時
                errorMessage = string.Format(AppCommon.MSG_ERROR_PIV_UNKNOWN, responseSW);
            }

            // PINブロック or リトライカウンターの状態に応じメッセージを編集
            if (isPinBlocked) {
                if (pinName.Equals(AppCommon.MSG_LABEL_ITEM_PGP_ADMIN_PIN)) {
                    errorMessage = AppCommon.MSG_ERROR_OPENPGP_ADMIN_PIN_LOCKED;

                } else if (pinName.Equals(AppCommon.MSG_LABEL_ITEM_PGP_RESET_CODE)) {
                    errorMessage = AppCommon.MSG_ERROR_OPENPGP_RESET_CODE_LOCKED;

                } else {
                    errorMessage = AppCommon.MSG_ERROR_OPENPGP_PIN_LOCKED;
                }

            } else if (retries < 3) {
                errorMessage = string.Format(AppCommon.MSG_FORMAT_OPENPGP_PIN_VERIFY_ERR, pinName, retries);
            }

            return (responseSW == CCIDProcessConst.SW_SUCCESS);
        }
    }
}
