using MaintenanceToolApp;
using System.Windows;

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
}
