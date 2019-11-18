using MaintenanceToolCommon;
using System;

namespace MaintenanceToolGUI
{
    internal static class Const
    {
        // HIDフレームに関する定義
        public const int HID_FRAME_LEN = 64;
        public const int HID_INIT_HEADER_LEN = 7;
        public const int HID_CONT_HEADER_LEN = 5;
        // HIDコマンドバイトに関する定義
        public const int HID_CMD_CTAPHID_PING = 0x81;
        public const int HID_CMD_CTAPHID_INIT = 0x86;
        public const int HID_CMD_ERASE_SKEY_CERT = 0xc0;
        public const int HID_CMD_INSTALL_SKEY_CERT = 0xc1;
        public const int HID_CMD_GET_FLASH_STAT = 0xc2;
        public const int HID_CMD_GET_VERSION_INFO = 0xc3;
        public const int HID_CMD_TOOL_PREF_PARAM = 0xc4;
        public const int HID_CMD_CTAPHID_CBOR = 0x90;
        public const int HID_CMD_UNKNOWN_ERROR = 0xbf;
    }

    public class HIDMain
    {
        // メイン画面の参照を保持
        private MainForm mainForm;
        // HIDデバイス関連
        private HIDProcess hidProcess = new HIDProcess();

        // CTAP2、U2F共通処理
        private Ctap2 ctap2;
        private U2f u2f;

        // ツール設定処理
        private ToolPreference toolPreference;

        // ブロードキャストCIDを保持
        private readonly byte[] CIDBytes = { 0xff, 0xff, 0xff, 0xff};

        // INITコマンドで受信したCIDを保持
        private byte[] ReceivedCID = null;

        // nonceを保持
        private byte[] nonceBytes = new byte[8];

        // リクエストデータ格納領域
        private byte[] RequestData = new byte[1024];

        // 当初リクエストされたHIDコマンドを退避
        private byte requestedCMD;

        public HIDMain(MainForm f)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // HIDデバイス関連
            // イベントの登録
            hidProcess.MessageTextEvent += new HIDProcess.MessageTextEventHandler(PrintMessageText);
            hidProcess.ReceiveHIDMessageEvent += new HIDProcess.ReceiveHIDMessageEventHandler(ReceiveHIDMessage);

            // FIDOデバイスに接続
            //  ウィンドウのハンドルを引き渡す
            hidProcess.OnFormCreate(mainForm.Handle);

            // CTAP2共通処理に各種参照を引き渡す
            ctap2 = new Ctap2(mainForm, AppCommon.TRANSPORT_HID);
            ctap2.SetHidMain(this);

            // U2F共通処理に各種参照を引き渡す
            u2f = new U2f(mainForm, AppCommon.TRANSPORT_HID);
            u2f.SetHidMain(this);
        }

        public void OnFormDestroy()
        {
            // U2F HIDデバイスを切断
            hidProcess.OnFormDestroy();
        }

        public void OnUSBDeviceArrival()
        {
            // USB HIDデバイスに接続
            hidProcess.OnUSBDeviceArrival();
        }

        public void OnUSBDeviceRemoveComplete()
        {
            hidProcess.OnUSBDeviceRemoveComplete();
        }

