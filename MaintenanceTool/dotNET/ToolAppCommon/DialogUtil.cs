﻿using Microsoft.Win32;
using Microsoft.WindowsAPICodePack.Dialogs;
using System;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;

namespace MaintenanceToolApp
{
    public class DialogUtil
    {
        private delegate IntPtr HookProc(int nCode, IntPtr wParam, IntPtr lParam);
        private delegate void TimerProc(IntPtr hWnd, uint uMsg, UIntPtr nIDEvent, uint dwTime);

        private const int WH_CALLWNDPROCRET = 12;
        private enum CbtHookAction : int
        {
            HCBT_MOVESIZE = 0,
            HCBT_MINMAX = 1,
            HCBT_QS = 2,
            HCBT_CREATEWND = 3,
            HCBT_DESTROYWND = 4,
            HCBT_ACTIVATE = 5,
            HCBT_CLICKSKIPPED = 6,
            HCBT_KEYSKIPPED = 7,
            HCBT_SYSCOMMAND = 8,
            HCBT_SETFOCUS = 9
        }

        [DllImport("user32.dll")]
        private static extern bool GetWindowRect(IntPtr hWnd, ref Rectangle lpRect);

        [DllImport("user32.dll")]
        private static extern int MoveWindow(IntPtr hWnd, int X, int Y, int nWidth, int nHeight, bool bRepaint);

        [DllImport("user32.dll")]
        private static extern IntPtr SetWindowsHookEx(int idHook, HookProc lpfn, IntPtr hInstance, int threadId);

        [DllImport("user32.dll")]
        private static extern int UnhookWindowsHookEx(IntPtr idHook);

        [DllImport("user32.dll")]
        private static extern IntPtr CallNextHookEx(IntPtr idHook, int nCode, IntPtr wParam, IntPtr lParam);

        [DllImport("kernel32.dll")]
        private static extern IntPtr GetCurrentThreadId();

        [StructLayout(LayoutKind.Sequential)]
        private struct CWPRETSTRUCT
        {
            public IntPtr lResult;
            public IntPtr lParam;
            public IntPtr wParam;
            public uint message;
            public IntPtr hwnd;
        };

        private static IntPtr _owner;
        private static HookProc _hookProc;
        private static IntPtr _hHook;

        static DialogUtil()
        {
            _hookProc = new HookProc(MessageBoxHookProc);
            _hHook = IntPtr.Zero;
        }

        private static void Initialize()
        {
            if (_hHook != IntPtr.Zero) {
                throw new NotSupportedException("multiple calls are not supported");
            }

            _hHook = SetWindowsHookEx(WH_CALLWNDPROCRET, _hookProc, IntPtr.Zero, (int)GetCurrentThreadId());
        }

        private static IntPtr MessageBoxHookProc(int nCode, IntPtr wParam, IntPtr lParam)
        {
            if (nCode < 0) {
                return CallNextHookEx(_hHook, nCode, wParam, lParam);
            }

            object? objectRef = Marshal.PtrToStructure(lParam, typeof(CWPRETSTRUCT));
            if (objectRef != null) {
                CWPRETSTRUCT msg = (CWPRETSTRUCT)objectRef;
                if (msg.message == (int)CbtHookAction.HCBT_ACTIVATE) {
                    try {
                        CenterWindow(msg.hwnd);
                    } finally {
                        UnhookWindowsHookEx(_hHook);
                        _hHook = IntPtr.Zero;
                    }
                }
            }

            return CallNextHookEx(_hHook, nCode, wParam, lParam);
        }

        private static void CenterWindow(IntPtr hChildWnd)
        {
            Rectangle recChild = new Rectangle(0, 0, 0, 0);
            GetWindowRect(hChildWnd, ref recChild);

            int width = recChild.Width - recChild.X;
            int height = recChild.Height - recChild.Y;

            Rectangle recParent = new Rectangle(0, 0, 0, 0);
            GetWindowRect(_owner, ref recParent);

            System.Drawing.Point ptCenter = new System.Drawing.Point(0, 0);
            ptCenter.X = recParent.X + ((recParent.Width - recParent.X) / 2);
            ptCenter.Y = recParent.Y + ((recParent.Height - recParent.Y) / 2);

            System.Drawing.Point ptStart = new System.Drawing.Point(0, 0);
            ptStart.X = (ptCenter.X - (width / 2));
            ptStart.Y = (ptCenter.Y - (height / 2));

            ptStart.X = (ptStart.X < 0) ? 0 : ptStart.X;
            ptStart.Y = (ptStart.Y < 0) ? 0 : ptStart.Y;

            MoveWindow(hChildWnd, ptStart.X, ptStart.Y, width, height, false);
        }

        private static MessageBoxResult Show(Window owner, string text, string caption, MessageBoxButton buttons, MessageBoxImage icon)
        {
            _owner = new WindowInteropHelper(owner).Handle;
            Initialize();
            return MessageBox.Show(owner, text, caption, buttons, icon);
        }

        //
        // メッセージボックス関数群
        //
        public static void ShowErrorMessage(Window owner, string captionText, string messageText)
        {
            Show(owner, messageText, captionText, MessageBoxButton.OK, MessageBoxImage.Error);
        }

        public static void ShowWarningMessage(Window owner, string captionText, string messageText)
        {
            Show(owner, messageText, captionText, MessageBoxButton.OK, MessageBoxImage.Warning);
        }

        public static void ShowInfoMessage(Window owner, string captionText, string messageText)
        {
            Show(owner, messageText, captionText, MessageBoxButton.OK, MessageBoxImage.Information);
        }

        public static bool DisplayPromptPopup(Window owner, string title, string message)
        {
            MessageBoxResult dialogResult = Show(owner, message, title, MessageBoxButton.YesNo, MessageBoxImage.Question);

            // Yesがクリックされた場合 true を戻す
            return (dialogResult == MessageBoxResult.Yes);
        }
    }

    public class FileDialogUtil
    {
        //
        // ファイル選択ダイアログでフォルダーを選択
        //
        public static void SelectFolderPath(Window owner, string dialogTitle, TextBox textBox)
        {
            // ファイル選択ダイアログで選択されたパスを
            // 指定のテキストボックスにセット
            CommonOpenFileDialog dialog = new CommonOpenFileDialog();
            dialog.Title = dialogTitle;
            dialog.IsFolderPicker = true;
            dialog.InitialDirectory = textBox.Text;
            if (dialog.ShowDialog(owner) == CommonFileDialogResult.Ok) {
                if (dialog.FileName.Length > 0) {
                    textBox.Text = dialog.FileName;

                    // カーソルをテキストの末尾に移動
                    textBox.ScrollToHorizontalOffset(textBox.ViewportWidth);
                }
            }
        }

        //
        // ファイル選択ダイアログでファイルを選択
        //
        public static void SelectFilePath(Window owner, string dialogTitle, TextBox textBox, string filter)
        {
            // ファイル選択ダイアログで選択されたパスを
            // 指定のテキストボックスにセット
            var dialog = new OpenFileDialog();
            dialog.Title = dialogTitle;
            dialog.InitialDirectory = textBox.Text;
            if (filter.Length > 0) {
                dialog.Filter = filter;
            }
            if (dialog.ShowDialog(owner) == true) {
                if (dialog.FileName.Length > 0) {
                    textBox.Text = dialog.FileName;

                    // カーソルをテキストの末尾に移動
                    textBox.ScrollToHorizontalOffset(textBox.ViewportWidth);
                }
            }
        }
    }
}
