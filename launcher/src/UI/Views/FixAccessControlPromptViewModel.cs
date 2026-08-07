namespace Windower.UI.Views
{
    using System;
    using System.Windows.Input;
    using Windower.Core;

    public class FixAccessControlPromptViewModel : ViewModelBase
    {
        private INavigationService navigation;
        private bool doNotAskAgain = false;

        public FixAccessControlPromptViewModel(INavigationService navigation)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));

            Close = new DelegateCommand(ExecuteClose);
        }

        public ICommand Close { get; }

        public bool IsAdministrator { get; } = SecurityService.IsAdministrator();

        public bool DoNotAskAgain
        {
            get => doNotAskAgain;
            set => Set(ref doNotAskAgain, value);
        }

        private void ExecuteClose(object arg) => navigation.Close((arg as bool? ?? false, !DoNotAskAgain));
    }
}