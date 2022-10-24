using System;
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

        // CCID I/Fからデータ受信時のイベント
        public delegate void HandlerOnCommandResponse(bool success, string errorMessage);
        private event HandlerOnCommandResponse OnCommandResponse = null!;
        private HandlerOnCommandResponse OnCommandResponseRef = null!;

        //
        // OpenPGP機能設定用関数
        // 
        public void DoOpenPGPCcidCommand(OpenPGPParameter parameter, HandlerOnCommandResponse handlerRef)
        {
            // 引き渡されたパラメーターを退避
            Parameter = parameter;

            // イベントを登録
            OnCommandResponseRef = handlerRef;
            OnCommandResponse += OnCommandResponseRef;

            // CCIDインタフェース経由で認証器に接続
            if (CCIDProcess.ConnectCCID() == false) {
                // OpenPGP機能を認識できなかった旨のエラーメッセージを設定し
                // 上位クラスに制御を戻す
                OnCommandResponse(false, AppCommon.MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL);
                return;
            }

            // コマンドに応じ、以下の処理に分岐
            switch (Parameter.Command) {
            case Command.COMMAND_OPENPGP_INSTALL_KEYS:
                // 機能実行に先立ち、PIVアプレットをSELECT
                DoRequestOpenPGPInsSelectApplication();
                break;

            default:
                // 上位クラスに制御を戻す
                DoCommandResponse(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                break;
            }
        }

        private void DoCommandResponse(bool success, string errorMessage)
        {
            // CCIDデバイスから切断し、上位クラスに制御を戻す
            CCIDProcess.DisconnectCCID();
            OnCommandResponse(success, errorMessage);

            // イベントを解除
            OnCommandResponse -= OnCommandResponseRef;
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

            // TODO: 仮の実装です。
            DoCommandResponse(true, AppCommon.MSG_NONE);
        }
    }
}