        private void ReceiveHIDMessage(byte[] message, int length)
        {
            // HIDデバイスからメッセージ受信時の処理を行う
            switch(hidProcess.receivedCMD) {
            case Const.HID_CMD_CTAPHID_PING:
                ctap2.DoResponsePing(message, length);
                break;
            case Const.HID_CMD_CTAPHID_INIT:
                DoResponseTestCtapHidInit(message, length);
                break;
            case Const.HID_CMD_ERASE_SKEY_CERT:
            case Const.HID_CMD_INSTALL_SKEY_CERT:
                DoResponseMaintSkeyCert(message, length);
                break;
            case Const.HID_CMD_GET_FLASH_STAT:
                DoResponseGetFlashStat(message, length);
                break;
            case Const.HID_CMD_GET_VERSION_INFO:
                DoResponseGetVersionInfo(message, length);
                break;
            case Const.HID_CMD_CTAPHID_CBOR:
                ctap2.DoResponseCtapHidCbor(message, length);
                break;
            case U2f.Const.BLE_CMD_MSG:
                u2f.DoResponse(message, length);
                break;
            case Const.HID_CMD_TOOL_PREF_PARAM:
                toolPreference.DoResponseToolPreference(message, length);
                break;
            case Const.HID_CMD_UNKNOWN_ERROR:
                if (requestedCMD == Const.HID_CMD_TOOL_PREF_PARAM) {
                    // ツール設定から呼び出された場合は、ツール設定クラスに制御を戻す
                    toolPreference.OnHidMainProcessExited(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                } else {
                    // メイン画面に制御を戻す
                    mainForm.OnPrintMessageText(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                    mainForm.OnAppMainProcessExited(false);
                }
                break;
            }
        }

        private void PrintMessageText(string messageText)
        {
            // 画面のテキストエリアにメッセージを表示
            mainForm.OnPrintMessageText(messageText);
        }

        public bool IsUSBDeviceDisconnected()
        {
            // USB HID接続がない場合は true
            if (hidProcess.IsUSBDeviceDisconnected()) {
                return true;
            }
            return false;
        }

        private bool CheckUSBDeviceDisconnected()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (hidProcess.IsUSBDeviceDisconnected()) {
                PrintMessageText(AppCommon.MSG_CMDTST_PROMPT_USB_PORT_SET);
                mainForm.OnAppMainProcessExited(false);
                return true;
            }
            return false;
        }

        public void SendHIDMessage(byte cmd, byte[] message, int length)
        {
            if (ReceivedCID != null) {
                requestedCMD = cmd;
                hidProcess.SendHIDMessage(ReceivedCID, cmd, message, length);
            }
        }

        //
        // CTAP2HID_INITコマンド関連処理
        //
        public void DoRequestCtapHidInit()
        {
            // 8バイトのランダムデータを生成
            new Random().NextBytes(nonceBytes);

            // INITコマンドを実行し、nonce を送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_CTAPHID_INIT, nonceBytes, nonceBytes.Length);
        }

        public void DoRequestCtapHidInitByToolPreference(ToolPreference tp)
        {
            toolPreference = tp;
            DoRequestCtapHidInit();
        }

        private void DoResponseTestCtapHidInit(byte[] message, int length)
        {
            // nonceの一致チェック
            for (int i = 0; i < nonceBytes.Length; i++) {
                if (nonceBytes[i] != message[i]) {
                    // nonceが一致しない場合は異常終了
                    PrintMessageText(AppCommon.MSG_CMDTST_INVALID_NONCE);
                    mainForm.OnAppMainProcessExited(false);
                    return;
                }
            }

            // レスポンスされたCIDを抽出し、クラス内で保持
            ReceivedCID = ExtractReceivedCID(message);

            // INITコマンドの後続処理判定
            if (ctap2.DoResponseCtapHidInit(message, length) == false) {
                if (u2f.DoResponseHidInit(message, length) == false) {
                    toolPreference.DoResponseHidInit(message, length);
                }
            }
        }

        private byte[] ExtractReceivedCID(byte[] message)
        {
            // メッセージからCIDを抽出して戻す
            byte[] receivedCID = new byte[4];
            for (int j = 0; j < receivedCID.Length; j++) {
                receivedCID[j] = message[8 + j];
            }
            return receivedCID;
        }

        public void DoEraseSkeyCert()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // コマンドバイトだけを送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_ERASE_SKEY_CERT, RequestData, 0);
        }

