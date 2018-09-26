using System.IO;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace U2FMaintenanceToolGUI
{
    class FormUtil
    {
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
                MessageBox.Show(informativeText, ToolGUICommon.MSG_INVALID_FIELD,
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
                MessageBox.Show(informativeText, ToolGUICommon.MSG_INVALID_FILE_PATH,
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
                MessageBox.Show(informativeText, ToolGUICommon.MSG_INVALID_NUMBER,
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
                textBox.Focus();
                return false;
            }
            return true;
        }
    }
}
