﻿using System.Reflection;
using System.Security.Principal;
using System.Threading;
using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        // Mutex作成
        private readonly Mutex MutexRef = new Mutex(false, Assembly.GetExecutingAssembly().GetName().Name);

        protected override void OnStartup(StartupEventArgs e)
        {
            // 管理者として実行されていない場合は
            // FIDO認証器と通信ができないため、
            // プログラムを起動させない
            if (CheckAdministratorRoll() == false) {
                MessageBox.Show(AppCommon.MSG_INVALID_USER_ROLL, AppCommon.MSG_TOOL_TITLE);
                Shutdown();
            }

            // Mutexの所有権を要求
            if (MutexRef.WaitOne(0, false) == false) {
                MessageBox.Show(AppCommon.MSG_ERROR_DOUBLE_START, AppCommon.MSG_TOOL_TITLE);
                MutexRef.Close();
                Shutdown();
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

        protected override void OnExit(ExitEventArgs e)
        {
            // USB接続を解放
            HIDProcess.DisconnectHIDDevice();

            // USBデバイスの脱着検知を終了
            USBDevice.TerminateUSBDeviceNotification();

            // アプリケーション終了ログを出力
            AppLogUtil.OutputLogInfo(string.Format("{0}を終了しました", AppCommon.MSG_TOOL_TITLE));

            // Mutexを解放
            if (MutexRef != null) {
                MutexRef.ReleaseMutex();
                MutexRef.Close();
            }
            base.OnExit(e);
        }
    }
}
