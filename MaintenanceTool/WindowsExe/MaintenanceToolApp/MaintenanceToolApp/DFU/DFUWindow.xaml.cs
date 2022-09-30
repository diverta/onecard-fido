using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp.DFU
{
    /// <summary>
    /// DFUWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class DFUWindow : Window
    {
        // 処理実行のためのプロパティー
        private readonly DFUParameter Parameter;

        public DFUWindow(DFUParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // パラメーターのバージョン情報を画面表示
            labelCurrentVersion.Content = Parameter.CurrentVersionInfo.FWRev;
            labelUpdateVersion.Content = Parameter.UpdateImageData.UpdateVersion;

            // トランスポート種別に応じ、ラベル文言を変える
            if (Parameter.Transport == AppDefine.Transport.TRANSPORT_CDC_ACM) {
                labelCaption3.Content = AppCommon.MSG_DFU_PROMPT_START_USB_DFU;
            } else if (Parameter.Transport == AppDefine.Transport.TRANSPORT_BLE) {
                labelCaption3.Content = AppCommon.MSG_DFU_PROMPT_START_BLE_DFU;
            } else {
                labelCaption3.Content = string.Empty;
            }

            // この画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            bool? b = ShowDialog();
            if (b == null) {
                return false;
            } else {
                return (bool)b;
            }
        }

        private void TerminateWindow(bool dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }

        //
        // イベント処理部
        // 
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }

        private void buttonOK_Click(object sender, RoutedEventArgs e)
        {
            // USB接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            TerminateWindow(true);
        }
    }
}
