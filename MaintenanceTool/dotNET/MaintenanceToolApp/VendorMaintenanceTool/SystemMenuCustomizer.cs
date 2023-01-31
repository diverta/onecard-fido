using MaintenanceToolApp;
using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using VendorMaintenanceTool.VendorFunction;
using static VendorMaintenanceTool.SystemMenuCustomizerConst;

namespace VendorMaintenanceTool
{
    internal class SystemMenuCustomizerConst
    {
        // Indicates the members to be retrieved or set
        public const uint MIIM_FTYPE = 0x00000100;
        public const uint MIIM_STRING = 0x00000040;
        public const uint MIIM_ID = 0x00000002;

        // The menu item type
        public const uint MFT_SEPARATOR = 0x00000800;

        // システムメニューの項目番号（０から始まる数値）
        //   ６番目＝区切り線
        //   ７番目＝独自メニュー項目
        public const uint ITEM_SEPARATOR = 5;
        public const uint ITEM_CUSTOM_MENU = 6;

        // キーボードアクセラレータメッセージ
        public const uint WM_SYSCOMMAND = 0x0112;

        // メニュー項目のID
        public const uint MENU_ID_0001 = 0x0001;
    }

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

        // 二重処理の抑止
        private bool initialized = false;

        //
        // 外部公開用
        //
        public static void AddCustomizedSystemMenu()
        {
            Instance.AddCustomizedSystemMenuInner();
        }

        //
        // 内部処理
        //
        private void AddCustomizedSystemMenuInner() {
            // 二重処理の抑止
            if (initialized) {
                return;
            } else {
                initialized = true;
            }

            // メニュー選択時のイベント捕捉を設定
            Window window = Application.Current.MainWindow;
            HwndSource? hwndSource = PresentationSource.FromVisual(window) as HwndSource;
            AddHookForCustomizedSystemMenu(hwndSource);

            // メニュー項目名称／業務処理を設定後、メニューを表示
            AddCustomizedSystemMenuItem(AppCommon.MSG_MENU_ITEM_NAME_VENDOR_FUNCTION, DoVendorFunction);
            ShowCustomizedSystemMenuItem(window);
        }

        private void ShowCustomizedSystemMenuItem(Window window)
        {
            //
            // システムメニューに「ベンダー向け機能」を追加
            //
            IntPtr hwnd = new WindowInteropHelper(window).Handle;
            IntPtr menu = GetSystemMenu(hwnd, false);

            // システムメニューに区切り線を挿入
            MENUITEMINFO item1 = new MENUITEMINFO();
            item1.cbSize = (uint)Marshal.SizeOf(item1);
            item1.fMask = MIIM_FTYPE;
            item1.fType = MFT_SEPARATOR;
            InsertMenuItem(menu, ITEM_SEPARATOR, true, ref item1);

            // システムメニューに独自メニュー項目を挿入
            //   wID        = メニュー項目のID
            //   dwTypeData = メニュー項目の表示名称
            MENUITEMINFO item2 = new MENUITEMINFO();
            item2.cbSize = (uint)Marshal.SizeOf(item2);
            item2.fMask = MIIM_STRING | MIIM_ID;
            item2.wID = MENU_ID_0001;
            item2.dwTypeData = MenuItemNameVendorFunction;
            InsertMenuItem(menu, ITEM_CUSTOM_MENU, true, ref item2);
        }

        private void AddCustomizedSystemMenuItem(string menuItemName, HandlerOnSystemMenuVendorFunctionSelected handler)
        {
            // メニュー項目名称／業務処理を設定
            MenuItemNameVendorFunction = menuItemName;
            OnSystemMenuVendorFunctionSelected += handler;
        }

        private void AddHookForCustomizedSystemMenu(HwndSource? hwndSource)
        {
            // システムメニューからメニューアイテム選択時のHookを追加
            if (hwndSource != null) {
                hwndSource.AddHook(new HwndSourceHook(HandlerHwndSourceHook));
            }
        }

        private IntPtr HandlerHwndSourceHook(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            // ユーザーがシステムメニューからコマンド選択時
            if (msg == WM_SYSCOMMAND) {
                // 「ベンダー向け機能」選択時
                if (wParam.ToInt32() == MENU_ID_0001) {
                    OnSystemMenuVendorFunctionSelected();
                }
            }
            return IntPtr.Zero;
        }

        //
        // 業務処理
        //
        private static void DoVendorFunction()
        {
            // TODO: 仮の実装です。
            Window window = Application.Current.MainWindow;
            DialogUtil.ShowWarningMessage(window, window.Title, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
        }
    }
}
