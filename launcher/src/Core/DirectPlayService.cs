// Handles legacy DirectPlay feature detection and DISM installation.
namespace Windower.Core
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Threading;
    using System.Threading.Tasks;
    using Boiler;
    using static System.FormattableString;

    public static class DirectPlayService
    {
        [ComImport]
        [Guid("743F1DC6-5ABA-429F-8BDF-C54D03253DC2")]
        private class DirectPlay8ClientStub
        {
        }

        [RemoteCallable]
        public static bool CheckDirectPlay(CancellationToken token)
        {
            try
            {
                _ = Marshal.ReleaseComObject(new DirectPlay8ClientStub());
                return true;
            }
            catch (UnauthorizedAccessException) { }
            return false;
        }

        public static Task<bool> CheckDirectPlayAsync() =>
            Program.RemoteCallAsync(CheckDirectPlay, CancellationToken.None);

        public static bool InstallDirectPlay()
        {
            var info = new ProcessStartInfo();
            var systemPath = Environment.Is64BitOperatingSystem && !Environment.Is64BitProcess ?
                Path.Combine(Environment.GetEnvironmentVariable("SystemRoot"), "SysNative") :
                Environment.GetFolderPath(Environment.SpecialFolder.System);

            var path = Path.Combine(systemPath, "Dism.exe");
            info.FileName = "cmd";
            info.Arguments = Invariant($"/C {path} /Online /Enable-Feature /FeatureName:DirectPlay /All");
            info.Verb = "runas";
            info.UseShellExecute = true;
            info.WindowStyle = ProcessWindowStyle.Hidden;

            try
            {
                Process.Start(info)?.WaitForExit();
            }
            catch (Win32Exception)
            {
                return false;
            }

            return CheckDirectPlay(CancellationToken.None);
        }

        public static async Task<bool> InstallDirectPlayAsync() => await Task.Run(() => InstallDirectPlay());
    }
}
