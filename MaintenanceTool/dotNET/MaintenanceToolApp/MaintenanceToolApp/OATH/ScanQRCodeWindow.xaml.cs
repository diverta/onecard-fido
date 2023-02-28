using MaintenanceToolApp;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;

namespace MaintenanceTool.OATH
{
    /// <summary>
    /// ScanQRCodeWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class ScanQRCodeWindow : Window
    {
        public ScanQRCodeWindow()
        {
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
            // QRコードのスキャンを実行
            Dictionary<string, string> parsedQRCodeInfo = new Dictionary<string, string>();
            if (QRCodeUtil.ScanQRCodeFromScreenShot(parsedQRCodeInfo) == false) {
                DialogUtil.ShowWarningMessage(this, Title, AppCommon.MSG_ERROR_OATH_QRCODE_SCAN_FAILED);
                InitFieldValue();
                return;
            }

            // スキャンしたアカウント情報の項目有無をチェック
            if (QRCodeUtil.CheckScannedAccountInfo(parsedQRCodeInfo) == false) {
                DialogUtil.ShowWarningMessage(this, Title, AppCommon.MSG_ERROR_OATH_SCANNED_ACCOUNT_INFO_INVALID);
                InitFieldValue();
                return;
            }

            // アカウント情報の各項目を画面表示
            labelAccountVal.Content = parsedQRCodeInfo["account"];
            labelIssuerVal.Content = parsedQRCodeInfo["issuer"];

            // TODO: 仮の実装です。
            DialogUtil.ShowWarningMessage(this, Title, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
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
