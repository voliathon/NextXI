namespace Windower.UI
{
    using System;
    using System.Globalization;
    using System.Windows;
    using System.Windows.Data;

    public class ResourceStringConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var name = string.Format(CultureInfo.InvariantCulture, parameter as string ?? "{1}_{0}", value ?? "null",
                value?.GetType().Name ?? "null");
            return Application.Current.TryFindResource("Strings." + name) ?? name;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) =>
            throw new NotSupportedException();
    }
}