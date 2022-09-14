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
        // Cancelボタン押下イベントを定義
        public delegate void HandlerOnCanceledTransferByUser();
        private event HandlerOnCanceledTransferByUser OnCanceledTransferByUser = null!;

        public DFUProcessingWindow()
        {
            InitializeComponent();
        }

        public void RegisterHandlerOnCanceledTransferByUser(HandlerOnCanceledTransferByUser handler)
        {
            // Cancelボタン押下イベントを登録
            OnCanceledTransferByUser += handler;
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

        public void NotifyCancelable(bool cancelable)
        {
            // 転送処理中の場合は、Cancelボタンを押下可能とする
            buttonCancel.IsEnabled = cancelable;
        }

        public void NotifyCancelDFUProcess()
        {
            // DFU処理がキャンセルされた場合はCancelを戻す
            DialogResult = false;
            Close();
        }

        public void NotifyDFUProcessTerminated()
        {
            // 処理結果を戻す
            DialogResult = true;
            Close();
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

            // Cancelボタンを使用不可とする
            buttonCancel.IsEnabled = false;
        }

        //
        // イベント処理部
        // 
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            // Cancelボタンを使用不可とする
            buttonCancel.IsEnabled = false;

            // Cancelボタンがクリックされた旨をDFU処理クラスに通知
            OnCanceledTransferByUser();
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
