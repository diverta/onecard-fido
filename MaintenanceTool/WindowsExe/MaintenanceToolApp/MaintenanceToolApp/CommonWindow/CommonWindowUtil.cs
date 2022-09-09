using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace MaintenanceToolApp.CommonWindow
{
    internal class CommonWindowUtil
    {
        //
        // 閉じるボタンの無効化
        //
        [DllImport("user32.dll")]
        private static extern int GetWindowLong(IntPtr hWnd, int nIndex);

        [DllImport("user32.dll")]
        private static extern int SetWindowLong(IntPtr hWnd, int nIndex, int dwNewLong);

        private const int GWL_STYLE = -16;
        private const int WS_SYSMENU = 0x80000;

        public static void DisableCloseWindowButton(Window window)
        {
            IntPtr handle = new WindowInteropHelper(window).Handle;
            int style = GetWindowLong(handle, GWL_STYLE);
            style = style & (~WS_SYSMENU);
            SetWindowLong(handle, GWL_STYLE, style);
        }
    }
}
