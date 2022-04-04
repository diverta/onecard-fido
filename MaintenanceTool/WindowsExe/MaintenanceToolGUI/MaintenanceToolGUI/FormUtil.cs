using System;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    class FormUtil
    {
        //
        // メッセージボックス中央表示対応のための処理群
        //
        private const int HCBT_ACTIVATE = 5;
        private const int GWL_HINSTANCE = -6;
        private const int WH_CBT = 5;
        private const uint SWP_NOSIZE = 0x0001;
        private const uint SWP_NOZORDER = 0x0004;
        private const uint SWP_NOACTIVATE = 0x0010;

        private delegate IntPtr HOOKPROC(int nCode, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll")]
        private static extern IntPtr GetWindowLong(IntPtr hWnd, int nIndex);

        [DllImport("kernel32.dll")]
        private static extern IntPtr GetCurrentThreadId();

        [DllImport("user32.dll")]
        private static extern IntPtr SetWindowsHookEx(int idHook, HOOKPROC lpfn, IntPtr hInstance, IntPtr threadId);

        [DllImport("user32.dll")]
        private static extern bool UnhookWindowsHookEx(IntPtr hHook);

        [DllImport("user32.dll")]
        private static extern IntPtr CallNextHookEx(IntPtr hHook, int nCode, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll")]
        private static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

        [DllImport("user32.dll")]
        private static extern bool SetWindowPos(IntPtr hWnd, int hWndInsertAfter, int x, int y, int cx, int cy, uint uFlags);

        private static IntPtr hOwner = (IntPtr)0;
        private static IntPtr hHook = (IntPtr)0;

        private static DialogResult MessageBoxShow(IWin32Window owner, string text, string caption, MessageBoxButtons buttons, MessageBoxIcon icon)
        {
            // 呼出元ウィンドウのインスタンスハンドルを取得
            hOwner = owner.Handle;
            IntPtr hInstance = GetWindowLong(owner.Handle, GWL_HINSTANCE);

            // スレッド識別子を取得
            IntPtr threadId = GetCurrentThreadId();

            // フック設定 --> HookProcが呼び出される
            hHook = SetWindowsHookEx(WH_CBT, new HOOKPROC(HookProc), hInstance, threadId);

            // メッセージボックスを表示
            return MessageBox.Show(owner, text, caption, buttons, icon);
        }

        private static IntPtr HookProc(int nCode, IntPtr wParam, IntPtr lParam)
        {
            // システムがウィンドウをアクティブ化しようとした場合
            if (nCode == HCBT_ACTIVATE) {
                // オーナーウィンドウの領域を取得
                RECT rcOwner = GetWindowRect(hOwner);

                // メッセージボックスの領域を取得
                RECT rcMsgBox = GetWindowRect(wParam);

                // メッセージボックスをオーナーウィンドウの中央位置に移動
                int x = rcOwner.Left + (rcOwner.Width - rcMsgBox.Width) / 2;
                int y = rcOwner.Top + (rcOwner.Height - rcMsgBox.Height) / 2;
                SetWindowPos(wParam, x, y);

                // フックを解除
                UnhookWindowsHookEx(hHook);
                hHook = (IntPtr)0;
            }

            // 次のフックを処理
            return CallNextHookEx(hHook, nCode, wParam, lParam);
        }

        private struct RECT
        {
            public int Left, Top, Right, Bottom;
            public int Width => Right - Left;
            public int Height => Bottom - Top;
        }

        private static RECT GetWindowRect(IntPtr hWnd)
        {
            RECT rc;
            GetWindowRect(hWnd, out rc);
            return rc;
        }

        private static bool SetWindowPos(IntPtr hWnd, int x, int y)
        {
            var flags = SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE;
            return SetWindowPos(hWnd, 0, x, y, 0, 0, flags);
        }

        //
        // メッセージボックス関数群
        //
        public static void ShowErrorMessage(string captionText, string messageText)
        {
            MessageBox.Show(messageText, captionText, MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        public static void ShowErrorMessage(IWin32Window owner, string captionText, string messageText)
        {
            MessageBoxShow(owner, messageText, captionText, MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        public static void ShowWarningMessage(IWin32Window owner, string captionText, string messageText)
        {
            MessageBoxShow(owner, messageText, captionText, MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }

        public static void ShowInfoMessage(IWin32Window owner, string captionText, string messageText)
        {
            MessageBoxShow(owner, messageText, captionText, MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        public static bool DisplayPromptPopup(IWin32Window owner, string message)
        {
            DialogResult dialogResult = MessageBoxShow(owner,
                message, MainForm.MaintenanceToolTitle,
                MessageBoxButtons.YesNo, MessageBoxIcon.Question);

            // Yesがクリックされた場合 true を戻す
            return (dialogResult == DialogResult.Yes);
        }

        public static bool DisplayPromptPopup(IWin32Window owner, string title, string message)
        {
            DialogResult dialogResult = MessageBoxShow(owner,
                message, title,
                MessageBoxButtons.YesNo, MessageBoxIcon.Question);

            // Yesがクリックされた場合 true を戻す
            return (dialogResult == DialogResult.Yes);
        }

        public static void selectFilePath(
            OpenFileDialog dialog, string title, string filter, TextBox textBox)
        {
            // ファイル選択ダイアログで選択されたパスを
            // 指定のテキストボックスにセット
            dialog.FileName = "";
            dialog.Title = title;
            dialog.Filter = filter;
            dialog.FilterIndex = 0;
            dialog.RestoreDirectory = true;
            if (dialog.ShowDialog() == DialogResult.OK) {
                textBox.Text = dialog.FileName;
            }
        }

        public static void SelectFolderPath(FolderBrowserDialog dialog, string dialogDescription, TextBox textBox)
        {
            // ファイル選択ダイアログで選択されたパスを
            // 指定のテキストボックスにセット
            dialog.Reset();
            if (textBox.Text.Length > 0) {
                dialog.SelectedPath = textBox.Text;
            }
            if (dialogDescription.Length > 0) {
                dialog.Description = dialogDescription;
            }
            dialog.ShowNewFolderButton = false;
            if (dialog.ShowDialog() == DialogResult.OK) {
                textBox.Text = dialog.SelectedPath;
            }
        }

        public static bool checkIsNumeric(TextBox textBox, string informativeText)
        {
            if (Regex.IsMatch(textBox.Text, "^[0-9]*$") == false) {
                ShowWarningMessage(textBox.Parent, MainForm.MaintenanceToolTitle, informativeText);
                textBox.Focus();
                return false;
            }
            return true;
        }

        public static bool compareEntry(TextBox destText, TextBox srcText, string informativeText)
        {
            if (destText.Text.Equals(srcText.Text)) {
                return true;
            }
            ShowWarningMessage(destText.Parent, MainForm.MaintenanceToolTitle, informativeText);
            destText.Focus();
            return false;
        }

        public static bool checkEntrySize(TextBox textBox, int minSize, int maxSize, string informativeText)
        {
            int size = textBox.Text.Length;
            if (size < minSize || size > maxSize) {
                ShowWarningMessage(textBox.Parent, MainForm.MaintenanceToolTitle, informativeText);
                textBox.Focus();
                return false;
            }
            return true;
        }

        public static bool checkValueInRange(TextBox textBox, int minValue, int maxValue, string informativeText)
        {
            int value = int.Parse(textBox.Text);
            if (value < minValue || value > maxValue) {
                ShowWarningMessage(textBox.Parent, MainForm.MaintenanceToolTitle, informativeText);
                textBox.Focus();
                return false;
            }
            return true;
        }

        public static bool checkValueWithPattern(TextBox textBox, string pattern, string informativeText)
        {
            if (Regex.IsMatch(textBox.Text, pattern) == false) {
                ShowWarningMessage(textBox.Parent, MainForm.MaintenanceToolTitle, informativeText);
                textBox.Focus();
                return false;
            }
            return true;
        }
    }
}
