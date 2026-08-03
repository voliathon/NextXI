namespace Windower.UI
{
    using Core;
    using System;
    using System.Globalization;
    using System.IO;
    using System.Threading;
    using System.Windows;

    internal static class UserInterface
    {
        public static void Run() => Run(null);

        public static void Run(Profile? profile) => RunInternal("Main", profile);

        public static void RunCrashReporter(string signature, string crashDumpPath, string stackTrace) =>
            RunInternal("CrashReporter", signature, crashDumpPath, stackTrace);

        private static void RunInternal(string view, params object[] args)
        {
            if (Thread.CurrentThread.GetApartmentState() == ApartmentState.STA)
            {
                var application = new WindowerApplication();
                LoadStringResources(application);

                application.ShutdownMode = ShutdownMode.OnLastWindowClose;
                application.Startup += (s, e) =>
                {
                    var window = (Window)ViewFactory.Create("Root", view, args);
                    application.MainWindow = window;
                    window.Show();
                };
                application.Run();
            }
            else
            {
                var thread = new Thread(() => RunInternal(view, args)) { Name = "UI Thread" };
                thread.SetApartmentState(ApartmentState.STA);
                thread.Start();
                thread.Join();
            }
        }

        private static void LoadStringResources(Application application)
        {
            var culture = CultureInfo.CurrentUICulture;

            var languageTag = culture.TwoLetterISOLanguageName;
            var cultureTag = culture.Name;

            try
            {
                application.Resources.MergedDictionaries.Add(new ResourceDictionary()
                {
                    Source = new Uri("/windower;component/res/Strings." + languageTag + ".xaml", UriKind.Relative)
                });
            }
            catch (IOException) { }

            if (cultureTag != languageTag)
            {
                try
                {
                    application.Resources.MergedDictionaries.Add(new ResourceDictionary()
                    {
                        Source = new Uri("/windower;component/res/Strings." + cultureTag + ".xaml", UriKind.Relative)
                    });
                }
                catch (IOException) { }
            }
        }
    }
}