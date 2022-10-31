using System;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.PIV
{
    internal class PIVConst
    {
        public const byte PIV_KEY_AUTHENTICATION = 0x9a;
        public const byte PIV_KEY_CARDMGM = 0x9b;
        public const byte PIV_KEY_SIGNATURE = 0x9c;
        public const byte PIV_KEY_KEYMGM = 0x9d;

        public const UInt32 PIV_OBJ_AUTHENTICATION = 0x5fc105;
        public const UInt32 PIV_OBJ_SIGNATURE = 0x5fc10a;
        public const UInt32 PIV_OBJ_KEY_MANAGEMENT = 0x5fc10b;

        public const byte TAG_DYNAMIC_AUTH_TEMPLATE = 0x7c;
        public const byte TAG_AUTH_WITNESS = 0x80;
        public const byte TAG_AUTH_CHALLENGE = 0x81;
        public const byte TAG_DATA_OBJECT = 0x5c;
        public const byte TAG_DATA_OBJECT_VALUE = 0x53;
        public const byte TAG_CERT = 0x70;
        public const byte TAG_CERT_COMPRESS = 0x71;
        public const byte TAG_CERT_LRC = 0xfe;
    }

    public class PIVParameter
    {
        public Command Command { get; set; }
        public string CommandTitle { get; set; }
        public string CommandDesc { get; set; }
        public bool CommandSuccess { get; set; }
        public string ResultMessage { get; set; }
        public string ResultInformativeMessage { get; set; }
        public string PkeyFilePath1 { get; set; }
        public string CertFilePath1 { get; set; }
        public string PkeyFilePath2 { get; set; }
        public string CertFilePath2 { get; set; }
        public string PkeyFilePath3 { get; set; }
        public string CertFilePath3 { get; set; }
        public string AuthPin { get; set; }
        public string CurrentPin { get; set; }
        public string NewPin { get; set; }
        //
        // 以下は処理生成中に設定
        //
        public PIVImportKeyParameter ImportKeyParameter1 { get; set; }
        public PIVImportKeyParameter ImportKeyParameter2 { get; set; }
        public PIVImportKeyParameter ImportKeyParameter3 { get; set; }
        public byte[] PivAuthChallenge = Array.Empty<byte>();

        public PIVParameter()
        {
            Command = Command.COMMAND_NONE;
            CommandTitle = string.Empty;
            CommandDesc = string.Empty;
            ResultMessage = string.Empty;
            ResultInformativeMessage = string.Empty;
            PkeyFilePath1 = string.Empty;
            CertFilePath1 = string.Empty;
            PkeyFilePath2 = string.Empty;
            CertFilePath2 = string.Empty;
            PkeyFilePath3 = string.Empty;
            CertFilePath3 = string.Empty;
            AuthPin = string.Empty;
            CurrentPin = string.Empty;
            NewPin = string.Empty;
            ImportKeyParameter1 = null!;
            ImportKeyParameter2 = null!;
            ImportKeyParameter3 = null!;
        }

        public override string ToString()
        {
            string command = string.Format("Command:{0} CommandTitle:{1}", Command, CommandTitle);
            string pkeyCertParam = string.Format("PkeyFilePath1:{0} CertFilePath1:{1} PkeyFilePath2:{2} CertFilePath2:{3} PkeyFilePath3:{4} CertFilePath3:{5} AuthPin:{6}",
                PkeyFilePath1, CertFilePath1, PkeyFilePath2, CertFilePath2, PkeyFilePath3, CertFilePath3, AuthPin);
            string pinCommandParam = string.Format("CurrentPin:{0} NewPin:{1}", CurrentPin, NewPin);
            return string.Format("{0}\n{1}\n{2}", command, pkeyCertParam, pinCommandParam);
        }
    }

    public class PIVProcess
    {
        // 処理実行のためのプロパティー
        private PIVParameter Parameter = null!;

        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnNotifyProcessTerminated(PIVParameter parameter);
        private HandlerOnNotifyProcessTerminated OnNotifyProcessTerminated = null!;

        //
        // PIV機能設定用関数
        // 
        public void DoPIVProcess(PIVParameter parameter, HandlerOnNotifyProcessTerminated handlerRef)
        {
            // 画面から引き渡されたパラメーターを退避
            Parameter = parameter;

            // コールバックを保持
            OnNotifyProcessTerminated = handlerRef;

            // 処理開始を通知
            NotifyProcessStarted();

            // コマンドに応じ、以下の処理に分岐
            switch (Parameter.Command) {
            case Command.COMMAND_CCID_PIV_IMPORT_KEY:
                DoRequestPIVImportKey();
                break;
            default:
                break;
            }
        }

        //
        // 鍵・証明書インポート処理
        //
        private void DoRequestPIVImportKey()
        {
            new PIVImportKeyProcess().DoProcess(Parameter, DoResponsePIVImportKey);
        }

        private void DoResponsePIVImportKey(bool success, string errorMessage)
        {
            // 画面に制御を戻す
            NotifyProcessTerminated(success, errorMessage);
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
    }
}