        public void DoInstallSkeyCert(string skeyFilePath, string certFilePath)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // 秘密鍵をファイルから読込
            InstallSkeyCert installSkeyCert = new InstallSkeyCert();
            if (installSkeyCert.ReadPemFile(skeyFilePath) == false) {
                mainForm.OnAppMainProcessExited(false);
                return;
            }
            // 証明書をファイルから読込
            if (installSkeyCert.ReadCertFile(certFilePath) == false) {
                mainForm.OnAppMainProcessExited(false);
                return;
            }
            // 秘密鍵・証明書の内容を配列にセットし、HIDデバイスに送信
            int RequestDataSize = installSkeyCert.GenerateInstallSkeyCertBytes(RequestData);
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_INSTALL_SKEY_CERT, RequestData, RequestDataSize);
        }

        private void DoResponseMaintSkeyCert(byte[] message, int length)
        {
            // ステータスバイトをチェック
            bool result = (message[0] == 0x00);
            // 画面に制御を戻す
            mainForm.OnAppMainProcessExited(result);
        }

        public void DoGetFlashStat()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // コマンドバイトだけを送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_GET_FLASH_STAT, RequestData, 0);
        }

        private void DoResponseGetFlashStat(byte[] message, int length)
        {
            // 戻りメッセージから、取得情報CSVを抽出
            byte[] responseBytes = AppCommon.ExtractCBORBytesFromResponse(message, length);
            string responseCSV = System.Text.Encoding.ASCII.GetString(responseBytes);
            AppCommon.OutputLogToFile("Flash ROM statistics: " + responseCSV);

            // 情報取得CSVから空き領域に関する情報を抽出
            string[] vars = responseCSV.Split(',');
            string strRemain = "";
            string strAvail = "";
            string strCorrupt = "";
            foreach (string v in vars) {
                if (v.StartsWith("largest_contig=")) {
                    strRemain = v.Split('=')[1];
                }
                else if (v.StartsWith("words_available=")) {
                    strAvail = v.Split('=')[1];
                }
                else if (v.StartsWith("corruption=")) {
                    strCorrupt = v.Split('=')[1];
                }
            }

            // 空き容量、破損状況を画面に表示
            string rateText = "";
            if (strRemain.Length > 0 && strAvail.Length > 0) {
                float rate = float.Parse(strRemain) / float.Parse(strAvail) * 100;
                rateText = string.Format(AppCommon.MSG_FSTAT_REMAINING_RATE, rate);
            } else {
                rateText = AppCommon.MSG_FSTAT_NON_REMAINING_RATE;
            }
            string corruptText = (strCorrupt.Equals("0")) ? 
                AppCommon.MSG_FSTAT_CORRUPTING_AREA_NOT_EXIST : AppCommon.MSG_FSTAT_CORRUPTING_AREA_EXIST;

            // 画面に制御を戻す
            mainForm.OnPrintMessageText(string.Format("  {0}{1}", rateText, corruptText));
            mainForm.OnAppMainProcessExited(true);
        }

        public void DoGetVersionInfo()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // コマンドバイトだけを送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_GET_VERSION_INFO, RequestData, 0);
        }

        private void DoResponseGetVersionInfo(byte[] message, int length)
        {
            // 戻りメッセージから、取得情報CSVを抽出
            byte[] responseBytes = AppCommon.ExtractCBORBytesFromResponse(message, length);
            string responseCSV = System.Text.Encoding.ASCII.GetString(responseBytes);
            AppCommon.OutputLogToFile("FIDO authenticator version info: " + responseCSV);

            // 情報取得CSVからバージョンに関する情報を抽出
            string[] vars = responseCSV.Split(',');
            string strDeviceName = "";
            string strFWRev = "";
            string strHWRev = "";
            foreach (string v in vars) {
                if (v.StartsWith("DEVICE_NAME=")) {
                    strDeviceName = v.Split('=')[1].Replace("\"", "");
                } else if (v.StartsWith("FW_REV=")) {
                    strFWRev = v.Split('=')[1].Replace("\"", "");
                } else if (v.StartsWith("HW_REV=")) {
                    strHWRev = v.Split('=')[1].Replace("\"", "");
                }
            }

            // 画面に制御を戻す
            mainForm.OnPrintMessageText(AppCommon.MSG_VERSION_INFO_HEADER);
            mainForm.OnPrintMessageText(string.Format(AppCommon.MSG_VERSION_INFO_DEVICE_NAME, strDeviceName));
            mainForm.OnPrintMessageText(string.Format(AppCommon.MSG_VERSION_INFO_FW_REV, strFWRev));
            mainForm.OnPrintMessageText(string.Format(AppCommon.MSG_VERSION_INFO_HW_REV, strHWRev));
            mainForm.OnAppMainProcessExited(true);
        }

        public void DoAuthReset()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // 実行するコマンドを退避
            ctap2.SetRequestType(Ctap2.RequestType.AuthReset);
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit();
        }

        public void DoClientPinSet(string pinNew, string pinOld)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // 実行引数を退避
            ctap2.SetClientPin(pinOld);
            ctap2.SetClientPinNew(pinNew);
            // 実行するコマンドを退避
            ctap2.SetRequestType(Ctap2.RequestType.ClientPinSet);
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit();
        }

        //
        // HID CTAP2ヘルスチェック
        //
        public void DoCtap2Healthcheck(string pin)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }

            // 実行するコマンドと引数を退避
            //   認証器からPINトークンを取得するため、
            //   ClientPINコマンド（getKeyAgreement）を
            //   事前実行する必要あり
            ctap2.SetClientPin(pin);
            ctap2.SetRequestType(Ctap2.RequestType.TestMakeCredential);

            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit();
        }

        //
        // HID U2Fヘルスチェック
        //
        public void DoU2FHealthCheck()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // 実行するコマンドを退避
            ctap2.SetRequestType(Ctap2.RequestType.None);
            u2f.SetRequestType(U2f.RequestType.TestRegister);
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit();
        }

        //
        // HID PINGテスト
        //
        public void DoTestCtapHidPing()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // 実行するコマンドを退避
            ctap2.SetRequestType(Ctap2.RequestType.TestCtapHidPing);
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit();
        }
    }
}
