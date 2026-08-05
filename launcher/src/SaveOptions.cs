namespace Windower
{
    using CommandLine;
    using Core;
    using System.Diagnostics.CodeAnalysis;

    [Verb("save")]
    public class SaveOptions : ProfileOptions
    {
        Maybe<string> baseProfileName;

        [Value(0, MetaName = "name", Required = true)]
        public string ProfileName { get; set; }

        [Option("overwrite")]
        public bool Overwrite { get; set; }

        [Option("based-on")]
        public string BaseProfileName
        {
            get => baseProfileName.Default(null);
            set => baseProfileName = string.IsNullOrWhiteSpace(value) ? new Maybe<string>() : value.Trim();
        }

        [SuppressMessage("Microsoft.Design", "CA1024")]
        public Profile GetProfile() => GetProfile(baseProfileName.Bind(x => Launcher.ProfileManager[x]).Default(Profile.Default))
                .With(Name: ProfileName);
    }
}