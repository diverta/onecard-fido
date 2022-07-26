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
        private UInt32 ObjectIdToFetch;

        // リクエストパラメーターを保持
        private ToolPIVParameter Parameter = null;

        // リクエストパラメーターを保持
        private ToolPIVSettingItem SettingItem = null;

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
            case ToolPIVConst.PIV_INS_GET_DATA:
                DoResponsePIVInsGetData(responseData, responseSW);
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
                AppUtil.OutputLogInfo(string.Format(AppCommon.MSG_PIV_PIN_RETRY_CNT_GET, retries));

                // PIV設定情報クラスを生成し、リトライカウンターを格納
                if (SettingItem == null) {
                    SettingItem = new ToolPIVSettingItem();
                }
                SettingItem.Retries = retries;

                // PIVオブジェクト（CHUID）を取得
                DoRequestPIVInsGetData(ToolPIVConst.PIV_OBJ_CHUID);

            } else {
                // 不明エラーが発生時は処理失敗ログを出力し、制御を戻す
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_PIN_RETRY_CNT_GET_FAILED);
                NotifyCommandTerminated(false);
            }
        }

        private void DoRequestPIVInsGetData(UInt32 objectId)
        {
            // 取得対象オブジェクトをAPDUに格納
            ObjectIdToFetch = objectId;
            byte[] apdu = GetPivInsGetApdu(objectId);

            // コマンドを実行
            CommandIns = ToolPIVConst.PIV_INS_GET_DATA;
            Process.SendIns(CommandIns, 0x3f, 0xff, apdu, 0xff);
        }


        private void DoResponsePIVInsGetData(byte[] responseData, UInt16 responseSW)
        {
            if (responseSW != CCIDConst.SW_SUCCESS) {
                // 処理失敗ログを出力（エラーではなく警告扱いとする）
                AppUtil.OutputLogWarn(string.Format(AppCommon.MSG_ERROR_PIV_DATA_OBJECT_GET_FAILED, ObjectIdToFetch));

            } else {
                // 処理成功ログを出力
                AppUtil.OutputLogInfo(string.Format(AppCommon.MSG_PIV_DATA_OBJECT_GET, ObjectIdToFetch));

                // TODO: 仮の実装です。
                string dump = AppUtil.DumpMessage(responseData, responseData.Length);
                AppUtil.OutputLogDebug(string.Format("{0} bytes\n{1}", responseData.Length, dump));
            }

            // オブジェクトIDに応じて後続処理分岐
            switch (ObjectIdToFetch) {
            case ToolPIVConst.PIV_OBJ_CHUID:
                DoRequestPIVInsGetData(ToolPIVConst.PIV_OBJ_CAPABILITY);
                break;
            case ToolPIVConst.PIV_OBJ_CAPABILITY:
                DoRequestPIVInsGetData(ToolPIVConst.PIV_OBJ_AUTHENTICATION);
                break;
            case ToolPIVConst.PIV_OBJ_AUTHENTICATION:
                DoRequestPIVInsGetData(ToolPIVConst.PIV_OBJ_SIGNATURE);
                break;
            case ToolPIVConst.PIV_OBJ_SIGNATURE:
                DoRequestPIVInsGetData(ToolPIVConst.PIV_OBJ_KEY_MANAGEMENT);
                break;
            case ToolPIVConst.PIV_OBJ_KEY_MANAGEMENT:
                NotifyCommandTerminated(true);
                break;
            default:
                OnCcidCommandNotifyErrorMessage(AppCommon.MSG_ERROR_PIV_PIN_RETRY_CNT_GET_FAILED);
                NotifyCommandTerminated(false);
                break;
            }
        }

        private byte[] GetPivInsGetApdu(UInt32 objectId)
        {
            byte[] apdu = new byte[5];
            int offset = 0;

            // 0x5c: TAG_DATA_OBJECT
            apdu[offset++] = 0x5c;

            // オブジェクト長の情報を設定
            apdu[offset++] = 3;
            apdu[offset++] = (byte)((objectId >> 16) & 0x000000ff);
            apdu[offset++] = (byte)((objectId >> 8) & 0x000000ff);
            apdu[offset++] = (byte)(objectId & 0x000000ff);
            return apdu;
        }
    }
}
