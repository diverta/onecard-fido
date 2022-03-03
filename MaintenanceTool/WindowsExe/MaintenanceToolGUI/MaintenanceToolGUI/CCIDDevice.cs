using MaintenanceToolCommon;
using System;
using System.Runtime.InteropServices;
using System.Text;

namespace MaintenanceToolGUI
{
    internal class CCIDDeviceConst
    {
        public const uint SCARD_S_SUCCESS = 0;
        public const uint SCARD_E_NO_SERVICE = 0x8010001D;
        public const uint SCARD_E_TIMEOUT = 0x8010000A;

        public const uint SCARD_SCOPE_USER = 0;
        public const uint SCARD_SCOPE_TERMINAL = 1;
        public const uint SCARD_SCOPE_SYSTEM = 2;

        public const int SCARD_STATE_UNAWARE = 0x0000;
        public const int SCARD_STATE_CHANGED = 0x00000002;
        public const int SCARD_STATE_PRESENT = 0x00000020;
        public const UInt32 SCARD_STATE_EMPTY = 0x00000010;
        public const int SCARD_SHARE_SHARED = 0x00000002;
        public const int SCARD_SHARE_EXCLUSIVE = 0x00000001;
        public const int SCARD_SHARE_DIRECT = 0x00000003;

        public const int SCARD_PROTOCOL_T0 = 1;
        public const int SCARD_PROTOCOL_T1 = 2;
        public const int SCARD_PROTOCOL_RAW = 4;

        public const int SCARD_LEAVE_CARD = 0;
        public const int SCARD_RESET_CARD = 1;
        public const int SCARD_UNPOWER_CARD = 2;
        public const int SCARD_EJECT_CARD = 3;

        public const int SCARD_UNKNOWN = 0x00000000;
        public const int SCARD_ABSENT = 0x00000001;
        public const int SCARD_PRESENT = 0x00000002;
        public const int SCARD_SWALLOWED = 0x00000003;
        public const int SCARD_POWERED = 0x00000004;
        public const int SCARD_NEGOTIABLE = 0x00000005;
        public const int SCARD_SPECIFICMODE = 0x00000006;
    }

    internal class CCIDDevice
    {
        private const string WINSCARD_DLL = "winscard.dll";
        private const string KERNEL32_DLL = "kernel32.dll";

        [DllImport(WINSCARD_DLL)]
        public static extern uint SCardEstablishContext(uint dwScope, IntPtr pvReserved1, IntPtr pvReserved2, out IntPtr phContext);

        [DllImport(WINSCARD_DLL, EntryPoint = "SCardListReadersW", CharSet = CharSet.Unicode)]
        public static extern uint SCardListReaders(
            IntPtr hContext, byte[] mszGroups, byte[] mszReaders, ref UInt32 pcchReaders);

        [DllImport(WINSCARD_DLL)]
        public static extern uint SCardReleaseContext(IntPtr phContext);

        [DllImport(WINSCARD_DLL, EntryPoint = "SCardConnectW", CharSet = CharSet.Unicode)]
        public static extern uint SCardConnect(IntPtr hContext, string szReader, uint dwShareMode, uint dwPreferredProtocols, ref IntPtr phCard, ref IntPtr pdwActiveProtocol);

        [DllImport(WINSCARD_DLL)]
        public static extern uint SCardDisconnect(IntPtr hCard, int Disposition);

        [StructLayout(LayoutKind.Sequential)]
        internal class SCARD_IO_REQUEST
        {
            internal uint dwProtocol;
            internal int cbPciLength;
            public SCARD_IO_REQUEST()
            {
                dwProtocol = 0;
            }
        }

        [DllImport(WINSCARD_DLL)]
        public static extern uint SCardTransmit(IntPtr hCard, IntPtr pioSendRequest, byte[] SendBuff, int SendBuffLen, SCARD_IO_REQUEST pioRecvRequest, byte[] RecvBuff, ref int RecvBuffLen);

