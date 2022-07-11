﻿using System;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    internal static class Const
    {
        // HIDコマンドバイトに関する定義
        public const int HID_CMD_CTAPHID_PING = 0x81;
        public const int HID_CMD_CTAPHID_INIT = 0x86;
        public const int HID_CMD_GET_FLASH_STAT = 0xc2;
        public const int HID_CMD_GET_VERSION_INFO = 0xc3;
        public const int HID_CMD_TOOL_PREF_PARAM = 0xc4;
        public const int HID_CMD_BOOTLOADER_MODE = 0xc5;
        public const int HID_CMD_ERASE_BONDS = 0xc6;
        public const int HID_CMD_FIRMWARE_RESET = 0xc7;
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

        // 実行機能を保持
        private AppCommon.RequestType requestType;

        // 実行コマンドクラスの参照を保持
        private Object ToolCommandRef = null;

        // HID接続検知時、通知先の実行コマンドクラスの参照を保持
        private Object NeedNotifyDetectConnectRef = null;

        // DFU処理
        public ToolDFU ToolDFURef;

        // バージョン照会の処理区分
        // true=HID接続完了時に実行, false=メニューからの実行
        private bool GetVersionInfoForDFU;

        public HIDMain(MainForm f)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // HIDデバイス関連
            // イベントの登録
            hidProcess.MessageTextEvent += new HIDProcess.MessageTextEventHandler(PrintMessageText);
            hidProcess.ReceiveHIDMessageEvent += new HIDProcess.ReceiveHIDMessageEventHandler(ReceiveHIDMessage);
            hidProcess.HIDConnectedEvent += new HIDProcess.HIDConnectedEventHandler(NotifyHIDDetectConnect);

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
            ToolDFURef.OnUSBDeviceRemoveComplete();
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
            case Const.HID_CMD_ERASE_BONDS:
                // ステータスバイトをチェックし、画面に制御を戻す
                DoResponseEraseBonds(message, length);
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
            case Const.HID_CMD_BOOTLOADER_MODE:
                if (requestType == AppCommon.RequestType.GotoBootLoaderMode) {
                    // ステータスバイトをチェックし、画面に制御を戻す
                    DoResponseBootLoaderMode(message, length);
                } else {
                    ToolDFURef.NotifyBootloaderModeResponse(hidProcess.receivedCMD, message);
                }
                break;
            case Const.HID_CMD_FIRMWARE_RESET:
                DoResponseFirmwareReset(hidProcess.receivedCMD, message);
                break;
            case Const.HID_CMD_UNKNOWN_ERROR:
                if (requestType == AppCommon.RequestType.ChangeToBootloaderMode) {
                    // DFU処理から呼び出された場合は、DFU処理クラスに制御を戻す
                    ToolDFURef.NotifyBootloaderModeResponse(hidProcess.receivedCMD, message);
                } else if (requestType == AppCommon.RequestType.HidFirmwareReset) {
                    // ファームウェアリセットの場合
                    DoResponseFirmwareReset(hidProcess.receivedCMD, message);
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
        public void DoRequestCtapHidInit(AppCommon.RequestType t)
        {
            // 実行するコマンドを退避
            requestType = t;

            // 8バイトのランダムデータを生成
            new Random().NextBytes(nonceBytes);

            // INITコマンドを実行し、nonce を送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_CTAPHID_INIT, nonceBytes, nonceBytes.Length);
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
            DoResponseCtapHidInit(message, length);
        }

        public void DoResponseCtapHidInit(byte[] message, int length)
        {
            switch (requestType) {
            case AppCommon.RequestType.EraseBonds:
                DoRequestEraseBonds();
                break;
            case AppCommon.RequestType.GotoBootLoaderMode:
                DoRequestBootLoaderMode();
                break;
            case AppCommon.RequestType.HidFirmwareReset:
                DoRequestFirmwareReset();
                break;
            case AppCommon.RequestType.TestCtapHidPing:
                // PINGコマンドを実行
                ctap2.DoRequestPing();
                break;
            case AppCommon.RequestType.TestRegister:
                // U2F Registerコマンドを実行
                u2f.DoRequestRegister(requestType);
                break;
            case AppCommon.RequestType.TestAuthenticateCheck:
            case AppCommon.RequestType.TestAuthenticate:
                // U2F Authenticate処理を続行
                u2f.DoRequestAuthenticate();
                break;
            case AppCommon.RequestType.TestMakeCredential:
            case AppCommon.RequestType.TestGetAssertion:
            case AppCommon.RequestType.ClientPinSet:
                // 認証器の公開鍵を取得
                ctap2.DoGetKeyAgreement(requestType);
                break;
            case AppCommon.RequestType.AuthReset:
                // Reset処理を続行
                ctap2.DoRequestAuthReset();
                break;
            case AppCommon.RequestType.ChangeToBootloaderMode:
                // ブートローダー遷移コマンドを実行
                DoRequestCommandBootloaderMode();
                break;
            default:
                break;
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

        public void DoEraseBonds()
        {
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit(AppCommon.RequestType.EraseBonds);
        }

        public void DoRequestEraseBonds()
        {
            // コマンドバイトだけを送信する
            hidProcess.SendHIDMessage(ReceivedCID, Const.HID_CMD_ERASE_BONDS, RequestData, 0);
        }

        public void DoResponseEraseBonds(byte[] message, int length)
        {
            // ステータスバイトをチェック
            bool result = (message[0] == 0x00);
            // 画面に制御を戻す
            mainForm.OnAppMainProcessExited(result);
        }

        public void DoBootLoaderMode()
        {
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit(AppCommon.RequestType.GotoBootLoaderMode);
        }

        public void DoRequestBootLoaderMode()
        {
            // コマンドバイトだけを送信する
            hidProcess.SendHIDMessage(ReceivedCID, Const.HID_CMD_BOOTLOADER_MODE, RequestData, 0);
        }

        private void DoResponseBootLoaderMode(byte[] message, int length)
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
            // レスポンスメッセージの１バイト目（ステータスコード）を確認
            if (message[0] != 0x00) {
                // エラーの場合は画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                return;
            }
            // 戻りメッセージから、取得情報CSVを抽出
            byte[] responseBytes = AppUtil.ExtractCBORBytesFromResponse(message, length);
            string responseCSV = System.Text.Encoding.ASCII.GetString(responseBytes);
            AppUtil.OutputLogDebug("Flash ROM statistics: " + responseCSV);

            // 情報取得CSVから空き領域に関する情報を抽出
            string[] vars = responseCSV.Split(',');
            string strUsed = "";
            string strAvail = "";
            string strCorrupt = "";
            foreach (string v in vars) {
                if (v.StartsWith("words_used=")) {
                    strUsed = v.Split('=')[1];
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
            if (strUsed.Length > 0 && strAvail.Length > 0) {
                float avail = float.Parse(strAvail);
                float remaining = avail - float.Parse(strUsed);
                float rate = remaining / avail * 100;
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
            GetVersionInfoForDFU = false;
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_GET_VERSION_INFO, RequestData, 0);
        }

        public void DoGetVersionInfoForDFU()
        {
            // コマンドバイトだけを送信する
            GetVersionInfoForDFU = true;
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_GET_VERSION_INFO, RequestData, 0);
        }

        private void DoResponseGetVersionInfo(byte[] message, int length)
        {
            // 戻りメッセージから、取得情報CSVを抽出
            byte[] responseBytes = AppUtil.ExtractCBORBytesFromResponse(message, length);
            string responseCSV = System.Text.Encoding.ASCII.GetString(responseBytes);

            // 情報取得CSVからバージョンに関する情報を抽出
            string[] vars = responseCSV.Split(',');
            string strDeviceName = "";
            string strFWRev = "";
            string strHWRev = "";
            string strSecic = "";
            foreach (string v in vars) {
                if (v.StartsWith("DEVICE_NAME=")) {
                    strDeviceName = v.Split('=')[1].Replace("\"", "");
                } else if (v.StartsWith("FW_REV=")) {
                    strFWRev = v.Split('=')[1].Replace("\"", "");
                } else if (v.StartsWith("HW_REV=")) {
                    strHWRev = v.Split('=')[1].Replace("\"", "");
                } else if (v.StartsWith("ATECC608A=")) {
                    strSecic = v.Split('=')[1].Replace("\"", "");
                }
            }

            if (GetVersionInfoForDFU) {
                // DFU処理のためのバージョン照会
                // (HID接続完了時の処理) である場合、
                // DFU処理クラスにバージョンを通知
                ToolDFURef.NotifyFirmwareVersionResponse(strFWRev, strHWRev);
                return;
            }

            // 画面に制御を戻す
            mainForm.OnPrintMessageText(AppCommon.MSG_VERSION_INFO_HEADER);
            mainForm.OnPrintMessageText(string.Format(AppCommon.MSG_VERSION_INFO_DEVICE_NAME, strDeviceName));
            mainForm.OnPrintMessageText(string.Format(AppCommon.MSG_VERSION_INFO_FW_REV, strFWRev));
            mainForm.OnPrintMessageText(string.Format(AppCommon.MSG_VERSION_INFO_HW_REV, strHWRev));
            // セキュアICの搭載有無を表示
            if (strSecic.Length > 0) {
                mainForm.OnPrintMessageText(AppCommon.MSG_VERSION_INFO_SECURE_IC_AVAIL);
            } else {
                mainForm.OnPrintMessageText(AppCommon.MSG_VERSION_INFO_SECURE_IC_UNAVAIL);
            }
            mainForm.OnAppMainProcessExited(true);
        }

        public void DoAuthReset()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit(AppCommon.RequestType.AuthReset);
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
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit(AppCommon.RequestType.ClientPinSet);
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

            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit(AppCommon.RequestType.TestMakeCredential);
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
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit(AppCommon.RequestType.TestRegister);
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
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit(AppCommon.RequestType.TestCtapHidPing);
        }

        //
        // ブートローダー遷移コマンド
        //
        public void DoCommandChangeToBootloaderMode()
        {
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit(AppCommon.RequestType.ChangeToBootloaderMode);
        }

        private void DoRequestCommandBootloaderMode()
        {
            // コマンドバイトだけを送信する
            hidProcess.SendHIDMessage(ReceivedCID, Const.HID_CMD_BOOTLOADER_MODE, RequestData, 0);
        }

        //
        // ファームウェアリセットコマンド
        //
        public void DoFirmwareReset(AppCommon.RequestType requestType, Object toolCommandRef)
        {
            // 実行コマンドクラスの参照を保持
            ToolCommandRef = toolCommandRef;

            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit(requestType);
        }

        private void DoRequestFirmwareReset()
        {
            // コマンドバイトだけを送信する
            hidProcess.SendHIDMessage(ReceivedCID, Const.HID_CMD_FIRMWARE_RESET, RequestData, 0);
        }

        private void DoResponseFirmwareReset(byte receivedCmd, byte[] message)
        {
            // レスポンスメッセージの１バイト目（ステータスコード）を確認
            if (message[0] != 0x00) {
                // エラーの場合は画面に制御を戻す
                CompletedResetFirmware(false);

            } else {
                // 再接続まで待機 --> completedResetFirmware が呼び出される
                WaitForNotifyHIDDetectConnect(ToolCommandRef);
            }
        }

        private void CompletedResetFirmware(bool success)
        {
            // 実行コマンドクラスの参照がない場合は終了
            if (ToolCommandRef == null) {
                return;
            }

            // レスポンスを戻す
            if (ToolCommandRef is ToolPGP) {
                ToolPGP toolPGP = (ToolPGP)ToolCommandRef;
                toolPGP.DoResponseResetFirmware(success);
            }
            if (ToolCommandRef is ToolPIV) {
                ToolPIV toolPIV = (ToolPIV)ToolCommandRef;
                toolPIV.DoResponseResetFirmware(success);
            }

            // 実行コマンドクラスの参照を初期化
            ToolCommandRef = null;
        }

        //
        // USB接続検知時の処理
        //
        public void WaitForNotifyHIDDetectConnect(Object toolCommandRef)
        {
            // HID接続が検知されたら、所定のコマンドに通知
            NeedNotifyDetectConnectRef = toolCommandRef;
        }

        private void NotifyHIDDetectConnect()
        {
            // 通知先コマンドクラスの参照がない場合は終了
            if (NeedNotifyDetectConnectRef == null) {
                return;
            }

            // HID接続検知を所定のコマンドに通知
            if (NeedNotifyDetectConnectRef is ToolDFU) {
                ToolDFURef.OnUSBDeviceArrival();

            } else if (NeedNotifyDetectConnectRef is ToolPGP || NeedNotifyDetectConnectRef is ToolPIV) {
                if (requestType == AppCommon.RequestType.HidFirmwareReset) {
                    CompletedResetFirmware(true);
                }
            }

            // 通知先コマンドクラスの参照を初期化
            NeedNotifyDetectConnectRef = null;
        }
    }
}
