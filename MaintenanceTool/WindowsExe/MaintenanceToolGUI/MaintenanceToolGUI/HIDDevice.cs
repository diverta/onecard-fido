using Microsoft.Win32.SafeHandles;
using System;
using System.IO;
using System.Runtime.InteropServices;

namespace MaintenanceToolGUI
{
    internal class HIDDevice
    {
        private const int DIGCF_DEFAULT = 0x1;
        private const int DIGCF_PRESENT = 0x2;
        private const int DIGCF_ALLCLASSES = 0x4;
        private const int DIGCF_PROFILE = 0x8;
        private const int DIGCF_DEVICEINTERFACE = 0x10;

        private const short FILE_ATTRIBUTE_NORMAL = 0x80;
        private const short INVALID_HANDLE_VALUE = -1;
        private const uint GENERIC_READ = 0x80000000;
        private const uint GENERIC_WRITE = 0x40000000;
        private const uint FILE_SHARE_READ = 0x00000001;
        private const uint FILE_SHARE_WRITE = 0x00000002;
        private const uint CREATE_NEW = 1;
        private const uint CREATE_ALWAYS = 2;
        private const uint OPEN_EXISTING = 3;

        private const string SETUPAPI_DLL = "setupapi.dll";
        private const string HID_DLL = "hid.dll";
        private const string KERNEL32_DLL = "kernel32.dll";

