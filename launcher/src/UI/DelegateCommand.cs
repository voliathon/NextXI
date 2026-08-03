namespace Windower.UI
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics.CodeAnalysis;
    using System.Windows.Input;

    public class DelegateCommand : ICommand
    {
        private Func<object, bool> canExecute;
        private Action<object> execute;
        private Dictionary<INotifyPropertyChanged, HashSet<string>> observedProperties =
            new Dictionary<INotifyPropertyChanged, HashSet<string>>();

        public DelegateCommand(Action<object> execute) :
            this(execute, null)
        { }

        public DelegateCommand(Action<object> execute, Func<object, bool> canExecute)
        {
            this.execute = execute ?? throw new ArgumentNullException(nameof(execute));
            this.canExecute = canExecute;
        }

        public event EventHandler CanExecuteChanged;

        public bool CanExecute(object parameter)
        {
            var result = canExecute?.Invoke(parameter) ?? true;
            return result;
        }

        public void Execute(object parameter) => execute(parameter);

        public DelegateCommand ObservesProperty(INotifyPropertyChanged obj, string name)
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            if (name == null)
            {
                throw new ArgumentNullException(nameof(name));
            }

            if (!observedProperties.TryGetValue(obj, out var names))
            {
                names = new HashSet<string>();
                observedProperties.Add(obj, names);
                obj.PropertyChanged += PropertyChanged;
            }
            names.Add(name);
            return this;
        }

        [SuppressMessage("Microsoft.Design", "CA1030")]
        public void RaiseCanExecuteChanged() => CanExecuteChanged?.Invoke(this, new EventArgs());

        private void PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (sender is INotifyPropertyChanged obj && observedProperties.TryGetValue(obj, out var names))
            {
                if (names.Contains(e.PropertyName))
                {
                    RaiseCanExecuteChanged();
                }
            }
        }
    }
}