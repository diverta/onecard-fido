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
    }
}