        [DllImport(SETUPAPI_DLL, CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr SetupDiGetClassDevs(
            ref Guid ClassGuid,
            IntPtr Enumerator,
            IntPtr hwndParent,
            uint Flags);

        [DllImport(HID_DLL, SetLastError = true)]
        private static extern void HidD_GetHidGuid(ref Guid hidGuid);

        [DllImport(SETUPAPI_DLL, CharSet = CharSet.Auto, SetLastError = true)]
        private static extern Boolean SetupDiEnumDeviceInterfaces(
            IntPtr hDevInfo,
            IntPtr devInfo,
            ref Guid interfaceClassGuid,
            UInt32 memberIndex,
            ref SP_DEVICE_INTERFACE_DATA deviceInterfaceData
        );

        [DllImport(SETUPAPI_DLL, CharSet = CharSet.Auto, SetLastError = true)]
        private static extern Boolean SetupDiGetDeviceInterfaceDetail(
            IntPtr hDevInfo,
            ref SP_DEVICE_INTERFACE_DATA deviceInterfaceData,
            ref SP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData,
            UInt32 deviceInterfaceDetailDataSize,
            out UInt32 requiredSize,
            ref SP_DEVINFO_DATA deviceInfoData
        );

        [DllImport(SETUPAPI_DLL, CharSet = CharSet.Auto, SetLastError = true)]
        private static extern bool SetupDiDestroyDeviceInfoList(IntPtr DeviceInfoSet);

        [DllImport(KERNEL32_DLL, SetLastError = true)]
        private static extern SafeFileHandle CreateFile(string lpFileName, uint dwDesiredAccess,
            uint dwShareMode, IntPtr lpSecurityAttributes, uint dwCreationDisposition,
            uint dwFlagsAndAttributes, IntPtr hTemplateFile);

        [DllImport(KERNEL32_DLL, SetLastError = true)]
        private static extern bool ReadFile(SafeFileHandle hFile, byte[] lpBuffer,
            uint nNumberOfBytesToRead, ref uint lpNumberOfBytesRead, IntPtr lpOverlapped);

        [DllImport(KERNEL32_DLL, SetLastError = true)]
        private static extern bool WriteFile(SafeFileHandle hFile, byte[] lpBuffer,
            uint nNumberOfBytesToWrite, ref uint lpNumberOfBytesWritten, IntPtr lpOverlapped);

        [DllImport(HID_DLL, SetLastError = true)]
        private static extern bool HidD_GetPreparsedData(
            SafeFileHandle hObject,
            ref IntPtr PreparsedData);

        [DllImport(HID_DLL, SetLastError = true)]
        private static extern Boolean HidD_FreePreparsedData(ref IntPtr PreparsedData);

        [DllImport(HID_DLL, SetLastError = true)]
        private static extern int HidP_GetCaps(
            IntPtr pPHIDP_PREPARSED_DATA,
            ref HIDP_CAPS myPHIDP_CAPS);

        [DllImport(HID_DLL, SetLastError = true)]
        private static extern Boolean HidD_GetAttributes(SafeFileHandle hObject, ref HIDD_ATTRIBUTES Attributes);

        [DllImport(HID_DLL, SetLastError = true, CallingConvention = CallingConvention.StdCall)]
        private static extern bool HidD_GetProductString(
            SafeFileHandle hDevice,
            IntPtr Buffer,
            uint BufferLength);

        [DllImport(HID_DLL, SetLastError = true, CallingConvention = CallingConvention.StdCall)]
        private static extern bool HidD_GetSerialNumberString(
            SafeFileHandle hDevice,
            IntPtr Buffer,
            uint BufferLength);

        [DllImport(HID_DLL, SetLastError = true, CallingConvention = CallingConvention.StdCall)]
        private static extern Boolean HidD_GetManufacturerString(
            SafeFileHandle hDevice,
            IntPtr Buffer,
            uint BufferLength);

        public struct InterfaceDetails
        {
            public string manufacturer;
            public string product;
            public int serialNumber;
            public ushort VID;
            public ushort PID;
            public string devicePath;
            public int IN_reportByteLength;
            public int OUT_reportByteLength;
            public ushort versionNumber;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct HIDP_CAPS
        {
            public System.UInt16 Usage;
            public System.UInt16 UsagePage;
            public System.UInt16 InputReportByteLength;
            public System.UInt16 OutputReportByteLength;
            public System.UInt16 FeatureReportByteLength;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 17)]
            public System.UInt16[] Reserved;			
            public System.UInt16 NumberLinkCollectionNodes;
            public System.UInt16 NumberInputButtonCaps;
            public System.UInt16 NumberInputValueCaps;
            public System.UInt16 NumberInputDataIndices;
            public System.UInt16 NumberOutputButtonCaps;
            public System.UInt16 NumberOutputValueCaps;
            public System.UInt16 NumberOutputDataIndices;
            public System.UInt16 NumberFeatureButtonCaps;
            public System.UInt16 NumberFeatureValueCaps;
            public System.UInt16 NumberFeatureDataIndices;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct SP_DEVINFO_DATA
        {
            public uint cbSize;
            public Guid ClassGuid;
            public uint DevInst;
            public IntPtr Reserved;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct SP_DEVICE_INTERFACE_DATA
        {
            public uint cbSize;
            public Guid InterfaceClassGuid;
            public uint Flags;
            public IntPtr Reserved;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
        private struct SP_DEVICE_INTERFACE_DETAIL_DATA
        {
            public int cbSize;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string DevicePath;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct HIDD_ATTRIBUTES
        {
            public Int32 Size;
            public Int16 VendorID;
            public Int16 ProductID;
            public Int16 VersionNumber;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct COMMTIMEOUTS
        {
            public UInt32 ReadIntervalTimeout;
            public UInt32 ReadTotalTimeoutMultiplier;
            public UInt32 ReadTotalTimeoutConstant;
            public UInt32 WriteTotalTimeoutMultiplier;
            public UInt32 WriteTotalTimeoutConstant;
        }

        public bool deviceConnected { get; set; }
        private SafeFileHandle handle_read;
        private SafeFileHandle handle_write;
        private FileStream FS_read;
        private FileStream FS_write;
        private HIDP_CAPS capabilities;
        public InterfaceDetails productInfo;
        public event dataReceivedEvent dataReceived;
        public delegate void dataReceivedEvent(byte[] message);
        public byte[] readData;

        public static InterfaceDetails[] GetConnectedDevices()
        {
            InterfaceDetails[] devices = new InterfaceDetails[0];

            SP_DEVINFO_DATA devInfo = new SP_DEVINFO_DATA();
            SP_DEVICE_INTERFACE_DATA devIface = new SP_DEVICE_INTERFACE_DATA();
            devInfo.cbSize = (uint)Marshal.SizeOf(devInfo);
            devIface.cbSize = (uint)(Marshal.SizeOf(devIface));

            Guid G = new Guid();
            HidD_GetHidGuid(ref G);

            IntPtr i = SetupDiGetClassDevs(ref G, IntPtr.Zero, IntPtr.Zero, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

            SP_DEVICE_INTERFACE_DETAIL_DATA didd = new SP_DEVICE_INTERFACE_DETAIL_DATA();
            if (IntPtr.Size == 8) {
                didd.cbSize = 8;
            } else {
                didd.cbSize = 4 + Marshal.SystemDefaultCharSize;
            }

            int j = -1;
            bool b = true;
            int error;
            SafeFileHandle tempHandle;

            while (b) {
                j++;

                b = SetupDiEnumDeviceInterfaces(i, IntPtr.Zero, ref G, (uint)j, ref devIface);
                error = Marshal.GetLastWin32Error();
                if (b == false)
                    break;

                uint requiredSize = 0;
                bool b1 = SetupDiGetDeviceInterfaceDetail(i, ref devIface, ref didd, 256, out requiredSize, ref devInfo);
                string devicePath = didd.DevicePath;

                tempHandle = CreateFile(devicePath, 
                    GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    IntPtr.Zero, OPEN_EXISTING, 0, IntPtr.Zero);

                IntPtr ptrToPreParsedData = new IntPtr();
                bool ppdSucsess = HidD_GetPreparsedData(tempHandle, ref ptrToPreParsedData);
                if (ppdSucsess == false)
                    continue;

                HIDP_CAPS capabilities = new HIDP_CAPS();
                int hidCapsSucsess = HidP_GetCaps(ptrToPreParsedData, ref capabilities);

                HIDD_ATTRIBUTES attributes = new HIDD_ATTRIBUTES();
                bool hidAttribSucsess = HidD_GetAttributes(tempHandle, ref attributes);

                string productName = "";
                string SN = "";
                string manfString = "";
                IntPtr buffer = Marshal.AllocHGlobal(126); 
                if (HidD_GetProductString(tempHandle, buffer, 126)) productName = Marshal.PtrToStringAuto(buffer);
                if (HidD_GetSerialNumberString(tempHandle, buffer, 126)) SN = Marshal.PtrToStringAuto(buffer);
                if (HidD_GetManufacturerString(tempHandle, buffer, 126)) manfString = Marshal.PtrToStringAuto(buffer);
                Marshal.FreeHGlobal(buffer);

                HidD_FreePreparsedData(ref ptrToPreParsedData);

                InterfaceDetails productInfo = new InterfaceDetails();
                productInfo.devicePath = devicePath;
                productInfo.manufacturer = manfString;
                productInfo.product = productName;
                productInfo.PID = (ushort)attributes.ProductID;
                productInfo.VID = (ushort)attributes.VendorID;
                productInfo.versionNumber = (ushort)attributes.VersionNumber;
                productInfo.IN_reportByteLength = capabilities.InputReportByteLength;
                productInfo.OUT_reportByteLength = capabilities.OutputReportByteLength;

                if (StringIsInteger(SN))
                    productInfo.serialNumber = Convert.ToInt32(SN);

                int newSize = devices.Length + 1;
                Array.Resize(ref devices, newSize);
                devices[newSize - 1] = productInfo;
            }
            SetupDiDestroyDeviceInfoList(i);

            return devices;
        }

        public HIDDevice(string devicePath)
        {
            InitDevice(devicePath);
        }

        private void InitDevice(string devicePath)
        {
            deviceConnected = false;

            handle_read = CreateFile(devicePath, 
                GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                IntPtr.Zero, OPEN_EXISTING, 0, IntPtr.Zero);

            handle_write = CreateFile(devicePath, 
                GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                IntPtr.Zero, OPEN_EXISTING, 0, IntPtr.Zero);

            IntPtr ptrToPreParsedData = new IntPtr();
            bool ppdSucsess = HidD_GetPreparsedData(handle_read, ref ptrToPreParsedData);

            capabilities = new HIDP_CAPS();
            int hidCapsSucsess = HidP_GetCaps(ptrToPreParsedData, ref capabilities);

            HIDD_ATTRIBUTES attributes = new HIDD_ATTRIBUTES();
            bool hidAttribSucsess = HidD_GetAttributes(handle_read, ref attributes);

            string productName = "";
            string SN = "";
            string manfString = "";
            IntPtr buffer = Marshal.AllocHGlobal(126);
            if (HidD_GetProductString(handle_read, buffer, 126)) productName = Marshal.PtrToStringAuto(buffer);
            if (HidD_GetSerialNumberString(handle_read, buffer, 126)) SN = Marshal.PtrToStringAuto(buffer);
            if (HidD_GetManufacturerString(handle_read, buffer, 126)) manfString = Marshal.PtrToStringAuto(buffer);
            Marshal.FreeHGlobal(buffer);

            HidD_FreePreparsedData(ref ptrToPreParsedData);

            if (handle_read.IsInvalid) {
                return;
            }

            deviceConnected = true;

            productInfo = new InterfaceDetails();
            productInfo.devicePath = devicePath;
            productInfo.manufacturer = manfString;
            productInfo.product = productName;
            if (StringIsInteger(SN))
                productInfo.serialNumber = Convert.ToInt32(SN);
            productInfo.PID = (ushort)attributes.ProductID;
            productInfo.VID = (ushort)attributes.VendorID;
            productInfo.versionNumber = (ushort)attributes.VersionNumber;
            productInfo.IN_reportByteLength = capabilities.InputReportByteLength;
            productInfo.OUT_reportByteLength = capabilities.OutputReportByteLength;

            FS_read = new FileStream(handle_read, FileAccess.ReadWrite, capabilities.OutputReportByteLength, false);
            FS_write = new FileStream(handle_write, FileAccess.ReadWrite, capabilities.InputReportByteLength, false);

            ReadAsync();
        }

        public void Close()
        {
            if (!deviceConnected) {
                return;
            }
            deviceConnected = false;

            try {
                if (FS_read != null)
                    FS_read.Close();
                if (FS_write != null)
                    FS_write.Close();

                if ((handle_read != null) && (!(handle_read.IsInvalid)))
                    handle_read.Close();
                if ((handle_write != null) && (!(handle_write.IsInvalid)))
                    handle_write.Close();

            } catch {}
        }

        public void Write(byte[] data)
        {
            if (!deviceConnected) {
                return;
            }

            byte[] packet = new byte[capabilities.OutputReportByteLength];
            Array.Copy(data, 0, packet, 1, data.Length);
            packet[0] = 0;

            if (FS_write.CanWrite) {
                FS_write.Write(packet, 0, packet.Length);
            }
        }

        private void ReadAsync()
        {
            if (!deviceConnected) {
                return;
            }

            if (FS_read.CanRead) {
                readData = new byte[capabilities.InputReportByteLength];
                FS_read.BeginRead(readData, 0, readData.Length, new AsyncCallback(GetInputReportData), readData);
            }
        }

        private void GetInputReportData(IAsyncResult ar)
        {
            if (!deviceConnected) {
                return;
            }

            try {
                FS_read.EndRead(ar);

                if (FS_read.CanRead) {
                    FS_read.BeginRead(readData, 0, readData.Length, new AsyncCallback(GetInputReportData), readData);
                    dataReceived(readData);
                }

            } catch {
                deviceConnected = false;
            }
        }

        private static bool StringIsInteger(string val)
        {
            Double result;
            return Double.TryParse(val, System.Globalization.NumberStyles.Integer,
                System.Globalization.CultureInfo.CurrentCulture, out result);
        }

        private static string NumToHexString(ushort num)
        {
            return String.Format("{0:X}", num);
        }
    }
}
