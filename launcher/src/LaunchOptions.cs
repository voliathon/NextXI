namespace Windower
{
    using CommandLine;
    using Core;
    using System.Diagnostics.CodeAnalysis;

    [Verb("launch")]
    public class LaunchOptions : ProfileOptions
    {
        Maybe<string> baseProfileName;

        [Option("no-gui")]
        public bool NoGui { get; set; }

        [Value(0, MetaName = "profile")]
        public string BaseProfileName
        {
            get => baseProfileName.Default(null);
            set => baseProfileName = string.IsNullOrWhiteSpace(value) ? new Maybe<string>() : value.Trim();
        }

        [SuppressMessage("Microsoft.Design", "CA1024")]
        public Profile GetProfile() =>
            GetProfile(baseProfileName.Bind(x =>
                {
                    if (NoGui)
                    {
                        return Launcher.ProfileManager[x];
                    }

                    return Launcher.ProfileManager.TryGetValue(x, out var profile) ? profile : new Maybe<Profile>();
                }).Default());
    }
}