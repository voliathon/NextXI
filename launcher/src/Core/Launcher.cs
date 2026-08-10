// The main launcher class is now focused purely on launch orchestration, process creation, resource checking, and DLL injection.
namespace Windower.Core
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Runtime.ExceptionServices;
    using System.Threading;
    using System.Threading.Tasks;
    using Boiler;
    using PlayOnline;

    public static class Launcher
    {
        public static string CorePath => GamePathResolver.GetCorePath();

        public static ProfileManager ProfileManager { get; } = new ProfileManager();

        [RemoteCallable]
        public static void Launch(Profile profile, CancellationToken token)
        {
            try
            {
                LaunchAsync(profile, null, token).Wait();
            }
            catch (AggregateException e)
            {
                ExceptionDispatchInfo.Capture(e.Flatten().InnerException).Throw();
            }
        }

        public static async Task LaunchAsync(Profile profile, IProgress<ProgressDetail<LaunchStatus>> progress, CancellationToken token)
        {
            profile = GamePathResolver.Resolve(profile);
            if (SecurityService.CheckPermissions(profile))
            {
                Process process = null;
                try
                {
                    var status = profile.Region?.IsInstalled() == true ? LaunchStatus.Launching : LaunchStatus.Installing;
                    progress?.Report(ProgressDetail.Create(status));

                    var region = (Region)profile.Region;
                    GraphicsUpdater.ApplyGraphicsEngine(profile.SelectedEngine, region.GetPOLInstallDirectory());

                    // Removed the double GetDirectoryName so the files land in the right spot!
                    await ResourceManager.CheckAndDownloadResourcesAsync(Path.GetDirectoryName(CorePath), progress);

                    using (var injector = await CreateInjectorAsync(profile, token))
                    {
                        process = injector.Process;

                        using (var settings = new SettingsChannel(injector.Process, profile.Settings))
                        {
                            await injector.Inject(Path.Combine(Path.GetDirectoryName(CorePath), "lua51.dll"));
                            await injector.Inject(CorePath);
                            injector.ResumeProcess();
                            progress?.Report(ProgressDetail.Create(LaunchStatus.TransferringSettings));
                            await settings.FinishAsync(token);
                        }
                    }

                    progress?.Report(ProgressDetail.Create(LaunchStatus.WaitingForWindow));
                    try
                    {
                        _ = process.WaitForInputIdle();
                        if (!Program.IsMono)
                        {
                            while (process.MainWindowHandle == IntPtr.Zero)
                            {
                                await Task.Delay(100, token);
                                process.Refresh();
                            }
                        }
                    }
                    catch (InvalidOperationException) { }

                    process = null;
                }
                catch (OperationCanceledException)
                {
                    if (process != null)
                    {
                        try
                        {
                            process.Kill();
                        }
                        catch (Win32Exception) { }
                        catch (InvalidOperationException) { }
                    }
                    throw;
                }
                finally
                {
                    process?.Dispose();
                }
            }
            else
            {
                var status = profile.Region?.IsInstalled() == true ? LaunchStatus.Elevating : LaunchStatus.Installing;
                progress?.Report(ProgressDetail.Create(status));
                await Program.ElevateAsync(Launch, profile, token);
            }

            progress?.Report(ProgressDetail.Create(1, 1, LaunchStatus.Complete));
        }

        private static async Task<Injector> CreateInjectorAsync(Profile profile, CancellationToken token)
        {
            var region = (Region)profile.Region;
            if (profile.UseSteam && Steam.IsInstalled)
            {
                var appId = region.GetInstalledSteamAppIds().Cast<long?>().FirstOrDefault();
                if (appId != null)
                {
                    var installDirs =
                        from r in ClientInfo.InstalledRegions
                        let dir = r.GetPOLInstallDirectory()
                        where !string.IsNullOrEmpty(dir)
                        select dir;

                    var steamDirs =
                        from r in ClientInfo.InstalledRegions
                        from id in r.GetInstalledSteamAppIds()
                        let path = Steam.GetInstalledGame(id)?.InstallDirectory
                        where path != null
                        select path;

                    installDirs = installDirs.Union(steamDirs).ToList();

                    var process = Steam.LaunchAsync((long)appId, p =>
                    {
                        if (installDirs.Any())
                        {
                            var executable = p.MainModule.FileName;
                            return installDirs.Any(path => executable.StartsWith(path));
                        }
                        return false;
                    }, token);

                    if (!installDirs.Any())
                    {
                        process = Task.WhenAll(process, WaitForInstallAsync(token))
                            .ContinueWith(t => process.IsCompleted ? process.Result : null, token);
                    }

                    var temp = await process;
                    token.ThrowIfCancellationRequested();
                    return new Injector(temp);
                }

                appId = region.GetOwnedSteamAppIds().Cast<long?>().FirstOrDefault();
                if (appId != null)
                {
                    return new Injector(await Steam.LaunchAsync((long)appId,
                        (p) => Path.GetFileName(p.MainModule.FileName).Equals("pol.exe", StringComparison.OrdinalIgnoreCase), token));
                }
            }
            else if (profile.Executable != null)
            {
                var args = profile.ExecutableArgs;
                args = args.Replace("{region}", profile.Region.ToString().ToLowerInvariant());
                args = args.Trim();
                return new Injector(profile.Executable, args);
            }
            else
            {
                var dir = region.GetPOLInstallDirectory();
                if (dir != null)
                {
                    return new Injector(Path.Combine(dir, "pol.exe"), "/game", "eAZcFcB");
                }
            }

            throw new InvalidOperationException();
        }

        private static async Task WaitForInstallAsync(CancellationToken token)
        {
            IEnumerable<string> installDirs;
            do
            {
                await Task.Delay(TimeSpan.FromSeconds(1), token);

                var dirs =
                    from r in ClientInfo.InstalledRegions
                    from id in r.GetInstalledSteamAppIds()
                    let path = Steam.GetInstalledGame(id)?.InstallDirectory
                    where path != null
                    select path;

                installDirs = dirs.ToList();
            }
            while (!installDirs.Any() && !token.IsCancellationRequested);
        }
    }
}
