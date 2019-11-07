using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class ToolPreferenceForm : Form
    {
        public ToolPreferenceForm()
        {
            InitializeComponent();
        }

        public void SetTitleAndVersionText(String toolName, String toolVersion)
        {
            // ツールタイトル表示
            labelToolName.Text = toolName;

            // バージョン表示
            labelVersion.Text = toolVersion;
        }
    }
}
