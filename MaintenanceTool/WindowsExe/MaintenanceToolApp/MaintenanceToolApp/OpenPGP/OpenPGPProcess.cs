using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.OpenPGP
{
    public class OpenPGPParameter
    {
        public string CommandTitle { get; set; }
        public Command Command { get; set; }
        public bool CommandSuccess { get; set; }
        public string ResultMessage { get; set; }
        public string RealName { get; set; }
        public string MailAddress { get; set; }
        public string Comment { get; set; }
        public string Passphrase { get; set; }
        public string PubkeyFolderPath { get; set; }
        public string BackupFolderPath { get; set; }
        public string CurrentPin { get; set; }
        public string NewPin { get; set; }

        public OpenPGPParameter()
        {
            CommandTitle = string.Empty;
            Command = Command.COMMAND_NONE;
            ResultMessage = string.Empty;
            RealName = string.Empty;
            MailAddress = string.Empty;
            Comment = string.Empty;
            Passphrase = string.Empty;
            PubkeyFolderPath = string.Empty;
            BackupFolderPath = string.Empty;
            CurrentPin = string.Empty;
            NewPin = string.Empty;
        }

        public override string ToString()
        {
            string command = string.Format("Command:{0} CommandTitle:{1}", Command, CommandTitle);
            string PGPKeyParam = string.Format("RealName:{0} MailAddress:{1} Comment:{2} Passphrase:{3} PubkeyFolderPath:{4} BackupFolderPath:{5}",
                RealName, MailAddress, Comment, Passphrase, PubkeyFolderPath, BackupFolderPath);
            string PinCommandParam = string.Format("CurrentPin:{0} NewPin:{1}", CurrentPin, NewPin);
            return string.Format("{0}\n{1}\n{2}", command, PGPKeyParam, PinCommandParam);
        }
    }

    public class OpenPGPProcess
    {
        // 処理実行のためのプロパティー
        private OpenPGPParameter Parameter = null!;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyProcessTerminated(OpenPGPParameter parameter);
        private event HandlerOnNotifyProcessTerminated OnNotifyProcessTerminated = null!;

        // イベントのコールバック参照
        private HandlerOnNotifyProcessTerminated OnNotifyProcessTerminatedRef = null!;

        //
        // OpenPGP機能設定用関数
        // 
        public void DoOpenPGPProcess(OpenPGPParameter parameter, HandlerOnNotifyProcessTerminated handlerRef)
        {
            // 画面から引き渡されたパラメーターを退避
            Parameter = parameter;

            // コールバックを登録
            OnNotifyProcessTerminatedRef = handlerRef;
            OnNotifyProcessTerminated += OnNotifyProcessTerminatedRef;

            // 処理開始を通知
            NotifyProcessStarted();

            // TODO: 仮の実装です。
            AppLogUtil.OutputLogDebug(Parameter.ToString());
            System.Threading.Thread.Sleep(1000);
            NotifyProcessTerminated(true);
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

        private void NotifyProcessTerminated(bool success)
        {
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

            // コールバックを解除
            OnNotifyProcessTerminated -= OnNotifyProcessTerminatedRef;
        }
    }
}
