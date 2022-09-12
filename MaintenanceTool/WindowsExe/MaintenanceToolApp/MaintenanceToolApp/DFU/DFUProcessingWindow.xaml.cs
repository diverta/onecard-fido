using MaintenanceToolApp.CommonWindow;
using System;
using System.Windows;

namespace MaintenanceToolApp.DFU
{
    /// <summary>
    /// DFUProcessingWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class DFUProcessingWindow : Window
    {
        public DFUProcessingWindow()
        {
            InitializeComponent();
        }

        public bool OpenForm(Window ownerWindow)
        {
            // この画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            bool? b = ShowDialog();
            if (b == null) {
                return false;
            } else {
                return (bool)b;
            }
        }

        public void NotifyStartDFUProcess(int maximum)
        {
            // プログレスバーをリセット
            InitFieldValue();
            levelIndicator.Maximum = maximum;

        }

        public void NotifyDFUProcess(string message, int progressValue)
        {
            // 進捗表示を更新
            labelProgress.Content = message;
            levelIndicator.Value = progressValue;
        }

        public void NotifyDFUProcessTerminated(bool success)
        {
            // TODO: 仮の実装です。
            Application.Current.Dispatcher.Invoke(new Action(() => {
                DialogResult = success;
                Close();
            }));
        }

        //
        // 内部処理
        //
        private void InitFieldValue()
        {
            // テキストをブランクに設定
            Title = AppCommon.MSG_DFU_PROCESS_TITLE_GOING;
            labelProgress.Content = "";
            levelIndicator.Value = 0;
        }

        //
        // イベント処理部
        // 
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            // TODO: 仮の実装です。
            DialogResult = false;
            Close();
        }

        //
        // 閉じるボタンの無効化
        //
        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);
            CommonWindowUtil.DisableCloseWindowButton(this);
        }
    }
}
