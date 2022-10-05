using System.Windows;

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

        public OpenPGPParameter()
        {
            RealName = string.Empty;
            MailAddress = string.Empty;
            Comment = string.Empty;
            Passphrase = string.Empty;
            PubkeyFolderPath = string.Empty;
            BackupFolderPath = string.Empty;
        }

        public override string ToString()
        {
            return string.Format("RealName:{0} MailAddress:{1} Comment:{2} Passphrase:{3} PubkeyFolderPath:{4} BackupFolderPath:{5}", 
                RealName, MailAddress, Comment, Passphrase, PubkeyFolderPath, BackupFolderPath);
        }
    }

    public class OpenPGPProcess
    {
        // 処理実行のためのプロパティー
        private readonly OpenPGPParameter Parameter;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow = App.Current.MainWindow;

        public OpenPGPProcess(OpenPGPParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
        }

        public void DoProcess()
        {
            // OpenPGP設定画面を開く
            new OpenPGPWindow(Parameter).ShowDialogWithOwner(ParentWindow);
        }
    }
}
