namespace Windower.UI.Views
{
    using System;
    using System.Windows.Input;

    public class CrashReportDescriptionViewModel : ViewModelBase
    {
        private INavigationService navigation;
        private string description;

        public CrashReportDescriptionViewModel(INavigationService navigation)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));

            Submit = new DelegateCommand(ExecuteSubmit);
        }

        public ICommand Submit { get; }

        public string Description
        {
            get => description;
            set => Set(ref description, value);
        }

        private void ExecuteSubmit(object arg) => navigation.Close(Description);
    }
}