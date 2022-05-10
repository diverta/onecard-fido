using ToolGUICommon;

namespace DevelopmentToolGUI
{
    class Ctap2
    {
        // トランスポート種別を保持
        private byte transportType;

        // トランスポート別処理の参照を保持
        private HIDMain hidMain;

        // メイン画面の参照を保持
        private MainForm mainForm;

        // 実行中のサブコマンドを保持
        private byte cborCommand;
        private byte cborSubCommand;

        // 実行機能を保持
        private AppCommon.RequestType requestType;

        public Ctap2(MainForm m, byte transportType_)
        {
            mainForm = m;
            transportType = transportType_;
        }

        public void SetHidMain(HIDMain p)
        {
            hidMain = p;
        }

        //
        // CBORコマンド関連処理
        //
        public void DoResponseCtapHidCbor(byte[] message, int length)
        {
            // ステータスバイトをチェック
            if (CheckStatusByte(message) == false) {
                // 画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                return;
            }

            switch (cborCommand) {
            case AppCommon.CTAP2_CBORCMD_CLIENT_PIN:
                DoResponseCommandClientPin(message, length);
                break;
            default:
                // 正しくレスポンスされなかったと判断し、画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                break;
            }
        }

        private bool CheckStatusByte(byte[] message)
        {
            switch (message[0]) {
            case 0x00:
                return true;
            case 0xfe:  // CTAP2_ERR_VENDOR_KEY_CRT_NOT_EXIST (CTAP2_ERR_VENDOR_FIRST+0x0e)
                mainForm.OnPrintMessageText(AppCommon.MSG_OCCUR_SKEYNOEXIST_ERROR);
                break;
            default:
                break;
            }
            return false;
        }

        private void DoResponseCommandClientPin(byte[] message, int length)
        {
            // レスポンスされたCBORを抽出
            byte[] cborBytes = AppUtil.ExtractCBORBytesFromResponse(message, length);

            switch (cborSubCommand) {
            case AppCommon.CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT:
                DoResponseCommandGetKeyAgreement(cborBytes);
                break;
            default:
                // 画面に制御を戻す
                mainForm.OnAppMainProcessExited(true);
                break;
            }
        }

        //
        // ClientPINコマンド関連処理
        //
        public void DoGetKeyAgreement(AppCommon.RequestType t)
        {
            // 実行するコマンドを退避
            requestType = t;
            cborCommand = AppCommon.CTAP2_CBORCMD_CLIENT_PIN;
            cborSubCommand = AppCommon.CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT;

            // GetAgreementコマンドバイトを生成
            CBOREncoder cborEncoder = new CBOREncoder();
            byte[] getAgreementCbor = cborEncoder.GetKeyAgreement(cborCommand, cborSubCommand);

            // GetAgreementコマンドを実行
            switch (transportType) {
            case AppCommon.TRANSPORT_HID:
                hidMain.SendHIDMessage(Const.HID_CMD_CTAPHID_CBOR, getAgreementCbor, getAgreementCbor.Length);
                break;
            default:
                break;
            }
        }

        public void DoResponseCommandGetKeyAgreement(byte[] cborBytes)
        {
            switch (requestType) {
            case AppCommon.RequestType.InstallSkeyCert:
                // 鍵・証明書インストール処理を続行
                hidMain.DoRequestInstallSkeyCert(cborBytes);
                break;
            default:
                // 画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                break;
            }
        }
    }
}
