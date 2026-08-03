namespace Windower
{
    using System;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    public static class Shell
    {
        private const string appUserModelId = "Windower.Windower";

        public static void Initialize()
        {
            try
            {
                var result = NativeMethods.SetCurrentProcessExplicitAppUserModelID(appUserModelId);
                if (result != NativeMethods.E_NOTIMPL)
                {
                    Marshal.ThrowExceptionForHR(result);
                }
            }
            catch (EntryPointNotFoundException) { }
        }

        public static void CreateLink(string path, string exe, string args)
        {
            var o = new CShellLink();
            var link = (IShellLink)o;
            var file = (IPersistFile)o;
            link.SetPath(exe);
            link.SetArguments(args);
            file.Save(path, true);
            Marshal.ReleaseComObject(link);
        }

        [ComImport]
        [Guid("00021401-0000-0000-C000-000000000046")]
        [ClassInterface(ClassInterfaceType.None)]
        private class CShellLink { }

        [ComImport]
        [Guid("000214F9-0000-0000-C000-000000000046")]
        [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
        private interface IShellLink
        {
            void GetPath(IntPtr pszFile, int cch, IntPtr pfd, uint fFlags);
            void GetIDList(out IntPtr ppidl);
            void SetIDList(IntPtr pidl);
            void GetDescription(IntPtr pszName, int cch);
            void SetDescription([MarshalAs(UnmanagedType.LPWStr)] string pszName);
            void GetWorkingDirectory(IntPtr pszDir, int cch);
            void SetWorkingDirectory([MarshalAs(UnmanagedType.LPWStr)] string pszDir);
            void GetArguments(IntPtr pszArgs, int cch);
            void SetArguments([MarshalAs(UnmanagedType.LPWStr)] string pszArgs);
            void GetHotkey(out ushort pwHotkey);
            void SetHotkey(ushort wHotkey);
            void GetShowCmd(out int piShowCmd);
            void SetShowCmd(int iShowCmd);
            void GetIconLocation(IntPtr pszIconPath, int cch, out int piIcon);
            void SetIconLocation([MarshalAs(UnmanagedType.LPWStr)] string pszIconPath, int iIcon);
            void SetRelativePath([MarshalAs(UnmanagedType.LPWStr)] string pszPathRel, int dwReserved);
            void Resolve(IntPtr hwnd, uint fFlags);
            void SetPath([MarshalAs(UnmanagedType.LPWStr)] string pszFile);
        }
    }
}