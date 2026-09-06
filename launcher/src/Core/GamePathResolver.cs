// Resolves installation paths across standalone registry keys and Steam installations.
namespace Windower.Core
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using Boiler;
    using PlayOnline;

    public static class GamePathResolver
    {
        public static string GetCorePath()
        {
            string myPath = Assembly.GetExecutingAssembly().Location;
            return Path.Combine(Path.GetDirectoryName(myPath), "core.dll");
        }

        public static Profile Resolve(Profile profile)
        {
            if (profile.Region != null && ((Region)profile.Region).IsInstalled())
            {
                return profile;
            }

            if (!profile.UseSteam)
            {
                var region = ClientInfo.InstalledRegions.Cast<Region?>().FirstOrDefault(r => r?.GetPOLInstallDirectory() != null);
                if (region != null)
                {
                    return profile.With(Region: region);
                }
            }

            var steamRegion =
                ClientInfo.InstalledRegions.Cast<Region?>().FirstOrDefault(r => r?.GetInstalledSteamAppIds().Any() == true) ??
                Enum.GetValues(typeof(Region)).Cast<Region?>().FirstOrDefault(r => r?.GetOwnedSteamAppIds().Any() == true);

            return steamRegion != null ? profile.With(UseSteam: true, Region: steamRegion) : profile;
        }

        public static IList<string> GetDirectories(Profile profile)
        {
            if (profile.Region?.IsInstalled() == true)
            {
                var region = (Region)profile.Region;

                if (profile.UseSteam && region.GetInstalledSteamAppIds().Any())
                {
                    var polJp = Region.JP.GetPOLInstallDirectory();
                    var ffxiJp = Region.JP.GetFF11InstallDirectory();
                    if (polJp != null && ffxiJp != null)
                    {
                        return new List<string> { polJp, ffxiJp };
                    }

                    var polNa = Region.NA.GetPOLInstallDirectory();
                    var ffxiNa = Region.NA.GetFF11InstallDirectory();
                    if (polNa != null && ffxiNa != null)
                    {
                        return new List<string> { polNa, ffxiNa };
                    }
                }

                var pol = region.GetPOLInstallDirectory();
                var ffxi = region.GetFF11InstallDirectory();

                if (pol != null && ffxi != null)
                {
                    return new List<string> { pol, ffxi };
                }
            }

            return new List<string>();
        }

        public static IList<string> GetPaths(Profile profile)
        {
            var region = (Region)profile.Region;
            if (profile.UseSteam && region.GetInstalledSteamAppIds().Any())
            {
                var polJp = Region.JP.GetPOLInstallDirectory();
                var ffxiJp = Region.JP.GetFF11InstallDirectory();
                if (polJp != null && ffxiJp != null)
                {
                    return new List<string>
                    {
                        Path.Combine(polJp, "pol.exe"),
                        Path.Combine(polJp, "util", "startpol.exe"),
                        Path.Combine(ffxiJp, "polboot.exe"),
                    };
                }

                var polNa = Region.NA.GetPOLInstallDirectory();
                var ffxiNa = Region.NA.GetFF11InstallDirectory();
                if (polNa != null && ffxiNa != null)
                {
                    return new List<string>
                    {
                        Path.Combine(polNa, "pol.exe"),
                        Path.Combine(polNa, "util", "startpol.exe"),
                        Path.Combine(ffxiNa, "polboot.exe"),
                    };
                }

                var polReg = region.GetPOLInstallDirectory();
                var ffxiReg = region.GetFF11InstallDirectory();
                if (polReg != null && ffxiReg != null)
                {
                    return new List<string>
                    {
                        Path.Combine(polReg, "pol.exe"),
                        Path.Combine(polReg, "util", "startpol.exe"),
                        Path.Combine(ffxiReg, "polboot.exe"),
                    };
                }
            }

            var pol = region.GetPOLInstallDirectory();
            if (pol != null)
            {
                return new List<string>
                {
                    Path.Combine(pol, "pol.exe"),
                    Path.Combine(pol, "util", "startpol.exe"),
                };
            }

            return new List<string>();
        }
    }
}
