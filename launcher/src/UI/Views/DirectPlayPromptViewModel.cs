namespace Windower.UI.Views
{
    using System;
    using System.Windows.Input;
    using Windower.Core;

    public class DirectPlayPromptViewModel : ViewModelBase
    {
        private INavigationService navigation;
        ICommand close;

        public DirectPlayPromptViewModel(INavigationService navigation)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));

            close = new DelegateCommand(ExecuteClose);
        }

        public ICommand Close => close;

        public bool IsAdministrator { get; } = Launcher.IsAdministrator();

        private void ExecuteClose(object arg) => navigation.Close(arg as bool? ?? false);
    }
}