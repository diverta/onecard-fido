using System.Windows.Forms;
using System.Timers;

namespace MaintenanceToolGUI
{
    public partial class DFUProcessingForm : Form
    {
        // 繰り返しタイマー
        private System.Timers.Timer TimerRef;

        public DFUProcessingForm()
        {
            InitializeComponent();

            // タイマーのセットアップ
            TimerRef = new System.Timers.Timer(1000);
            TimerRef.Elapsed += TimerElapsed;
        }

        private void TimerElapsed(object sender, ElapsedEventArgs e)
        {
            // プログレスバーを右側に伸ばす
            LevelIndicator.Value++;
            if (LevelIndicator.Value == LevelIndicator.Maximum) {
                TimerRef.Stop();
            }
        }

        public bool OpenForm(IWin32Window owner)
        {
            // プログレスバーをリセット
            LevelIndicator.Value = 0;

            // 処理進捗画面を開く
            DialogResult = DialogResult.Cancel;
            return (ShowDialog(owner) == DialogResult.OK);
        }

        private void TerminateWindow()
        {
            // 処理進捗画面を閉じる
            Close();
        }

        public void NotifyStartDFUProcess()
        {
            // プログレスバーの進捗カウントアップを開始
            TimerRef.Start();
        }

        public void NotifyDFUProcess(string message)
        {
            // 進捗表示ラベルを更新
            LabelProgress.Text = message;
        }

        public void NotifyTerminateDFUProcess(bool success)
        {
            // タイマーを停止
            TimerRef.Stop();

            // DFU処理が正常終了した場合はOK、異常終了した場合はAbortを戻す
            if (success) {
                DialogResult = DialogResult.OK;
            } else {
                DialogResult = DialogResult.Abort;
            }
            TerminateWindow();
        }
    }
}
