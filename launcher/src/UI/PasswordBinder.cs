namespace Windower.UI
{
    using System;
    using System.Security;
    using System.Windows;
    using System.Windows.Controls;

    public static class PasswordBinder
    {
        public static readonly DependencyProperty SecurePasswordProperty =
            DependencyProperty.RegisterAttached("SecurePassword", typeof(SecureString), typeof(PasswordBinder));

        public static readonly DependencyProperty AttachedProperty =
            DependencyProperty.RegisterAttached("Attached", typeof(bool), typeof(PasswordBinder),
                new FrameworkPropertyMetadata(false, OnAttachChanged));

        public static SecureString GetSecurePassword(DependencyObject obj)
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            return (SecureString)obj.GetValue(SecurePasswordProperty);
        }

        public static void SetSecurePassword(DependencyObject obj, SecureString value)
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            obj.SetValue(SecurePasswordProperty, value);
        }

        public static bool GetAttached(DependencyObject obj)
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            return (bool)obj.GetValue(AttachedProperty);
        }

        public static void SetAttached(DependencyObject obj, bool value)
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            obj.SetValue(AttachedProperty, value);
        }

        private static void OnAttachChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is PasswordBox passwordBox)
            {
                var oldValue = (bool)e.OldValue;
                var newValue = (bool)e.NewValue;
                if (oldValue != newValue)
                {
                    if (newValue)
                    {
                        passwordBox.PasswordChanged += OnPasswordChanged;
                    }
                    else
                    {
                        passwordBox.PasswordChanged -= OnPasswordChanged;
                    }
                }
            }
        }

        private static void OnPasswordChanged(object sender, RoutedEventArgs e)
        {
            if (sender is PasswordBox passwordBox)
            {
                SetSecurePassword(passwordBox, passwordBox.SecurePassword);
            }
        }
    }
}