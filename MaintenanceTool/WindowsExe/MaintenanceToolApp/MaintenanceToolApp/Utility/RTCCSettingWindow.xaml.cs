using System.Windows;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.Utility
{
    /// <summary>
    /// RTCCSettingWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class RTCCSettingWindow : Window
    {
        public RTCCSettingWindow()
        {
            // 画面項目の初期化
            InitializeComponent();
            InitFieldValue();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // ユーティリティー画面を、オーナー画面の中央にモード付きで表示
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
        // 画面初期化処理
        //
        private void InitFieldValue()
        {
            // ラジオボタン「USB経由」を選択状態にする
            buttonTransportUSB.IsChecked = true;

            // 時刻表示ラベルをブランクにする
            LabelToolTimestamp.Content = string.Empty;
            LabelDeviceTimestamp.Content = string.Empty;
        }

        private void DoSetParameter(Transport transport)
        {
            // トランスポート種別を設定
        }

        //
        // 時刻設定処理
        //
        private void DoSetTimestamp()
        {
            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (DialogUtil.DisplayPromptPopup(this, AppCommon.MSG_PROMPT_RTCC_SET_TIMESTAMP, AppCommon.MSG_COMMENT_RTCC_SET_TIMESTAMP) == false) {
                return;
            }
        }

        //
        // 時刻参照処理
        //
        private void DoGetTimestamp()
        {
        }

        //
        // イベント処理部
        // 
        private void buttonTransportUSB_Checked(object sender, RoutedEventArgs e)
        {
            DoSetParameter(Transport.TRANSPORT_HID);
        }

        private void buttonTransportBLE_Checked(object sender, RoutedEventArgs e)
        {
            DoSetParameter(Transport.TRANSPORT_BLE);
        }

        private void buttonGetTimestamp_Click(object sender, RoutedEventArgs e)
        {
            DoGetTimestamp();
        }

        private void buttonSetTimestamp_Click(object sender, RoutedEventArgs e)
        {
            DoSetTimestamp();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }
    }
}
