using System;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    class ToolPIVCcid
    {
        // CCID処理クラスの参照を保持
        CCIDProcess Process;

        // 処理機能を保持
        private AppCommon.RequestType RequestType;
        private byte CommandIns;

        // リクエストパラメーターを保持
        private ToolPIVParameter Parameter = null;

        // CCID I/Fからデータ受信時のイベント
        public delegate void CcidCommandTerminatedEvent(bool success);
        public event CcidCommandTerminatedEvent OnCcidCommandTerminated;

        public delegate void CcidCommandNotifyErrorMessageEvent(string errorMessage);
        public event CcidCommandNotifyErrorMessageEvent OnCcidCommandNotifyErrorMessage;

        public ToolPIVCcid()
        {
            // CCID処理クラスを生成
            Process = new CCIDProcess();
            Process.OnDataReceived += OnDataReceived;
        }

        //
        // PIV機能設定用関数
        // 
        public void DoPIVCcidCommand(AppCommon.RequestType requestType, ToolPIVParameter parameter)
        {
            // 画面から引き渡されたパラメーターを退避
            RequestType = requestType;
            Parameter = parameter;

            // CCIDインタフェース経由で認証器に接続
            if (StartCCIDConnection() == false) {
                // 上位クラスに制御を戻す
                OnCcidCommandTerminated(false);
                return;
            }

            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
            case AppCommon.RequestType.PIVStatus:
                // 機能実行に先立ち、PIVアプレットをSELECT
                DoRequestPIVInsSelectApplication();
                break;
            default:
                // 上位クラスに制御を戻す
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                NotifyCommandTerminated(false);
                break;
            }
        }

        private void OnDataReceived(byte[] responseData, UInt16 responseSW)
        {
            // コマンドに応じ、以下の処理に分岐
            switch (CommandIns) {
            case ToolPIVConst.PIV_INS_SELECT:
                DoResponsePIVInsSelectApplication(responseData, responseSW);
                break;
            case ToolPIVConst.PIV_INS_VERIFY:
                DoResponsePIVInsVerify(responseData, responseSW);
                break;
            default:
                // 上位クラスに制御を戻す
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
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
                // PIV機能を認識できなかった旨のエラーメッセージを設定
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_SELECTING_CARD_FAIL);
                return false;
            }
        }

        private void NotifyCommandTerminated(bool success)
        {
            // CCIDデバイスから切断し、上位クラスに制御を戻す
            Process.Disconnect();
            OnCcidCommandTerminated(success);
        }

        //
        // CCID I/Fコマンド実行関数
        //
        private void DoRequestPIVInsSelectApplication()
        {
            // PIV appletを選択
            byte[] aidBytes = new byte[] { 0xa0, 0x00, 0x00, 0x03, 0x08 };
            CommandIns = ToolPIVConst.PIV_INS_SELECT;
            Process.SendIns(CommandIns, 0x04, 0x00, aidBytes, 0xff);
        }

        private void DoResponsePIVInsSelectApplication(byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (responseSW != CCIDConst.SW_SUCCESS) {
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_APPLET_SELECT_FAILED);
                NotifyCommandTerminated(false);
                return;
            }

            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
            case AppCommon.RequestType.PIVStatus:
                // PINリトライカウンターを照会
                DoRequestPivInsVerify(null);
                break;
            default:
                // 上位クラスに制御を戻す
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                NotifyCommandTerminated(false);
                break;
            }
        }

        private void DoRequestPivInsVerify(string pinCode)
        {
            // コマンドAPDUを生成
            byte[] apdu = new byte[0];

            // コマンドを実行
            CommandIns = ToolPIVConst.PIV_INS_VERIFY;
            Process.SendIns(CommandIns, 0x00, ToolPIVConst.PIV_KEY_PIN, apdu, 0xff);
        }

        private void DoResponsePIVInsVerify(byte[] responseData, UInt16 responseSW)
        {
            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
            case AppCommon.RequestType.PIVStatus:
                // PINリトライカウンターを照会
                DoPivStatusProcessWithPinRetryResponse(responseData, responseSW);
                break;
            default:
                // 上位クラスに制御を戻す
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                NotifyCommandTerminated(false);
                break;
            }
        }

        private void DoPivStatusProcessWithPinRetryResponse(byte[] responseData, UInt16 responseSW)
        {
            if ((responseSW >> 8) == 0x63) {
                // PINリトライカウンターを取得
                byte retries = (byte)(responseSW & 0x000f);
                AppUtil.OutputLogDebug(string.Format(AppCommon.MSG_PIV_PIN_RETRY_CNT_GET, retries));

                // TODO: PIV設定情報クラスを生成し、リトライカウンターを格納
                NotifyCommandTerminated(true);

            } else {
                // 不明エラーが発生時は処理失敗ログを出力し、制御を戻す
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_PIN_RETRY_CNT_GET_FAILED);
                NotifyCommandTerminated(false);
            }
        }
    }
}
