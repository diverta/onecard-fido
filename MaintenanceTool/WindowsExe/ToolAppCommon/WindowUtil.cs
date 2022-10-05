using MaintenanceToolApp;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;

namespace ToolAppCommon
{
    public class WindowUtil
    {
        public static bool CheckUSBDeviceDisconnected(Window window)
        {
            if (HIDProcess.IsUSBDeviceDisconnected()) {
                DialogUtil.ShowWarningMessage(window, AppCommon.MSG_TOOL_TITLE, AppCommon.MSG_CMDTST_PROMPT_USB_PORT_SET);
                return true;
            }
            return false;
        }
    }

    public class TextBoxUtil
    {
        public static bool CheckEntrySize(TextBox textBox, int minSize, int maxSize, string title, string informativeText, Window parentWindow)
        {
            int size = textBox.Text.Length;
            if (size < minSize || size > maxSize) {
                DialogUtil.ShowWarningMessage(parentWindow, title, informativeText);
                textBox.Focus();
                return false;
            }
            return true;
        }

        public static bool CheckValueWithPattern(TextBox textBox, string pattern, string title, string informativeText, Window parentWindow)
        {
            if (Regex.IsMatch(textBox.Text, pattern) == false) {
                DialogUtil.ShowWarningMessage(parentWindow, title, informativeText);
                textBox.Focus();
                return false;
            }
            return true;
        }
    }

    public class PasswordBoxUtil
    {
        public static bool CheckEntrySize(PasswordBox passwordBox, int minSize, int maxSize, string title, string informativeText, Window parentWindow)
        {
            int size = passwordBox.Password.Length;
            if (size < minSize || size > maxSize) {
                DialogUtil.ShowWarningMessage(parentWindow, title, informativeText);
                passwordBox.Focus();
                return false;
            }
            return true;
        }

        public static bool CheckIsNumeric(PasswordBox passwordBox, string title, string informativeText, Window parentWindow)
        {
            if (Regex.IsMatch(passwordBox.Password, "^[0-9]*$") == false) {
                DialogUtil.ShowWarningMessage(parentWindow, title, informativeText);
                passwordBox.Focus();
                return false;
            }
            return true;
        }

        public static bool CompareEntry(PasswordBox passwordBoxDest, PasswordBox passwordBoxSrc, string title, string informativeText, Window parentWindow)
        {
            if (passwordBoxDest.Password.Equals(passwordBoxSrc.Password)) {
                return true;
            }
            DialogUtil.ShowWarningMessage(parentWindow, title, informativeText);
            passwordBoxDest.Focus();
            return false;
        }
    }
}
