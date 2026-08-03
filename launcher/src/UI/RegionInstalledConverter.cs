namespace Windower.UI
{
    using PlayOnline;
    using System;
    using System.Globalization;
    using System.Windows.Data;

    public class RegionInstalledConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) =>
            parameter is Region region && (region.IsInstalled() || region.IsOwned());

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) =>
            throw new NotSupportedException();
    }
}