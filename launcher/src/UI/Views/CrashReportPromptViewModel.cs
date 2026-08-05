namespace Windower.UI.Views
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Windows.Input;

    public class CrashReportPromptViewModel : ViewModelBase
    {
        private INavigationService navigation;
        private string location;

        public CrashReportPromptViewModel(INavigationService navigation, string location)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));

            Close = new DelegateCommand(ExecuteClose);
            Submit = new DelegateCommand(ExecuteSubmit);
            ShowReport = new DelegateCommand(ExecuteShowReport, CanExecuteShowReport);

            this.location = location;
        }

        public static bool EncryptionEnabled { get; } = CrashReporter.EncryptionEnabled;

        public ICommand ShowReport { get; }

        public ICommand Close { get; }

        public ICommand Submit { get; }

        private void ExecuteShowReport(object arg)
        {
            if (location != null && File.Exists(location))
            {
                _ = Process.Start(location);
            }
        }

        private bool CanExecuteShowReport(object arg) => location != null && File.Exists(location);

        private void ExecuteClose(object arg) => navigation.Close(false);

        private void ExecuteSubmit(object arg) => navigation.Close(true);
    }
}