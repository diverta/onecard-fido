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
        private static DFUProcessingWindow Instance = null!;

        private DFUProcessingWindow()
        {
            InitializeComponent();
        }

        //
        // 公開用メソッド
        //
        public static DFUProcessingWindow NewInstance()
        {
            Instance = new DFUProcessingWindow();
            return Instance;
        }

        public static DFUProcessingWindow GetInstance()
        {
            return Instance;
        }

        public bool OpenForm(Window ownerWindow)
        {
            // TODO: 仮の実装です。
            InitFieldValue();

            // この画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            bool? b = ShowDialog();
            if (b == null) {
                return false;
            } else {
                return (bool)b;
            }
        }

        //
        // 内部処理
        //
        private void InitFieldValue()
        {
            // テキストをブランクに設定
            Title = AppCommon.MSG_DFU_PROCESS_TITLE_GOING;
            labelProgress.Content = "進捗を表示します。";
            levelIndicator.Value = 50;
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
