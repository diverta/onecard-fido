using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class CommonProcessingForm : Form
    {
        private static CommonProcessingForm Instance = new CommonProcessingForm();
        private bool isShown = false;

        private CommonProcessingForm()
        {
            InitializeComponent();
        }

        private DialogResult OpenFormInner(IWin32Window owner)
        {
            // 処理進捗画面を開く
            isShown = true;
            return ShowDialog(owner);
        }

        private void NotifyTerminateInner()
        {
            // 処理進捗画面を閉じる
            if (isShown) {
                isShown = false;
                Close();
            }
        }

        //
        // 公開用メソッド
        //
        public static DialogResult OpenForm(IWin32Window owner)
        {
            return Instance.OpenFormInner(owner);
        }

        public static void NotifyTerminate()
        {
            Instance.NotifyTerminateInner();
        }
    }
}
