using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    class FormUtil
    {
        public static void ShowErrorMessage(string captionText, string messageText)
        {
            MessageBox.Show(messageText, captionText, MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        public static void ShowWarningMessage(string captionText, string messageText)
        {
            MessageBox.Show(messageText, captionText, MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }

        public static void ShowErrorMessage(IWin32Window owner, string captionText, string messageText)
        {
            MessageBox.Show(owner, messageText, captionText, MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        public static void ShowWarningMessage(IWin32Window owner, string captionText, string messageText)
        {
            MessageBox.Show(owner, messageText, captionText, MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }

        public static void ShowInfoMessage(IWin32Window owner, string captionText, string messageText)
        {
            MessageBox.Show(owner, messageText, captionText, MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        public static bool DisplayPromptPopup(IWin32Window owner, string message)
        {
            DialogResult dialogResult = MessageBox.Show(owner,
                message, MainForm.MaintenanceToolTitle,
                MessageBoxButtons.YesNo, MessageBoxIcon.Question);

            // Yesがクリックされた場合 true を戻す
            return (dialogResult == DialogResult.Yes);
        }

        public static bool DisplayPromptPopup(IWin32Window owner, string title, string message)
        {
            DialogResult dialogResult = MessageBox.Show(owner,
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
                ShowWarningMessage(MainForm.MaintenanceToolTitle, informativeText);
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
            ShowWarningMessage(MainForm.MaintenanceToolTitle, informativeText);
            destText.Focus();
            return false;
        }

        public static bool checkEntrySize(TextBox textBox, int minSize, int maxSize, string informativeText)
        {
            int size = textBox.Text.Length;
            if (size < minSize || size > maxSize) {
                ShowWarningMessage(MainForm.MaintenanceToolTitle, informativeText);
                textBox.Focus();
                return false;
            }
            return true;
        }

        public static bool checkValueInRange(TextBox textBox, int minValue, int maxValue, string informativeText)
        {
            int value = int.Parse(textBox.Text);
            if (value < minValue || value > maxValue) {
                ShowWarningMessage(MainForm.MaintenanceToolTitle, informativeText);
                textBox.Focus();
                return false;
            }
            return true;
        }

        public static bool checkValueWithPattern(TextBox textBox, string pattern, string informativeText)
        {
            if (Regex.IsMatch(textBox.Text, pattern) == false) {
                ShowWarningMessage(MainForm.MaintenanceToolTitle, informativeText);
                textBox.Focus();
                return false;
            }
            return true;
        }
    }
}
