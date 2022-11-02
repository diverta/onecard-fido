using System;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.PIV
{
    internal class PIVCCIDConst
    {
        public const byte PIV_INS_SELECT = 0xA4;
        public const byte PIV_INS_VERIFY = 0x20;
        public const byte PIV_INS_AUTHENTICATE = 0x87;
        public const byte PIV_INS_PUT_DATA = 0xdb;
        public const byte YKPIV_INS_IMPORT_ASYMM_KEY = 0xfe;

        public const byte PIV_KEY_PIN = 0x80;
    }

    internal class PIVCCIDProcess
    {
        // 処理実行のためのプロパティー
        private PIVParameter Parameter = null!;

        // CCID I/Fからデータ受信時のコールバックを保持
        public delegate void HandlerOnCommandResponse(bool success, string errorMessage);
        private HandlerOnCommandResponse OnCommandResponse = null!;

        //
        // PIV機能設定用関数
        // 
        public void DoProcess(PIVParameter parameter, HandlerOnCommandResponse handlerRef)
        {
            // 引き渡されたパラメーターを退避
            Parameter = parameter;

            // コールバックを保持
            OnCommandResponse = handlerRef;

            // 機能実行に先立ち、PIVアプレットをSELECT
            DoRequestPIVInsSelectApplication();
        }

        private void DoCommandResponse(bool success, string errorMessage)
        {
            // 上位クラスに制御を戻す
            OnCommandResponse(success, errorMessage);
        }

        //
        // CCID I/Fコマンド実行関数
        //
        private void DoRequestPIVInsSelectApplication()
        {
            // PIV appletを選択
            byte[] aidBytes = new byte[] { 0xa0, 0x00, 0x00, 0x03, 0x08 };
            CCIDParameter param = new CCIDParameter(PIVCCIDConst.PIV_INS_SELECT, 0x04, 0x00, aidBytes, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponsePIVInsSelectApplication);
        }

        private void DoResponsePIVInsSelectApplication(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                DoCommandResponse(false, AppCommon.MSG_ERROR_PIV_APPLET_SELECT_FAILED);
                return;
            }

            // コマンドに応じ、以下の処理に分岐
            switch (Parameter.Command) {
            case Command.COMMAND_CCID_PIV_IMPORT_KEY:
            case Command.COMMAND_CCID_PIV_SET_CHUID:
                // PIV管理機能認証を実行
                DoRequestPivAdminAuth();
                break;
            default:
                // 上位クラスに制御を戻す
                DoCommandResponse(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                break;
            }
        }

        //
        // PIV管理機能認証
        //
        private void DoRequestPivAdminAuth()
        {
            // PIV管理機能認証を実行
            new PIVCCIDAdminAuthProcess().DoPIVCcidCommand(Parameter, DoResponsePIVAdminAuth);
        }

        private void DoResponsePIVAdminAuth(bool success, string errorMessage)
        {
            // エラーが発生時は以降の処理を行わない
            if (success == false) {
                DoCommandResponse(false, errorMessage);
                return;
            }

            // コマンドに応じ、以下の処理に分岐
            switch (Parameter.Command) {
            case Command.COMMAND_CCID_PIV_IMPORT_KEY:
                // PIN番号認証を実行
                DoRequestPivPinVerify();
                break;
            default:
                // 上位クラスに制御を戻す
                DoCommandResponse(true, AppCommon.MSG_NONE);
                break;
            }
        }

        //
        // PIN番号による認証
        //
        private void DoRequestPivPinVerify()
        {
            // PIN番号による認証を実行
            new PIVCCIDPinAuthProcess().DoPIVCcidCommand(Parameter, DoResponsePIVPinVerify);
        }

        private void DoResponsePIVPinVerify(bool success, string errorMessage)
        {
            // 上位クラスに制御を戻す
            DoCommandResponse(success, errorMessage);
        }
    }
}
