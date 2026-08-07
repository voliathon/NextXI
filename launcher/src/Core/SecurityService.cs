// Handles UAC elevation requirements, filesystem write verification, Windows AppCompat flags, and directory ACL access rules.
namespace Windower.Core
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Security.AccessControl;
    using System.Security.Principal;
    using System.Threading;
    using System.Threading.Tasks;
    using Boiler;
    using Microsoft.Win32;
    using Windower.PlayOnline;

    public static class SecurityService
    {
        public static bool IsAdministrator()
        {
            var identity = WindowsIdentity.GetCurrent();
            var principal = new WindowsPrincipal(identity);
            return principal.IsInRole(WindowsBuiltInRole.Administrator);
        }

        public static bool IsElevationRequired(Profile profile)
        {
            profile = GamePathResolver.Resolve(profile);
            if (!IsAdministrator())
            {
                if (profile.RunAsAdmin)
                {
                    return true;
                }

                if (!((PlayOnline.Region)profile.Region).IsInstalled())
                {
                    return true;
                }

                var directories = GamePathResolver.GetDirectories(profile);
                return !directories.Any() || !directories.All(CanWrite);
            }
            return false;
        }

        public static bool CheckPermissions(Profile profile)
        {
            if (!IsElevationRequired(profile))
            {
                foreach (var path in GamePathResolver.GetPaths(profile))
                {
                    SetAsInvokerFlag(path);
                }
                return true;
            }
            return false;
        }

        public static void FixAccessControl(Profile profile) => FixAccessControlAsync(profile).Wait();

        public static Task FixAccessControlAsync(Profile profile)
        {
            profile = GamePathResolver.Resolve(profile);
            return FixAccessControlAsync(GamePathResolver.GetDirectories(profile).Where(p => !CanWrite(p)).ToArray());
        }

        [RemoteCallable]
        public static void FixAccessControl(string[] paths, CancellationToken token) => FixAccessControlAsync(paths).Wait();

        public static async Task FixAccessControlAsync(string[] paths)
        {
            if (IsAdministrator())
            {
                foreach (var p in paths)
                {
                    SetAccessControl(p);
                }
            }
            else
            {
                await Program.ElevateAsync(FixAccessControl, paths.ToArray(), CancellationToken.None);
            }
        }

        public static bool CanWrite(string path)
        {
            try
            {
                while (true)
                {
                    try
                    {
                        var tempPath = Path.Combine(path, Path.GetRandomFileName());
                        using (new FileStream(tempPath, FileMode.CreateNew, FileAccess.ReadWrite, FileShare.None, 1, FileOptions.DeleteOnClose))
                        { }
                        return true;
                    }
                    catch (IOException e)
                    {
                        if (e.HResult != unchecked((int)0x80070050)) // ERROR_FILE_EXISTS
                        {
                            throw;
                        }
                    }
                }
            }
            catch (UnauthorizedAccessException) { }
            return false;
        }

        public static void SetAsInvokerFlag(string path)
        {
            using (var hkcu = RegistryKey.OpenBaseKey(RegistryHive.CurrentUser, RegistryView.Registry64))
            using (var key = hkcu.CreateSubKey(@"SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers"))
            {
                var oldValue = key.GetValue(path)?.ToString() ?? "";

                var flags =
                    from f in oldValue.Split((char[])null, StringSplitOptions.RemoveEmptyEntries)
                    where f != "RUNASADMINISTRATOR" && f != "RUNASINVOKER"
                    select f;
                flags = flags.Concat(new[] { "RUNASINVOKER" });

                var newValue = string.Join(" ", flags);

                if (newValue != oldValue)
                {
                    key.SetValue(path, newValue, RegistryValueKind.String);
                }
            }
        }

        public static void SetAccessControl(string path)
        {
            if (!string.IsNullOrWhiteSpace(path))
            {
                var info = new DirectoryInfo(path.Trim());
                var security = info.GetAccessControl();
                var rule = new FileSystemAccessRule(
                    new SecurityIdentifier(WellKnownSidType.BuiltinUsersSid, null),
                    FileSystemRights.Modify | FileSystemRights.DeleteSubdirectoriesAndFiles,
                    InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit,
                    PropagationFlags.None,
                    AccessControlType.Allow);

                security.AddAccessRule(rule);
                info.SetAccessControl(security);
            }
        }
    }
}
