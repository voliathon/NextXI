namespace Windower.UI
{
    using System;
    using System.Globalization;
    using System.Windows.Data;

    public class BooleanEqualityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) => Equals(value, parameter);

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) =>
            (bool)value ? parameter : Binding.DoNothing;
    }
}