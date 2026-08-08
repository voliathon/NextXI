namespace Windower.UI.Views
{
    using Core;
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Windows.Input;

    public class AboutViewModel : ViewModelBase
    {
        private INavigationService navigation;

        public AboutViewModel(INavigationService navigation)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));

            Close = new DelegateCommand(ExecuteClose);
            OpenWebpage = new DelegateCommand(ExecuteOpenWebpage);
        }

        public ICommand Close { get; }

        public ICommand OpenWebpage { get; }

        public static string Copyright { get; } = GetCopyright();

        public static Version LauncherVersion { get; } = GetLauncherVersion();

        public static Version CoreVersion { get; } = GetCoreVersion();

        public static string BuildTag { get; } = Program.BuildTag;

        private static string GetCopyright()
        {
            var attribute = typeof(AboutViewModel).Assembly.GetCustomAttributes(typeof(AssemblyCopyrightAttribute))
                .FirstOrDefault();
            return ((AssemblyCopyrightAttribute)attribute)?.Copyright;
        }

        private static Version GetLauncherVersion() => typeof(AboutViewModel).Assembly.GetName().Version;

        private static Version GetCoreVersion()
        {
            FileVersionInfo info;
            try
            {
                info = FileVersionInfo.GetVersionInfo(Launcher.CorePath);
            }
            catch (FileNotFoundException)
            {
                return null;
            }
            return new Version(info.ProductMajorPart, info.ProductMinorPart, info.ProductBuildPart, info.ProductPrivatePart);
        }

        private void ExecuteClose(object arg) => navigation.Close();

        private void ExecuteOpenWebpage(object arg)
        {
            if (arg is string url)
            {
                // You MUST use ProcessStartInfo with UseShellExecute = true!
                System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo
                {
                    FileName = url,
                    UseShellExecute = true
                });
            }
        }
    }
}
