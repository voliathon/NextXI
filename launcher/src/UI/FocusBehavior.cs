namespace Windower.UI
{
    using System;
    using System.Windows;
    using System.Windows.Input;

    public static class FocusBehavior
    {
        public static readonly DependencyProperty FocusFirstProperty =
            DependencyProperty.RegisterAttached("FocusFirst", typeof(bool), typeof(FocusBehavior),
                new PropertyMetadata(false, OnFocusFirstChanged));

        public static bool GetFocusFirst(DependencyObject obj)
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            return (bool)obj.GetValue(FocusFirstProperty);
        }

        public static void SetFocusFirst(DependencyObject obj, bool value)
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            obj.SetValue(FocusFirstProperty, value);
        }

        private static void OnFocusFirstChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is FrameworkElement control && e.NewValue is bool value)
            {
                if (value)
                {
                    control.Loaded += OnLoaded;
                }
                else
                {
                    control.Loaded -= OnLoaded;
                }
            }
        }

        private static void OnLoaded(object sender, RoutedEventArgs e)
        {
            var control = sender as FrameworkElement;
            control?.MoveFocus(new TraversalRequest(FocusNavigationDirection.Next));
        }
    }
}