        [DllImport(WINSCARD_DLL)]
        public static extern uint SCardControl(IntPtr hCard, int controlCode, byte[] inBuffer, int inBufferLen, byte[] outBuffer, int outBufferLen, ref int bytesReturned);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct SCARD_READERSTATE
        {
            internal string szReader;
            internal IntPtr pvUserData;
            internal UInt32 dwCurrentState;
            internal UInt32 dwEventState;
            internal UInt32 cbAtr;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 36)]
            internal byte[] rgbAtr;
        }

        [DllImport(WINSCARD_DLL, EntryPoint = "SCardGetStatusChangeW", CharSet = CharSet.Unicode)]
        public static extern uint SCardGetStatusChange(IntPtr hContext, int dwTimeout, [In, Out] SCARD_READERSTATE[] rgReaderStates, int cReaders);

        [DllImport(WINSCARD_DLL)]
        public static extern int SCardStatus(IntPtr hCard, string szReader, ref int cch, ref int state, ref int protocol, ref byte[] bAttr, ref int cByte);


        [DllImport(KERNEL32_DLL, SetLastError = true)]
        public static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport(KERNEL32_DLL)]
        public static extern void FreeLibrary(IntPtr handle);

        [DllImport(KERNEL32_DLL)]
        public static extern IntPtr GetProcAddress(IntPtr handle, string procName);

        private IntPtr hContext = IntPtr.Zero;
        private IntPtr hCard = IntPtr.Zero;
        private string ReaderName;

        // 受信データを保持
        private const uint MaxRecvDataLen = 256;
        private byte[] RecvBuffer = new byte[MaxRecvDataLen + 2];
        private int RecvLength = 0;

        private bool EstablishContext()
        {
            uint ret = SCardEstablishContext(CCIDDeviceConst.SCARD_SCOPE_USER, IntPtr.Zero, IntPtr.Zero, out hContext);
            if (ret != CCIDDeviceConst.SCARD_S_SUCCESS) {
                if (ret == CCIDDeviceConst.SCARD_E_NO_SERVICE) {
                    AppCommon.OutputLogError("SCardEstablishContext returns SCARD_E_NO_SERVICE");
                } else {
                    AppCommon.OutputLogError(string.Format("SCardEstablishContext returns {0}", ret));
                }
                return false;
            }
            return true;
        }

        private bool ListReaders()
        {
            uint pcchReaders = 0;

            // CCIDデバイス名称のバッファサイズを取得
            uint ret = SCardListReaders(hContext, null, null, ref pcchReaders);
            if (ret != CCIDDeviceConst.SCARD_S_SUCCESS) {
                // 検出失敗
                AppCommon.OutputLogError("CCIDデバイスを確認できません。");
                return false;
            }

            // CCIDデバイス名称を取得
            byte[] mszReaders = new byte[pcchReaders * 2];
            ret = SCardListReaders(hContext, null, mszReaders, ref pcchReaders);
            if (ret != CCIDDeviceConst.SCARD_S_SUCCESS) {
                // 検出失敗
                AppCommon.OutputLogError("CCIDデバイス名称を取得できませんでした。");
                return false;
            }

            UnicodeEncoding unicodeEncoding = new UnicodeEncoding();
            string readerNameMultiString = unicodeEncoding.GetString(mszReaders);

            // 認識した最初のCCIDデバイスを使用
            int nullindex = readerNameMultiString.IndexOf((char)0);
            ReaderName = readerNameMultiString.Substring(0, nullindex);
            AppCommon.OutputLogDebug(string.Format("CCIDデバイス[{0}]を検出しました。", ReaderName));
            return true;
        }

        private bool Connect()
        {
            IntPtr activeProtocol = IntPtr.Zero;

            uint ret = SCardConnect(hContext, ReaderName, CCIDDeviceConst.SCARD_SHARE_SHARED, CCIDDeviceConst.SCARD_PROTOCOL_T1, ref hCard, ref activeProtocol);
            if (ret != CCIDDeviceConst.SCARD_S_SUCCESS) {
                AppCommon.OutputLogError(string.Format("CCIDデバイスに接続できませんでした。RC={0}", ret));
                return false;
            }
            AppCommon.OutputLogDebug(string.Format("CCIDデバイス[{0}]に接続しました。", ReaderName));
            return true;
        }

        public bool ConnectDevice()
        {
            if (EstablishContext() == false) {
                return false;
            }
            if (ListReaders() == false) {
                return false;
            }
            return Connect();
        }

        public bool Transmit(byte[] sendBuffer)
        {
            SCARD_IO_REQUEST ioRecv = new SCARD_IO_REQUEST {
                cbPciLength = 255
            };

            int pcbRecvLength = RecvBuffer.Length;
            int cbSendLength = sendBuffer.Length;

            IntPtr handle = LoadLibrary(WINSCARD_DLL);
            IntPtr pci = GetProcAddress(handle, "g_rgSCardT1Pci");
            FreeLibrary(handle);

            uint ret = SCardTransmit(hCard, pci, sendBuffer, cbSendLength, ioRecv, RecvBuffer, ref pcbRecvLength);
            if (ret != CCIDDeviceConst.SCARD_S_SUCCESS) {
                AppCommon.OutputLogError(string.Format("CCIDデバイスへの送信処理が失敗しました。RC={0}", ret));
                RecvLength = 0;
                return false;
            }

            // 受信データ長を保持
            RecvLength = pcbRecvLength;
            return true;
        }

        public byte[] GetReceivedBytes()
        {
            // 例外回避
            if (RecvLength == 0) {
                return new byte[0];
            }

            // 受信したデータを、別領域にコピーして戻す
            byte[] receivedBytes = new byte[RecvLength];
            Array.Copy(RecvBuffer, 0, receivedBytes, 0, RecvLength);
            return receivedBytes;
        }

        public void Disconnect()
        {
            uint ret = SCardDisconnect(hCard, CCIDDeviceConst.SCARD_LEAVE_CARD);
            if (ret != CCIDDeviceConst.SCARD_S_SUCCESS) {
                AppCommon.OutputLogError(string.Format("CCIDデバイス切断処理が失敗しました。RC={0}", ret));
            } else {
                AppCommon.OutputLogDebug(string.Format("CCIDデバイス[{0}]から切断しました。", ReaderName));
            }

            // 内部変数を初期化する。
            hContext = IntPtr.Zero;
            hCard = IntPtr.Zero;
            ReaderName = null;
            RecvLength = 0;
        }
    }
}
