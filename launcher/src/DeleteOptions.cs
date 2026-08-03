namespace Windower
{
    using CommandLine;

    [Verb("delete")]
    public class DeleteOptions
    {
        [Value(0, MetaName = "name", Required = true)]
        public string ProfileName { get; set; }
    }
}