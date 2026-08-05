namespace Windower.UI
{
    using System;
    using System.Runtime.InteropServices;
    using System.Windows;
    using System.Windows.Interop;
    using System.Windows.Media.Imaging;

    public static class StockIcons
    {
        private static Lazy<BitmapSource> shield = new Lazy<BitmapSource>(() => GetStockIcon(NativeMethods.SIID_SHIELD));

        public static BitmapSource Shield => shield.Value;

        private static BitmapSource GetStockIcon(int id)
        {
            var info = default(NativeMethods.SHSTOCKICONINFO);
            info.cbSize = (uint)Marshal.SizeOf(typeof(NativeMethods.SHSTOCKICONINFO));
            try
            {
                Marshal.ThrowExceptionForHR(NativeMethods.SHGetStockIconInfo(id,
                    NativeMethods.SHGSI_ICON | NativeMethods.SHGSI_SMALLICON, ref info));
                using (new SafeIconHandle(info.hIcon))
                {
                    return Imaging.CreateBitmapSourceFromHIcon(info.hIcon, Int32Rect.Empty, BitmapSizeOptions.FromEmptyOptions());
                }
            }
            catch (DllNotFoundException) { }
            catch (EntryPointNotFoundException) { }
            return null;
        }
    }
}