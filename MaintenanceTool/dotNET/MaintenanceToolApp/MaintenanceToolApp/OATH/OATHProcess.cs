﻿using MaintenanceToolApp;
using MaintenanceToolApp.PIV;
using System;
using System.Collections.Generic;
using System.Text;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;
using static MaintenanceToolApp.AppDefine.Command;
using static MaintenanceToolApp.AppDefine.Transport;

namespace MaintenanceTool.OATH
{
    public class OATHParameter
    {
        public string CommandTitle { get; set; }
        public Command Command { get; set; }
        public bool CommandSuccess { get; set; }
        public string ResultMessage { get; set; }
        public string ResultInformativeMessage { get; set; }
        public Transport Transport { get; set; }
        //
        // 以下は処理生成中に設定
        //
        public string OATHAccountName { get; set; }
        public string OATHAccountIssuer { get; set; }
        public string OATHBase32Secret { get; set; }

        public OATHParameter()
        {
            CommandTitle = string.Empty;
            Command = COMMAND_NONE;
            ResultMessage = string.Empty;
            ResultInformativeMessage = string.Empty;
            Transport = TRANSPORT_NONE;
            OATHAccountName = string.Empty;
            OATHAccountIssuer = string.Empty;
            OATHBase32Secret= string.Empty;
        }
    }

    public class OATHProcess
    {
        // 処理実行のためのプロパティー
        private readonly OATHParameter Parameter;

        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnNotifyProcessTerminated(OATHParameter parameter);
        private HandlerOnNotifyProcessTerminated OnNotifyProcessTerminated = null!;

        public OATHProcess(OATHParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
        }

        public void DoProcess(HandlerOnNotifyProcessTerminated handlerRef)
        {
            // コールバックを保持
            OnNotifyProcessTerminated = handlerRef;

            // 処理開始を通知
            NotifyProcessStarted();

            // CCIDインタフェース経由で認証器に接続
            if (CCIDProcess.ConnectCCID() == false) {
                // OATH機能を認識できなかった旨のエラーメッセージを設定し
                // 上位クラスに制御を戻す
                NotifyProcessTerminated(false, AppCommon.MSG_ERROR_OATH_APPLET_SELECT_FAILED);
                return;
            }

            // 機能実行に先立ち、アプレットをSELECT
            DoRequestInsSelectApplication();
        }

        // 
        // 共通処理
        //
        private void NotifyProcessStarted()
        {
            // コマンド開始メッセージをログファイルに出力
            string startMsg = string.Format(AppCommon.MSG_FORMAT_START_MESSAGE, Parameter.CommandTitle);
            AppLogUtil.OutputLogInfo(startMsg);
        }

        private void NotifyProcessTerminated(bool success, string errorMessage)
        {
            // CCIDデバイスから切断
            CCIDProcess.DisconnectCCID();

            // エラーメッセージを画面＆ログ出力
            if (success == false && errorMessage.Length > 0) {
                // ログ出力する文言からは、改行文字を除去
                AppLogUtil.OutputLogError(AppUtil.ReplaceCRLF(errorMessage));
                Parameter.ResultInformativeMessage = errorMessage;
            }

            // コマンドの実行結果をログ出力
            string formatted = string.Format(AppCommon.MSG_FORMAT_END_MESSAGE,
                Parameter.CommandTitle,
                success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);
            if (success) {
                AppLogUtil.OutputLogInfo(formatted);
            } else {
                AppLogUtil.OutputLogError(formatted);
            }

            // パラメーターにコマンド成否を設定
            Parameter.CommandSuccess = success;
            Parameter.ResultMessage = formatted;

            // 画面に制御を戻す            
            OnNotifyProcessTerminated(Parameter);
        }

