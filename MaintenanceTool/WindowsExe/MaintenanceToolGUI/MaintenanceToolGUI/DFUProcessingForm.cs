using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class DFUProcessingForm : Form
    {
        // DFU処理クラスの参照を保持
        private ToolDFU ToolDFURef;

        public DFUProcessingForm(ToolDFU td)
        {
            InitializeComponent();
            ToolDFURef = td;
        }
    }
}
