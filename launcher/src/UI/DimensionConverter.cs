namespace Windower.UI
{
    using Core;
    using System;
    using System.Globalization;
    using System.Windows.Data;

    public class DimensionConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture) =>
            values != null && values.Length == 2 ?
            new Dimension(values[0] as int? ?? 0, values[1] as int? ?? 0) :
            default(Dimension);

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            var dimension = value as Dimension? ?? default(Dimension);
            return new object[] { dimension.Width, dimension.Height };
        }
    }
}