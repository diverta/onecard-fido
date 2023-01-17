using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using ToolAppCommon;

namespace MaintenanceToolApp
{
    public class USBDevice
    {
        public static void StartUSBDeviceNotification(Window window)
        {
            // Adds the windows message processing hook and registers USB device add/removal notification.
            HwndSource source = HwndSource.FromHwnd(new WindowInteropHelper(window).Handle);
            if (source != null) {
                IntPtr windowHandle = source.Handle;
                source.AddHook(HwndHandler);
                UsbNotification.RegisterUsbDeviceNotification(windowHandle);
            }
        }

        public static void TerminateUSBDeviceNotification()
        {
            UsbNotification.UnregisterUsbDeviceNotification();
        }

        private static IntPtr HwndHandler(IntPtr hwnd, int msg, IntPtr wparam, IntPtr lparam, ref bool handled)
        {
            if (msg == UsbNotification.WmDevicechange) {
                switch ((int)wparam) {
                case UsbNotification.DbtDeviceremovecomplete:
                    // USB接続を解放
                    HIDProcess.DisconnectHIDDevice();
                    break;

                case UsbNotification.DbtDevicearrival:
                    // USBデバイスの接続試行
                    HIDProcess.ConnectHIDDevice();
                    break;
                }
            }

            handled = false;
            return IntPtr.Zero;
        }
    }

    internal static class UsbNotification
    {
        public const int WmDevicechange = 0x0219;
        public const int DbtDevicearrival = 0x8000;
        public const int DbtDeviceremovecomplete = 0x8004;
        private const int DbtDevtypDeviceinterface = 5;

        // USB devices
        private static readonly Guid GuidDevinterfaceUSBDevice = new Guid("A5DCBF10-6530-11D2-901F-00C04FB951ED");
        private static IntPtr notificationHandle;

        public static void RegisterUsbDeviceNotification(IntPtr windowHandle)
        {
            //
            // Registers a window to receive notifications when USB devices are plugged or unplugged.
            //   windowHandle: Handle to the window receiving notifications.
            //
            DevBroadcastDeviceinterface dbi = new DevBroadcastDeviceinterface {
                DeviceType = DbtDevtypDeviceinterface,
                Reserved = 0,
                ClassGuid = GuidDevinterfaceUSBDevice,
                Name = 0
            };

            dbi.Size = Marshal.SizeOf(dbi);
            IntPtr buffer = Marshal.AllocHGlobal(dbi.Size);
            Marshal.StructureToPtr(dbi, buffer, true);

            notificationHandle = RegisterDeviceNotification(windowHandle, buffer, 0);
            if (notificationHandle == IntPtr.Zero) {
                AppLogUtil.OutputLogError(AppCommon.MSG_USB_DETECT_FAILED);
            } else {
                AppLogUtil.OutputLogInfo(AppCommon.MSG_USB_DETECT_STARTED);
            }
        }

        public static void UnregisterUsbDeviceNotification()
        {
            // Unregisters the window for USB device notifications
            if (notificationHandle != IntPtr.Zero) {
                UnregisterDeviceNotification(notificationHandle);
                AppLogUtil.OutputLogInfo(AppCommon.MSG_USB_DETECT_END);
            }
        }

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr RegisterDeviceNotification(IntPtr recipient, IntPtr notificationFilter, int flags);

        [DllImport("user32.dll")]
        private static extern bool UnregisterDeviceNotification(IntPtr handle);

        [StructLayout(LayoutKind.Sequential)]
        private struct DevBroadcastDeviceinterface
        {
            internal int Size;
            internal int DeviceType;
            internal int Reserved;
            internal Guid ClassGuid;
            internal short Name;
        }
    }
}
