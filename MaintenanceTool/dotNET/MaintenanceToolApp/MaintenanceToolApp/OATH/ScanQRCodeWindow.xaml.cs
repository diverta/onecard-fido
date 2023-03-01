using MaintenanceToolApp;
using MaintenanceToolApp.CommonWindow;
using System;
using System.ComponentModel;
using System.Threading.Tasks;
using System.Windows;

namespace MaintenanceTool.OATH
{
    /// <summary>
    /// ScanQRCodeWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class ScanQRCodeWindow : Window
    {
        // パラメーターの参照を保持
        private readonly OATHParameter Parameter;

        public ScanQRCodeWindow(OATHParameter parameter)
        {
            // パラメーターの参照を保持
            Parameter = parameter;

            // 画面項目の初期化
            InitializeComponent();
            InitFieldValue();
        }

        public void ShowDialogWithOwner(Window ownerWindow)
        {
            // この画面を、親画面の中央にモード付きで表示
            Owner = ownerWindow;
            ownerWindow.Hide();
            ShowDialog();
        }

        public void DoScan()
        {
            // 画面項目の初期化
            InitFieldValue();

            // QRコードのスキャンを画面スレッドで実行
            if (OATHProcess.ScanQRCode(Parameter) == false) {
                DialogUtil.ShowWarningMessage(this, Title, Parameter.ResultInformativeMessage);
                return;
            }

            // パラメーターを設定し、コマンドを実行
            Parameter.CommandTitle = Title;
            Task task = Task.Run(() => {
                new OATHProcess(Parameter).DoProcess(OnOATHProcessTerminated);
            });

            // 進捗画面を表示
            CommonProcessingWindow.OpenForm(this);

            if (Parameter.CommandSuccess == false) {
                // 処理失敗時は、エラーメッセージをポップアップ表示
                DialogUtil.ShowWarningMessage(this, Parameter.ResultMessage, Parameter.ResultInformativeMessage);
                return;
            }

            // アカウント情報の各項目を画面表示
            labelAccountVal.Content = Parameter.OATHAccountName;
            labelIssuerVal.Content = Parameter.OATHAccountIssuer;
            labelPassword.Content = Parameter.OATHTOTPValue;

            // 処理完了メッセージをポップアップ表示
            DialogUtil.ShowInfoMessage(this, Title, Parameter.ResultMessage);
        }

        private void OnOATHProcessTerminated(OATHParameter parameter)
        {
            Application.Current.Dispatcher.Invoke(new Action(() => {
                // 進捗画面を閉じる
                CommonProcessingWindow.NotifyTerminate();
            }));
        }

        //
        // 画面初期化処理
        //
        private void InitFieldValue()
        {
            // ワンタイムパスワードの更新ボタンを使用不可とする
            buttonUpdate.IsEnabled = false;

            // 画面表示項目を初期化
            labelAccountVal.Content = string.Empty;
            labelIssuerVal.Content = string.Empty;
            labelPassword.Content = string.Empty;
        }

        //
        // イベント処理部
        // 
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            // この画面を閉じる
            Close();
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            // 親画面を表示
            Owner.Show();
        }

        private void buttonScan_Click(object sender, RoutedEventArgs e)
        {
            DoScan();
        }
    }
}
