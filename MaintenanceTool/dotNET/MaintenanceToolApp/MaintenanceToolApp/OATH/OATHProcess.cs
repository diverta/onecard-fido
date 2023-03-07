using MaintenanceToolApp;
using MaintenanceToolApp.PIV;
using System;
using System.Collections.Generic;
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
        public UInt32 OATHTOTPValue { get; set; }
        public List<string> AccountList { get; set; }
        public string SelectedAccount { get; set; }

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
            OATHTOTPValue= 0;
            AccountList = new List<string>();
            SelectedAccount = string.Empty;
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

            switch (Parameter.CommandTitle) {
            case AppCommon.MSG_LABEL_COMMAND_OATH_UPDATE_TOTP:
                // ワンタイムパスワード生成処理に移行
                new OATHTotpProcess(Parameter).DoCalculate(NotifyProcessTerminated);
                break;
            case AppCommon.MSG_LABEL_COMMAND_OATH_GENERATE_TOTP:
                // アカウント登録処理-->ワンタイムパスワード生成処理を一息に実行
                DoAccountAdd();
                break;
            case AppCommon.MSG_LABEL_COMMAND_OATH_DELETE_ACCOUNT:
                // アカウント削除処理に移行
                // TODO: 仮の実装です。
                NotifyProcessTerminated(true, AppCommon.MSG_NONE);
                break;
            default:
                // アカウント一覧取得処理に移行
                new OATHAccountProcess(Parameter).DoAccountList(NotifyProcessTerminated);
                break;
            }
        }

        //
        // アカウント登録処理-->ワンタイムパスワード生成処理を一息に実行
        //
        private void DoAccountAdd()
        {
            // アカウント登録処理に移行
            new OATHAccountProcess(Parameter).DoAccountAdd(NotifyAccountAddTerminated);
        }

        private void NotifyAccountAddTerminated(bool success, string errorMessage)
        {
            // 処理失敗時は上位クラスに制御を戻す
            if (success == false) {
                NotifyProcessTerminated(success, errorMessage);
                return;
            }

            // ワンタイムパスワード生成処理に移行
            new OATHTotpProcess(Parameter).DoCalculate(NotifyProcessTerminated);
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
