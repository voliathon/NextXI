namespace Windower.UI
{
    using System;
    using System.Globalization;
    using System.Reflection;
    using System.Runtime.ExceptionServices;
    using System.Windows;

    internal static class ViewFactory
    {
        public static string ViewFormat { get; set; } = typeof(ViewFactory).Namespace + ".Views.{0}View";
        public static string ViewModelFormat { get; set; } = typeof(ViewFactory).Namespace + ".Views.{0}ViewModel";

        public static FrameworkElement Create(string name, params object[] args)
        {
            try
            {
                var viewType = Type.GetType(string.Format(CultureInfo.InvariantCulture, ViewFormat, name));
                if (viewType == null)
                {
                    throw new ArgumentException("Invalid view name: " + name);
                }

                var viewModelType = Type.GetType(string.Format(CultureInfo.InvariantCulture, ViewModelFormat, name));
                if (viewModelType == null)
                {
                    throw new ArgumentException("Invalid view model name: " + name);
                }

                var view = (FrameworkElement)Activator.CreateInstance(viewType);
                view.DataContext = Activator.CreateInstance(viewModelType, args);
                return view;
            }
            catch (TargetInvocationException e)
            {
                if (e.InnerException != null)
                {
                    ExceptionDispatchInfo.Capture(e.InnerException).Throw();
                }
                throw;
            }
        }
    }
}