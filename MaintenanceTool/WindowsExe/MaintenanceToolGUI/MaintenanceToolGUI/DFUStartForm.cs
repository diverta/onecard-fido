using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class DFUStartForm : Form
    {
        // DFU処理クラスの参照を保持
        private ToolDFU ToolDFURef;

        public DFUStartForm()
        {
            InitializeComponent();
        }

        public void OpenForm(ToolDFU td)
        {
            ToolDFURef = td;
            LabelCurrentVersion.Text = ToolDFURef.CurrentVersion;
            LabelUpdateVersion.Text = ToolDFURef.UpdateVersion;
            ShowDialog();
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            // この画面を閉じる
            TerminateWindow(DialogResult.Cancel);
        }

        private void TerminateWindow(DialogResult dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }
    }
}
