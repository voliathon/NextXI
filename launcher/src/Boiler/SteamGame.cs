using System;

namespace Boiler
{
    public struct SteamGame : IGame, IEquatable<SteamGame>
    {
        public SteamGame(long appId, string name, string installDirectory)
        {
            AppId = appId;
            Name = name;
            InstallDirectory = installDirectory;
        }

        public bool IsSteamGame => true;

        public long AppId { get; }

        public string Name { get; }

        public string InstallDirectory { get; }

        public static bool operator ==(SteamGame left, SteamGame right) => left.AppId == right.AppId;

        public static bool operator !=(SteamGame left, SteamGame right) => !(left == right);

        public static bool operator ==(SteamGame left, IGame right) => left.AppId == right?.AppId;

        public static bool operator !=(SteamGame left, IGame right) => !(left == right);

        public static bool operator ==(IGame left, SteamGame right) => left?.AppId == right.AppId;

        public static bool operator !=(IGame left, SteamGame right) => !(left == right);

        public bool Equals(SteamGame other) => AppId == other.AppId;

        public bool Equals(IGame other) => AppId == other?.AppId;

        public override bool Equals(object obj)
        {
            if (obj is SteamGame temp1)
            {
                return Equals(temp1);
            }
            else if (obj is IGame temp2)
            {
                return Equals(temp2);
            }

            return false;
        }

        public override int GetHashCode() => AppId.GetHashCode();
    }
}