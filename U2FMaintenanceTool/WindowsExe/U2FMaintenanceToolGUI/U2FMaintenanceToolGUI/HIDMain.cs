using U2FMaintenanceToolCommon;

namespace U2FMaintenanceToolGUI
{
    internal static class Const
    {
        // HIDフレームに関する定義
        public const int HID_FRAME_LEN = 64;
        public const int HID_INIT_HEADER_LEN = 7;
        public const int HID_CONT_HEADER_LEN = 5;
        // HIDコマンドバイトに関する定義
        public const int HID_CMD_CTAPHID_INIT = 0x86;
        public const int HID_CMD_ERASE_SKEY_CERT = 0xc0;
        public const int HID_CMD_INSTALL_SKEY_CERT = 0xc1;
    }

    internal class HIDMain
    {
        // メイン画面の参照を保持
        private MainForm mainForm;
        // HIDデバイス関連
        private HIDProcess hidProcess = new HIDProcess();

        // ブロードキャストCID、nonceを保持
        private readonly byte[] CIDBytes = { 0xff, 0xff, 0xff, 0xff};
        private readonly byte[] nonceBytes = {0x71, 0xcb, 0x1c, 0x3b, 0x10, 0x8e, 0xc9, 0x24};

        public HIDMain(MainForm f)
        {
            // メイン画面の参照を保持
            mainForm = f;
            AppCommon.logFileName = "U2FMaintenanceToolGUI.log";

            // HIDデバイス関連
            // イベントの登録
            hidProcess.MessageTextEvent += new HIDProcess.MessageTextEventHandler(PrintMessageText);
            hidProcess.ReceiveHIDMessageEvent += new HIDProcess.ReceiveHIDMessageEventHandler(ReceiveHIDMessage);

            // FIDOデバイスに接続
            //  ウィンドウのハンドルを引き渡す
            hidProcess.OnFormCreate(mainForm.Handle);
        }

        public void OnFormDestroy()
        {
            // U2F HIDデバイスを切断
            hidProcess.OnFormDestroy();
        }

        public void OnUSBDeviceArrival()
        {
            hidProcess.OnUSBDeviceArrival();
        }

        public void OnUSBDeviceRemoveComplete()
        {
            hidProcess.OnUSBDeviceRemoveComplete();
        }

        private void ReceiveHIDMessage(byte[] message, int length)
        {
            // HIDデバイスからメッセージ受信時の処理を行う
            if (hidProcess.receivedCMD == Const.HID_CMD_CTAPHID_INIT) {
                DoResponseTestCtapHidInit(message, length);
            }
        }

        private void PrintMessageText(string messageText)
        {
            // 画面のテキストエリアにメッセージを表示
            mainForm.OnPrintMessageText(messageText);
        }

        public void DoTestCtapHidInit()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (hidProcess.IsUSBDeviceDisconnected()) {
                PrintMessageText(AppCommon.MSG_CMDTST_PROMPT_USB_PORT_SET);
                mainForm.OnAppMainProcessExited(false);
                return;
            }

            // nonce を送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_CTAPHID_INIT, nonceBytes);
        }

        private void DoResponseTestCtapHidInit(byte[] message, int length)
        {
            // nonceの一致チェック
            bool result = true;
            for (int i = 0; i < nonceBytes.Length; i++) {
                if (nonceBytes[i] != message[i]) {
                    result = false;
                    break;
                }
            }
            // 画面に制御を戻す
            mainForm.OnAppMainProcessExited(result);
        }
    }
}
