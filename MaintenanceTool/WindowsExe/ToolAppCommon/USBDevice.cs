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

        private static IntPtr HwndHandler(IntPtr hwnd, int msg, IntPtr wparam, IntPtr lparam, ref bool handled)
        {
            if (msg == UsbNotification.WmDevicechange) {
                switch ((int)wparam) {
                case UsbNotification.DbtDeviceremovecomplete:
                    // TODO: 仮の実装です。
                    AppLogUtil.OutputLogInfo(String.Format("DbtDeviceremovecomplete 0x{0:x4}", (int)wparam));
                    break;

                case UsbNotification.DbtDevicearrival:
                    // TODO: 仮の実装です。
                    AppLogUtil.OutputLogInfo(String.Format("DbtDevicearrival 0x{0:x4}", (int)wparam));
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
        }

        public static void UnregisterUsbDeviceNotification()
        {
            // Unregisters the window for USB device notifications
            UnregisterDeviceNotification(notificationHandle);
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
