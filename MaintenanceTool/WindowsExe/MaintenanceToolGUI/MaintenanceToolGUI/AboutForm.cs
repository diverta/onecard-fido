using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class AboutForm : Form
    {
        public AboutForm()
        {
            InitializeComponent();
        }

        public void SetTitleAndVersionText(String toolName, String toolVersion)
        {
            // ツールタイトル表示
            Text = String.Format("{0}について", toolName);
            label1.Text = toolName;

            // バージョン表示
            labelVersion.Text = toolVersion;
        }
    }
}
