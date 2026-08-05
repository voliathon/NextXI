namespace Windower.UI.Views
{
    using System;
    using System.Windows.Input;

    public class ClientNotFoundViewModel : ViewModelBase
    {
        private INavigationService navigation;

        public ClientNotFoundViewModel(INavigationService navigation)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));

            Close = new DelegateCommand(ExecuteClose);
        }

        public ICommand Close { get; }

        private void ExecuteClose(object arg) => navigation.Close();
    }
}