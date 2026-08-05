namespace Windower
{
    using CommandLine;

    [Verb("get-args")]
    public class GetArgsOptions
    {
        [Value(0, MetaName = "name", Required = true)]
        public string ProfileName { get; set; }
    }
}