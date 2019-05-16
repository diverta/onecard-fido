using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace U2FMaintenanceToolGUI
{
    public partial class AboutForm : Form
    {
        public AboutForm()
        {
            InitializeComponent();

            // バージョン表示
            labelVersion.Text = "Version 0.1.7";
        }
    }
}
