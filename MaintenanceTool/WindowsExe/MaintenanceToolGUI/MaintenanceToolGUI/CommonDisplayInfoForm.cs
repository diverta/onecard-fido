using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class CommonDisplayInfoForm : Form
    {
        private static CommonDisplayInfoForm Instance = new CommonDisplayInfoForm();

        private CommonDisplayInfoForm()
        {
            InitializeComponent();
        }

        private DialogResult OpenFormInner(IWin32Window owner, string title, string info)
        {
            // 情報表示画面を開く
            Text = title;
            textInfo.Text = info;
            return ShowDialog(owner);
        }

        private void buttonOK_Click(object sender, System.EventArgs e)
        {
            Close();
        }

        //
        // 公開用メソッド
        //
        public static DialogResult OpenForm(IWin32Window owner, string title, string info)
        {
            return Instance.OpenFormInner(owner, title, info);
        }
    }
}
