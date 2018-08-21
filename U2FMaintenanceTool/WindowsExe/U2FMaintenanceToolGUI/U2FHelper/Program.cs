using System;
using System.Windows.Forms;
using System.Threading;

namespace U2FHelper
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
            string mutexName = "U2FHelper.exe";
            bool createdNew;
            Mutex mutex = new Mutex(true, mutexName, out createdNew);
            if (createdNew == false) {
                MessageBox.Show("U2F Helperは既に起動されています.");
                mutex.Close();
                return;
            }

            try {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                new MainForm();
                Application.Run();
            } finally {
                mutex.ReleaseMutex();
                mutex.Close();
            }
        }
    }
}
