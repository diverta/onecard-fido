using MaintenanceToolApp;
using System.Threading;
using System.Windows;

namespace VendorMaintenanceTool
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        // Mutex作成
        private readonly Mutex MutexRef = new Mutex(false, "MaintenanceToolApp");

        protected override void OnStartup(StartupEventArgs e)
        {
            // Mutexの所有権を要求
            if (MutexRef.WaitOne(0, false) == false) {
                MessageBox.Show(AppCommon.MSG_ERROR_DOUBLE_START, AppCommon.MSG_TOOL_TITLE);
                MutexRef.Close();
                Shutdown();
            }
        }

        protected override void OnExit(ExitEventArgs e)
        {
            // 業務終了時の処理
            CommandProcess.OnMainWindowTerminated();

            // Mutexを解放
            if (MutexRef != null) {
                MutexRef.ReleaseMutex();
                MutexRef.Close();
            }
            base.OnExit(e);
        }
    }
}
