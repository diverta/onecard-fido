using MaintenanceToolCommon;
using System;
using System.Text;

namespace MaintenanceToolGUI
{
    public class ToolPGPCcid
    {
        // 上位クラスの参照を保持
        ToolPGP ToolPGPRef;

        // CCID処理クラスの参照を保持
        CCIDProcess Process;

        // 処理機能を保持
        private AppCommon.RequestType RequestType;
        private byte CommandIns;

        // リクエストパラメーターを保持
        private ToolPGPParameter Parameter = null;

        public ToolPGPCcid(ToolPGP toolPGPRef)
        {
            // 上位クラスの参照を保持
            ToolPGPRef = toolPGPRef;

            // CCID処理クラスを生成
            Process = new CCIDProcess();
            Process.OnDataReceived += OnDataReceived;
        }

        //
        // OpenPGP機能設定用関数
        // 
        public void DoOpenPGPCcidCommand(AppCommon.RequestType requestType, ToolPGPParameter parameter)
        {
            // 画面から引き渡されたパラメーターを退避
            RequestType = requestType;
            Parameter = parameter;

            // CCIDインタフェース経由で認証器に接続
            if (StartCCIDConnection() == false) {
                // 上位クラスに制御を戻す
                ToolPGPRef.NotifyProcessTerminatedFromCcid(false);
                return;
            }

            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
                case AppCommon.RequestType.OpenPGPInstallKeys:
                    // 機能実行に先立ち、PIVアプレットをSELECT
                    DoRequestOpenPGPInsSelectApplication();
                    break;

                default:
                    // 上位クラスに制御を戻す
                    NotifyCommandTerminated(false);
                    break;
            }
        }

        private void OnDataReceived(byte[] responseData, UInt16 responseSW)
        {
            // コマンドに応じ、以下の処理に分岐
            switch (CommandIns) {
            case ToolPGPConst.OPENPGP_INS_SELECT:
                DoResponseOpenPGPInsSelectApplication(responseData, responseSW);
                break;

            case ToolPGPConst.OPENPGP_INS_VERIFY:
                DoResponseOpenPGPInsVerify(responseData, responseSW);
                break;

            default:
                // 上位クラスに制御を戻す
                NotifyCommandTerminated(false);
                break;
            }
        }

        private bool StartCCIDConnection()
        {
            // CCIDデバイスに接続
            if (Process.Connect()) {
                return true;

            } else {
                // OpenPGP機能を認識できなかった旨のエラーメッセージを設定
                ToolPGPRef.NotifyErrorMessageFromCcid(ToolGUICommon.MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL);
                return false;
            }
        }

        private void NotifyCommandTerminated(bool success)
        {
            // CCIDデバイスから切断し、上位クラスに制御を戻す
            Process.Disconnect();
            ToolPGPRef.NotifyProcessTerminatedFromCcid(success);
        }

        //
        // CCID I/Fコマンド実行関数
        //
        private void DoRequestOpenPGPInsSelectApplication()
        {
            // OpenPGP appletを選択
            byte[] aidBytes = new byte[] { 0xD2, 0x76, 0x00, 0x01, 0x24, 0x01 };
            CommandIns = ToolPGPConst.OPENPGP_INS_SELECT;
            Process.SendIns(CommandIns, 0x04, 0x00, aidBytes, 0xff);
        }

        private void DoResponseOpenPGPInsSelectApplication(byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (responseSW != CCIDConst.SW_SUCCESS) {
                ToolPGPRef.NotifyErrorMessageFromCcid(ToolGUICommon.MSG_ERROR_OPENPGP_APPLET_SELECT_FAILED);
                NotifyCommandTerminated(false);
                return;
            }

            // 次の処理に移行
            DoRequestOpenPGPInsVerify();
        }

        private void DoRequestOpenPGPInsVerify()
        {
            // パラメーターの管理用PIN番号を使用し、PIN認証を実行
            string pin = Parameter.Passphrase;
            byte[] pinBytes = Encoding.GetEncoding("Shift_JIS").GetBytes(pin);
            CommandIns = ToolPGPConst.OPENPGP_INS_VERIFY;
            Process.SendIns(CommandIns, 0x00, 0x83, pinBytes, 0xff);
        }

        private void DoResponseOpenPGPInsVerify(byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (responseSW != CCIDConst.SW_SUCCESS) {
                string errMsg;
                if ((responseSW & 0xfff0) == 0x63c0) {
                    // 入力PINが不正の場合はその旨のメッセージを出力
                    int retries = responseSW & 0x000f;
                    errMsg = string.Format(ToolGUICommon.MSG_FORMAT_OPENPGP_PIN_VERIFY_ERR, ToolGUICommon.MSG_LABEL_ITEM_PGP_ADMIN_PIN, retries);

                } else {
                    errMsg = string.Format(ToolGUICommon.MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_ERR, ToolGUICommon.MSG_LABEL_COMMAND_OPENPGP_ADMIN_PIN_VERIFY);
                }

                ToolPGPRef.NotifyErrorMessageFromCcid(errMsg);
                NotifyCommandTerminated(false);
                return;
            }

            // 上位クラスに制御を戻す
            NotifyCommandTerminated(true);
        }
    }
}
