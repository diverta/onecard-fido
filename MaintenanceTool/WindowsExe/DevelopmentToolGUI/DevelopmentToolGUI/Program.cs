using System;
using System.Security.Principal;
using System.Threading;
using System.Windows.Forms;

namespace DevelopmentToolGUI
{
    static class Program
    {
        /// <summary>
        /// アプリケーションのメイン エントリ ポイントです。
        /// </summary>
        [STAThread]
        static void Main()
        {
            // このプログラムの名称を取得
            string title = MainForm.GetMaintenanceToolTitle();

            // 管理者として実行されていない場合は
            // FIDO認証器と通信ができないため、
            // プログラムを起動させない
            if (CheckAdministratorRoll() == false) {
                // FormUtil.ShowErrorMessage(title, ToolGUICommon.MSG_INVALID_USER_ROLL);
                return;
            }

            // Mutexを生成し、プログラムの多重起動を抑止
            string mutexName = "DevelopmentToolGUI.exe";
            bool createdNew;
            Mutex mutex = new Mutex(true, mutexName, out createdNew);
            if (createdNew == false) {
                // FormUtil.ShowErrorMessage(title, ToolGUICommon.MSG_ERROR_DOUBLE_START);
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
        static bool CheckAdministratorRoll()
        {
            // このプログラムが管理者として
            // 実行されている場合はtrueを戻す
            WindowsIdentity wid = WindowsIdentity.GetCurrent();
            WindowsPrincipal wp = new WindowsPrincipal(wid);
            return wp.IsInRole(WindowsBuiltInRole.Administrator);
        }
    }
}
