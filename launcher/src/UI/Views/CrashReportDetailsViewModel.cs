namespace Windower.UI.Views
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Windows.Input;
    using System.Windows.Shapes;

    public class CrashReportDetailsViewModel : ViewModelBase
    {
        private INavigationService navigation;
        private string localReport;
        private Uri remoteReport;
        private bool deleteLocalReport;

        public CrashReportDetailsViewModel(INavigationService navigation, string status, string localReport, Uri remoteReport)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));

            Close = new DelegateCommand(ExecuteClose);
            ShowLocalReport = new DelegateCommand(ExecuteShowLocalReport, CanExecuteShowLocalReport);
            ShowRemoteReport = new DelegateCommand(ExecuteShowRemoteReport, CanExecuteShowRemoteReport);
            Status = status;

            switch (Status)
            {
                default:
                case "ReportNotSubmitted":
                case "ReportingDisabled":
                    DeleteLocalReport = false;
                    break;

                case "ReportSubmitted":
                case "ReportExists":
                    DeleteLocalReport = true;
                    break;
            }

            this.localReport = localReport;
            this.remoteReport = remoteReport;
        }

        public ICommand ShowLocalReport { get; }

        public ICommand ShowRemoteReport { get; }

        public ICommand Close { get; }

        public string Status { get; }

        public bool DeleteLocalReport
        {
            get => deleteLocalReport;
            set => Set(ref deleteLocalReport, value);
        }

        private void ExecuteShowLocalReport(object obj)
        {
            if (File.Exists(localReport))
            {
                Process.Start(new ProcessStartInfo(localReport) { UseShellExecute = true });
            }
        }

        private bool CanExecuteShowLocalReport(object obj) => File.Exists(localReport);

        private void ExecuteShowRemoteReport(object obj)
        {
            if (remoteReport.IsAbsoluteUri)
            {
                Process.Start(new ProcessStartInfo(remoteReport.AbsoluteUri) { UseShellExecute = true });
            }
        }

        private bool CanExecuteShowRemoteReport(object obj) => remoteReport != null && remoteReport.IsAbsoluteUri;

        private void ExecuteClose(object arg) => navigation.Close(DeleteLocalReport);
    }
}