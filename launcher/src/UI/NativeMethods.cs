namespace Windower.UI
{
    using System;
    using System.Runtime.InteropServices;

    internal static class NativeMethods
    {
        public const int SHGSI_SMALLICON = 0x000000001;
        public const int SHGSI_ICON = 0x00000100;

        public const int SIID_SHIELD = 77;

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool DestroyIcon(IntPtr hIcon);

        [DllImport("shell32.dll", CallingConvention = CallingConvention.Winapi)]
        public static extern int SHGetStockIconInfo(int siid, uint uFlags, ref SHSTOCKICONINFO psii);

        [DllImport("dwmapi.dll", CallingConvention = CallingConvention.Winapi)]
        public static extern int DwmExtendFrameIntoClientArea(IntPtr hWnd, [In] ref MARGINS pMarInset);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct SHSTOCKICONINFO
        {
            public uint cbSize;
            public IntPtr hIcon;
            public int iSysImageIndex;
            public int iIcon;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
            public string szPath;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct MARGINS
        {
            public int cxLeftWidth;
            public int cxRightWidth;
            public int cyTopHeight;
            public int cyBottomHeight;
        }
    }
}