namespace Windower
{
    using System;
    using System.Collections;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Xml.Linq;

    internal static class Paths
    {
        private static Lazy<GlobalPaths> lazyGlobalPaths = new Lazy<GlobalPaths>(() => new GlobalPaths());

        public static void Initialize()
        {
            Environment.SetEnvironmentVariable("SAVEDGAMES", GetSavedGamesPath());
            Environment.SetEnvironmentVariable("WINDOWER", GetWindowerPath());
        }

        public static string GlobalSettingsPath => lazyGlobalPaths.Value.SettingsPath;

        public static string GlobalUserPath => lazyGlobalPaths.Value.UserPath;

        public static string GlobalTempPath => lazyGlobalPaths.Value.TempPath;

        public static string ExpandPath(string path) => Environment.ExpandEnvironmentVariables(path);

        public static string CollapsePath(string path)
        {
            if (path != null)
            {
                var env =
                    from DictionaryEntry e in Environment.GetEnvironmentVariables()
                    let p = GetPath(e)
                    where p != null
                    orderby p.Length descending, p, e.Key
                    select new { Path = p, Variable = (string)e.Key };

                foreach (var p in env)
                {
                    if (path.StartsWith(p.Path, StringComparison.InvariantCultureIgnoreCase))
                    {
                        return '%' + p.Variable + '%' + path.Substring(p.Path.Length);
                    }
                }
            }

            return path;
        }

        private static string GetSavedGamesPath()
        {
            SafeCoTaskMemHandle buffer = null;
            try
            {
                var id = NativeMethods.FOLDERID_SavedGames;
                Marshal.ThrowExceptionForHR(NativeMethods.SHGetKnownFolderPath(ref id, 0, IntPtr.Zero, out buffer));
                var success = false;
                try
                {
                    buffer.DangerousAddRef(ref success);
                    return Marshal.PtrToStringUni(buffer.DangerousGetHandle());
                }
                finally
                {
                    if (success)
                    {
                        buffer.DangerousRelease();
                    }
                }
            }
            catch (EntryPointNotFoundException)
            {
                return Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), "My Games");
            }
            finally
            {
                buffer?.Dispose();
            }
        }

        private static string GetWindowerPath()
        {
            return Path.GetDirectoryName(typeof(Paths).Assembly.Location);
        }

        private static string GetPath(DictionaryEntry e)
        {
            var path = (string)e.Value;
            try
            {
                if (Path.IsPathRooted(path) && string.Equals(path, Path.GetFullPath(path),
                    StringComparison.InvariantCultureIgnoreCase) && !File.Exists(path))
                {
                    return Path.GetFullPath(path);
                }
            }
            catch (ArgumentException) { }
            catch (NotSupportedException) { }
            catch (PathTooLongException) { }
            return null;
        }

        private class GlobalPaths
        {
            public GlobalPaths()
            {
                var root = GetPathsRoot();

                SettingsPath = (string)root.Element("settings-path") ?? Path.Combine("%LOCALAPPDATA%", "NextXI");
                UserPath = (string)root.Element("user-path") ?? Path.Combine("%SAVEDGAMES%", "NextXI");
                TempPath = (string)root.Element("temp-path") ?? Path.Combine("%TEMP%", "NextXI");
            }

            public string SettingsPath { get; }

            public string UserPath { get; }

            public string TempPath { get; }

            private static XElement GetPathsRoot()
            {
                var path = Path.Combine(GetWindowerPath(), "paths.xml");
                try
                {
                    return XDocument.Load(path).Element("paths");
                }
                catch (FileNotFoundException) { }
                return new XElement("paths");
            }
        }
    }
}
