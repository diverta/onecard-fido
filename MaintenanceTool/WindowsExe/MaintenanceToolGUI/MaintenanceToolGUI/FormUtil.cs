using System.IO;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    class FormUtil
    {
        public static bool DisplayPromptPopup(string message)
        {
            DialogResult dialogResult = MessageBox.Show(
                message, MainForm.MaintenanceToolTitle,
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

        public static string createFilePath(
            SaveFileDialog dialog, string title, string fileName, string filter)
        {
            // ファイル保存ダイアログで生成されたパスを戻す
            dialog.FileName = fileName;
            dialog.Title = title;
            dialog.Filter = filter;
            dialog.FilterIndex = 0;
            dialog.RestoreDirectory = true;
            if (dialog.ShowDialog() != DialogResult.OK) {
                return string.Empty;
            }
            return dialog.FileName;
        }

        public static bool checkMustEntry(TextBox textBox, string informativeText)
        {
            if (textBox.Text.Equals(string.Empty))
            {
                MessageBox.Show(informativeText, MainForm.MaintenanceToolTitle,
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
                textBox.Focus();
                return false;
            }
            return true;
        }

        public static bool checkFileExist(TextBox textBox, string informativeText)
        {
            if (File.Exists(textBox.Text) == false)
            {
                MessageBox.Show(informativeText, MainForm.MaintenanceToolTitle,
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
                textBox.Focus();
                return false;
            }
            return true;
        }

        public static bool checkIsNumber(TextBox textBox, string informativeText)
        {
            if (Regex.IsMatch(textBox.Text, "^[1-9]{1}[0-9]*$") == false)
            {
                MessageBox.Show(informativeText, MainForm.MaintenanceToolTitle,
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
                textBox.Focus();
                return false;
            }
            return true;
        }

        public static bool checkIsNumeric(TextBox textBox, string informativeText)
        {
            if (Regex.IsMatch(textBox.Text, "^[0-9]*$") == false) {
                MessageBox.Show(informativeText, MainForm.MaintenanceToolTitle,
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
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
            MessageBox.Show(informativeText, MainForm.MaintenanceToolTitle,
                MessageBoxButtons.OK, MessageBoxIcon.Warning);
            destText.Focus();
            return false;
        }

        public static bool checkEntrySize(TextBox textBox, int minSize, int maxSize, string informativeText)
        {
            int size = textBox.Text.Length;
            if (size < minSize || size > maxSize) {
                MessageBox.Show(informativeText, MainForm.MaintenanceToolTitle,
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
                textBox.Focus();
                return false;
            }
            return true;
        }

        public static bool checkValueInRange(TextBox textBox, int minValue, int maxValue, string informativeText)
        {
            int value = int.Parse(textBox.Text);
            if (value < minValue || value > maxValue) {
                MessageBox.Show(informativeText, MainForm.MaintenanceToolTitle,
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
                textBox.Focus();
                return false;
            }
            return true;
        }

        public static bool checkValueWithPattern(TextBox textBox, string pattern, string informativeText)
        {
            if (Regex.IsMatch(textBox.Text, pattern) == false) {
                MessageBox.Show(informativeText, MainForm.MaintenanceToolTitle,
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
                textBox.Focus();
                return false;
            }
            return true;
        }
    }
}
