namespace Windower.PlayOnline
{
    using System;
    using System.Collections.Immutable;
    using System.Linq;

    internal static class ClientInfo
    {
        public static IImmutableList<Region> InstalledRegions { get; } =
            Enum.GetValues(typeof(Region)).Cast<Region>().Where(r => r.IsInstalled()).ToImmutableArray();

        public static IImmutableList<Region> OwnedRegions { get; } =
            Enum.GetValues(typeof(Region)).Cast<Region>().Where(r => r.IsOwned()).ToImmutableArray();
    }
}