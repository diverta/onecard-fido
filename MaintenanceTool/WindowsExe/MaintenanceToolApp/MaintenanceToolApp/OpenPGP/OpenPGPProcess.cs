using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.OpenPGP
{
    public class OpenPGPParameter
    {
        public string RealName { get; set; }
        public string MailAddress { get; set; }
        public string Comment { get; set; }
        public string Passphrase { get; set; }
        public string PubkeyFolderPath { get; set; }
        public string BackupFolderPath { get; set; }
        public Command SelectedPinCommand { get; set; }
        public string SelectedPinCommandName { get; set; }
        public string CurrentPin { get; set; }
        public string NewPin { get; set; }

        public OpenPGPParameter()
        {
            RealName = string.Empty;
            MailAddress = string.Empty;
            Comment = string.Empty;
            Passphrase = string.Empty;
            PubkeyFolderPath = string.Empty;
            BackupFolderPath = string.Empty;
            SelectedPinCommandName = string.Empty;
            CurrentPin = string.Empty;
            NewPin = string.Empty;
        }

        public override string ToString()
        {
            string PGPKeyParam = string.Format("RealName:{0} MailAddress:{1} Comment:{2} Passphrase:{3} PubkeyFolderPath:{4} BackupFolderPath:{5}",
                RealName, MailAddress, Comment, Passphrase, PubkeyFolderPath, BackupFolderPath);
            string PinCommandParam = string.Format("SelectedPinCommand:{0} SelectedPinCommandName:{1} CurrentPin:{2} NewPin:{3}",
                SelectedPinCommand, SelectedPinCommandName, CurrentPin, NewPin);
            return string.Format("{0}\n{1}", PGPKeyParam, PinCommandParam);
        }
    }

    public class OpenPGPProcess
    {
        // 処理実行のためのプロパティー
        private OpenPGPParameter Parameter = null!;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow = App.Current.MainWindow;

        //
        // OpenPGP機能設定用関数
        // 
        public void DoOpenPGPCommand(OpenPGPParameter parameter)
        {
            // 画面から引き渡されたパラメーターを退避
            Parameter = parameter;

            // TODO: 仮の実装です。
            AppLogUtil.OutputLogDebug(Parameter.ToString());
        }
    }
}
