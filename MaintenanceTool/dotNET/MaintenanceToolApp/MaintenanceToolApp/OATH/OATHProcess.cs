using MaintenanceToolApp;
using System.Collections.Generic;
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

        public OATHParameter()
        {
            CommandTitle = string.Empty;
            Command = COMMAND_NONE;
            ResultMessage = string.Empty;
            ResultInformativeMessage = string.Empty;
            Transport = TRANSPORT_NONE;
            OATHAccountName = string.Empty;
            OATHAccountIssuer = string.Empty;
        }
    }

    public class OATHProcess
    {
        // 処理実行のためのプロパティー
        private readonly OATHParameter Parameter;

        public OATHProcess(OATHParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
        }

        public void DoProcess()
        {
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
            return true;
        }
    }
}
