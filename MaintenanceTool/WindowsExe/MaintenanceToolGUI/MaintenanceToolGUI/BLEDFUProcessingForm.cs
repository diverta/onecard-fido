using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class BLEDFUProcessingForm : Form
    {
        // Cancelボタン押下イベントを定義
        public delegate void CanceledDFUByUserEvent();
        public event CanceledDFUByUserEvent OnCanceledDFUByUser;

        public BLEDFUProcessingForm()
        {
            InitializeComponent();
        }

        public DialogResult OpenForm(IWin32Window owner)
        {
            // 処理進捗画面を開く
            return ShowDialog(owner);
        }

        private void ButtonCancel_Click(object sender, System.EventArgs e)
        {
            // Cancelボタンを使用不可とする
            ButtonCancel.Enabled = false;

            // Cancelボタンがクリックされた旨をDFU処理クラスに通知
            OnCanceledDFUByUser();
        }

        public void NotifyStartDFUProcess(int maximum)
        {
            // プログレスバーをリセット
            InitFieldValue();
            LevelIndicator.Maximum = maximum;

        }

        public void NotifyDFUProcess(string message, int progressValue)
        {
            // 進捗表示を更新
            LabelProgress.Text = message;
            LevelIndicator.Value = progressValue;
        }

        public void NotifyCancelable(bool cancelable)
        {
            // 転送処理中の場合は、Cancelボタンを押下可能とする
            ButtonCancel.Enabled = cancelable;
        }

        public void NotifyCancelDFUProcess()
        {
            // DFU処理がキャンセルされた場合はCancelを戻す
            DialogResult = DialogResult.Cancel;
            TerminateWindow();
        }

        public void NotifyTerminateDFUProcess(bool success)
        {
            // DFU処理が正常終了した場合はOK、異常終了した場合はAbortを戻す
            if (success) {
                DialogResult = DialogResult.OK;
            } else {
                DialogResult = DialogResult.Abort;
            }
            TerminateWindow();
        }

        private void InitFieldValue()
        {
            // テキストをブランクに設定
            Text = ToolGUICommon.MSG_DFU_PROCESS_TITLE_GOING;
            LabelProgress.Text = "";
            LevelIndicator.Value = 0;

            // Cancelボタンを使用不可とする
            ButtonCancel.Enabled = false;
        }

        private void TerminateWindow()
        {
            // 処理進捗画面を閉じる
            Close();
        }
    }
}
