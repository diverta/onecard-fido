using System.Windows;

namespace MaintenanceToolApp.OpenPGP
{
    public class OpenPGPParameter
    {
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
            new OpenPGPWindow().ShowDialogWithOwner(ParentWindow);
        }
    }
}
