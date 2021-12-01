using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class BLEDFUProcessingForm : Form
    {
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
            DialogResult = DialogResult.Cancel;
            TerminateWindow();
        }

        private void TerminateWindow()
        {
            // 処理進捗画面を閉じる
            Close();
        }
    }
}