        //
        // CCID I/Fコマンド実行関数
        //
        private void DoRequestInsSelectApplication()
        {
            // OATH appletを選択
            byte[] aidBytes = new byte[] { 0xa0, 0x00, 0x00, 0x05, 0x27, 0x21, 0x01 };
            CCIDParameter param = new CCIDParameter(PIVCCIDConst.PIV_INS_SELECT, 0x04, 0x00, aidBytes, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponseInsSelectApplication);
        }

        private void DoResponseInsSelectApplication(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                NotifyProcessTerminated(false, string.Format(AppCommon.MSG_OCCUR_UNKNOWN_ERROR_SW, responseSW));
                return;
            }

            // アカウント登録処理に移行
            DoRequestAccountAdd(Parameter);
        }

        //
        // アカウント登録処理
        //
        private void DoRequestAccountAdd(OATHParameter parameter)
        {
            // APDUを生成
            byte[] apduBytes;
            if (GenerateAccountAddAPDU(parameter, out apduBytes) == false) {
                NotifyProcessTerminated(false, "アカウント登録処理用のリクエストデータ生成に失敗しました。");
                return;
            }

            // TODO: 仮の実装です。
            NotifyProcessTerminated(true, AppCommon.MSG_NONE);
        }

        public static bool GenerateAccountAddAPDU(OATHParameter Parameter, out byte[] apduBytes)
        {
            apduBytes = Array.Empty<byte>();
            try {
                // アカウント名をバイト配列化
                string accountText = string.Format("{0}:{1}", Parameter.OATHAccountIssuer, Parameter.OATHAccountName);
                byte[] accountBytes = Encoding.ASCII.GetBytes(accountText);

                // Secret（Base32暗号テキスト）をバイト配列化
                byte[] secretBytes;
                Base32Util.Decode(Parameter.OATHBase32Secret, out secretBytes);

                // 変数初期化
                int apduBytesSize = 2 + accountBytes.Length + 4 + secretBytes.Length;
                apduBytes = new byte[apduBytesSize];
                int offset = 0;

                // アカウント
                apduBytes[offset++] = 0x71;
                apduBytes[offset++] = (byte)accountBytes.Length;

                // アカウントをコピー
                Array.Copy(accountBytes, 0, apduBytes, offset, accountBytes.Length);
                offset += accountBytes.Length;

                // Secret
                apduBytes[offset++] = 0x73;
                apduBytes[offset++] = (byte)(secretBytes.Length + 2);
                apduBytes[offset++] = 0x21;
                apduBytes[offset++] = 0x06;

                // Secretをコピー
                Array.Copy(secretBytes, 0, apduBytes, offset, secretBytes.Length);
                return true;

            } catch (Exception e) {
                AppLogUtil.OutputLogError(string.Format("OATHProcess.GenerateAccountAddAPDU: {0}", e.Message));
                return false;
            }
        }

        //
        // QRコードのスキャン
        //
        public static bool ScanQRCode(OATHParameter parameter)
        {
            // QRコードのスキャンを実行
            Dictionary<string, string> parsedQRCodeInfo = new Dictionary<string, string>();
            if (QRCodeUtil.ScanQRCodeFromScreenShot(parsedQRCodeInfo) == false) {
                parameter.ResultInformativeMessage = AppCommon.MSG_ERROR_OATH_QRCODE_SCAN_FAILED;
                return false;
            }

            // スキャンしたアカウント情報の項目有無をチェック
            if (QRCodeUtil.CheckScannedAccountInfo(parsedQRCodeInfo) == false) {
                parameter.ResultInformativeMessage = AppCommon.MSG_ERROR_OATH_SCANNED_ACCOUNT_INFO_INVALID;
                return false;
            }

            // アカウント情報の各項目をパラメーターに設定
            parameter.OATHAccountName = parsedQRCodeInfo["account"];
            parameter.OATHAccountIssuer = parsedQRCodeInfo["issuer"];
            parameter.OATHBase32Secret = parsedQRCodeInfo["secret"];
            return true;
        }
    }
}
