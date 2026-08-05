namespace Boiler
{
    using System;

    public interface IGame : IEquatable<IGame>
    {
        bool IsSteamGame { get; }

        long AppId { get; }

        string Name { get; }

        string InstallDirectory { get; }
    }
}