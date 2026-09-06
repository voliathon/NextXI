namespace Windower.UI.Views
{
    using Core;
    using System;
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

        // ---------------------------------------------------------
        // FORCED 1.0.0.0 VERSIONS (Bypasses Assembly/File checks entirely)
        // ---------------------------------------------------------
        public static Version LauncherVersion { get; } = new Version(1, 0, 0, 0);
        public static Version CoreVersion { get; } = new Version(1, 0, 0, 0);

        public static string BuildTag { get; } = Program.BuildTag;

        private static string GetCopyright()
        {
            var attribute = typeof(AboutViewModel).Assembly.GetCustomAttributes(typeof(AssemblyCopyrightAttribute))
                .FirstOrDefault();
            return ((AssemblyCopyrightAttribute)attribute)?.Copyright;
        }

        private void ExecuteClose(object arg) => navigation.Close();

        private void ExecuteOpenWebpage(object arg)
        {
            if (arg is string url)
            {
                System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo
                {
                    FileName = url,
                    UseShellExecute = true
                });
            }
        }
    }
}
