﻿using System;
using System.IO;
using System.Text;

namespace U2FHelper
{
    static class AppCommon
    {
        // macOS版と共通のメッセージ文言を使用
        // 共通
        public const string MSG_INVALID_FILE_PATH = "ファイルが存在しません。";
        public const string MSG_SUCCESS = "成功";
        public const string MSG_FAILURE = "失敗";

        // ホーム画面
        public const string MSG_FORMAT_START_MESSAGE = "{0}を開始します。";
        public const string MSG_FORMAT_END_MESSAGE = "{0}が{1}しました。";

        // コマンド種別に対応する処理名称
        public const string PROCESS_NAME_PAIRING = "ペアリング";

        // Windows版固有のメッセージ文言
        // USB管理
        public const string MSG_U2FCOMMAND_PROCESS = "U2F管理コマンドの実行";
        public const string MSG_HID_BLE_CONNECTION = "U2F HIDデバイスから要求された処理";
        public const string MSG_USB_DETECT_FAILED = "USBデバイス検知の開始に失敗しました。";
        public const string MSG_USB_DETECT_STARTED = "USBデバイス検知を開始しました。";
        public const string MSG_USB_DETECT_END = "USBデバイス検知を終了しました。";
        public const string MSG_HID_REMOVED = "U2F HIDデバイスが取り外されました。";
        public const string MSG_HID_CONNECTED = "U2F HIDデバイスに接続されました。";
        public const string MSG_HID_MESSAGE_TRANSFERRED = "U2F HIDデバイスからメッセージが転送されました。";
        public const string MSG_BLE_MESSAGE_TRANSFERRED = "U2F BLEデバイスからメッセージが転送されました。";
        public const string MSG_FORMAT_NOT_INSTALLED = "{0}が導入されていません。";
        public const string MSG_FORMAT_PROCESS_STARTED = "{0}を開始しました: {1} {2}";
        public const string MSG_FORMAT_PROCESS_EXITED = "{0}が{1}しました: {2} {3}";

        // BLE関連のメッセージ文言
        public const string MSG_BLE_U2F_SERVICE_NOT_FOUND = "FIDO BLE U2Fサービスが見つかりません。";
        public const string MSG_BLE_U2F_SERVICE_FOUND = "FIDO BLE U2Fサービスが見つかりました。";
        public const string MSG_U2F_DEVICE_CONNECT_FAILED = "FIDO U2Fデバイスの接続に失敗しました。";
        public const string MSG_U2F_DEVICE_CONNECTED = "FIDO U2Fデバイスに接続しました。";
        public const string MSG_U2F_DEVICE_DISCONNECTED = "FIDO U2Fデバイスの接続が切断されました。";
        public const string MSG_BLE_CHARACT_NOT_DISCOVERED = "FIDO BLE U2Fサービスと通信できません。";
        public const string MSG_BLE_NOTIFICATION_FAILED = "FIDO BLE U2Fサービスからデータを受信できません。";
        public const string MSG_BLE_NOTIFICATION_START = "受信データの監視を開始します。";
        public const string MSG_REQUEST_SEND_FAILED = "リクエスト送信が失敗しました。";
        public const string MSG_REQUEST_SENT = "リクエストを送信しました。";

        // ファイル名
        public const string FILENAME_U2FHELPER_LOG = "U2FHelper.log";
        public const string FILENAME_U2FCOMMAND_EXE = "U2FMaintenanceToolCMD.exe";

        public static void OutputLogText(string logText)
        {
            // ログファイルにメッセージを出力する
            string fname = FILENAME_U2FHELPER_LOG;
            StreamWriter sr = new StreamWriter(
                (new FileStream(fname, FileMode.Append)), Encoding.Default);
            sr.WriteLine(logText);
            sr.Close();
        }

        public static void OutputLogToFile(string message, bool printTimeStamp)
        {
            // メッセージに現在時刻を付加する
            string formatted;
            if (printTimeStamp) {
                formatted = string.Format("{0} {1}", DateTime.Now.ToString(), message);
            } else {
                formatted = string.Format("{0}", message);
            }

            // ログファイルにメッセージを出力する
            OutputLogText(formatted);
        }

        public static string DumpMessage(byte[] message, int length)
        {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < length; i++) {
                sb.Append(string.Format("{0:x2} ", message[i]));
                if ((i % 16 == 15) && (i < length - 1)) {
                    sb.Append("\r\n");
                }
            }
            return sb.ToString();
        }
    }
}
