using System;
using System.Threading;
using System.Windows.Forms;

namespace U2FMaintenanceToolGUI
{
    static class Program
    {
        /// <summary>
        /// アプリケーションのメイン エントリ ポイントです。
        /// </summary>
        [STAThread]
        static void Main()
        {
            // Mutexを生成し、プログラムの多重起動を抑止
            string mutexName = "U2FMaintenanceToolGUI.exe";
            bool createdNew;
            Mutex mutex = new Mutex(true, mutexName, out createdNew);
            if (createdNew == false) {
                MessageBox.Show("U2F管理ツールは既に起動されています.");
                mutex.Close();
                return;
            }

            try {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                Application.Run(new MainForm());
            } finally {
                mutex.ReleaseMutex();
                mutex.Close();
            }
        }
    }
}
