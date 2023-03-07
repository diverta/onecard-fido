using MaintenanceToolApp;
using System.ComponentModel;
using System.Windows;

namespace MaintenanceTool.OATH
{
    /// <summary>
    /// TOTPDisplayWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class TOTPDisplayWindow : Window
    {
        // パラメーターの参照を保持
        private readonly OATHParameter Parameter;

        public TOTPDisplayWindow(OATHParameter parameter)
        {
            // パラメーターの参照を保持
            Parameter = parameter;

            // 画面項目の初期化
            InitializeComponent();
        }

        public void ShowDialogWithOwner(Window ownerWindow)
        {
            // アカウント情報の各項目を画面に初期表示
            DisplayAccountInfo();

            // この画面を、親画面の中央にモード付きで表示
            Owner = ownerWindow;
            ownerWindow.Hide();
            ShowDialog();
        }

        private void DisplayAccountInfo()
        {
            // アカウント情報の各項目を画面表示
            labelIssuerVal.Content = Parameter.OATHAccountIssuer;
            labelAccountVal.Content = Parameter.OATHAccountName;
            labelPassword.Content = string.Format("{0:000000}", Parameter.OATHTOTPValue);
        }

        private void DoUpdate()
        {
            // ワンタイムパスワードを更新
            DoOATHProcess(AppCommon.MSG_LABEL_COMMAND_OATH_UPDATE_TOTP);
        }

        private void DoOATHProcess(string commandTitle)
        {
            // パラメーターを設定し、コマンドを実行
            Parameter.CommandTitle = commandTitle;
            if (OATHWindowUtil.DoOATHProcess(this, Parameter) == false) {
                return;
            }

            // アカウント情報の各項目を画面表示
            DisplayAccountInfo();
        }

        //
        // イベント処理部
        // 
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            // 親画面を表示
            Owner.Show();
        }

        private void buttonUpdate_Click(object sender, RoutedEventArgs e)
        {
            DoUpdate();
        }
    }
}
