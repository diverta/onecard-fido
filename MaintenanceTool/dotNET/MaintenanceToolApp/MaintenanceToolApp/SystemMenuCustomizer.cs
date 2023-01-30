using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace MaintenanceToolApp
{
    internal class SystemMenuCustomizer
    {
        [StructLayout(LayoutKind.Sequential)]
        struct MENUITEMINFO
        {
            public uint cbSize;
            public uint fMask;
            public uint fType;
            public uint fState;
            public uint wID;
            public IntPtr hSubMenu;
            public IntPtr hbmpChecked;
            public IntPtr hbmpUnchecked;
            public IntPtr dwItemData;
            public string dwTypeData;
            public uint cch;
            public IntPtr hbmpItem;

            public static uint SizeOf
            {
                get { return (uint)Marshal.SizeOf(typeof(MENUITEMINFO)); }
            }
        }

        [DllImport("user32.dll")]
        static extern IntPtr GetSystemMenu(IntPtr hWnd, bool bRevert);

        [DllImport("user32.dll")]
        static extern bool InsertMenuItem(IntPtr hMenu, uint uItem, bool fByPosition, [In] ref MENUITEMINFO lpmii);

        // このクラスのインスタンス
        private static readonly SystemMenuCustomizer Instance = new SystemMenuCustomizer();

        // 親画面に対するイベント通知
        public delegate void HandlerOnSystemMenuVendorFunctionSelected();
        private event HandlerOnSystemMenuVendorFunctionSelected OnSystemMenuVendorFunctionSelected = null!;

        // メニュー項目の表示名称を保持
        private string MenuItemNameVendorFunction = string.Empty;

        //
        // 外部公開用
        //
        public static void AddCustomizedSystemMenu(Window window)
        {
            Instance.AddCustomizedSystemMenuInner(window);
        }

        public static void AddCustomizedSystemMenuItem(string menuItemName, HandlerOnSystemMenuVendorFunctionSelected handler)
        {
            Instance.MenuItemNameVendorFunction = menuItemName;
            Instance.OnSystemMenuVendorFunctionSelected += handler;
        }

        public static void AddHookForCustomizedSystemMenu(HwndSource? hwndSource)
        {
            Instance.AddHookForCustomizedSystemMenuInner(hwndSource);
        }

        //
        // 内部処理
        //
        private void AddCustomizedSystemMenuInner(Window window)
        {
            //
            // システムメニューに「ベンダー向け機能」を追加
            //
            IntPtr hwnd = new WindowInteropHelper(window).Handle;
            IntPtr menu = GetSystemMenu(hwnd, false);

            // システムメニューの６番目に、区切り線を挿入
            //   fMask = MIIM_FTYPE
            //   fType = MFT_SEPARATOR
            MENUITEMINFO item1 = new MENUITEMINFO();
            item1.cbSize = (uint)Marshal.SizeOf(item1);
            item1.fMask = 0x00000100;
            item1.fType = 0x00000800;
            InsertMenuItem(menu, 5, true, ref item1);

            // システムメニューの７番目に、独自メニュー項目を挿入
            //   fMask      = fMask = MIIM_STRING | MIIM_ID;
            //   wID        = メニュー項目のID
            //   dwTypeData = メニュー項目の表示名称
            MENUITEMINFO item2 = new MENUITEMINFO();
            item2.cbSize = (uint)Marshal.SizeOf(item2);
            item2.fMask = 0x00000040 | 0x00000002;
            item2.wID = 0x0001;
            item2.dwTypeData = MenuItemNameVendorFunction;
            InsertMenuItem(menu, 6, true, ref item2);
        }

        private void AddHookForCustomizedSystemMenuInner(HwndSource? hwndSource)
        {
            // システムメニューからメニューアイテム選択時のHookを追加
            if (hwndSource != null) {
                hwndSource.AddHook(new HwndSourceHook(HandlerHwndSourceHook));
            }
        }

        private IntPtr HandlerHwndSourceHook(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            // Hook WM_SYSCOMMAND
            if (msg == 0x112) {
                // システムメニューから「ベンダー向け機能」が選択されたときの処理
                if (wParam.ToInt32() == 0x0001) {
                    OnSystemMenuVendorFunctionSelected();
                }
            }
            return IntPtr.Zero;
        }
    }
}
