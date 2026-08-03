namespace Boiler
{
    using System.Collections.Generic;
    using System.Diagnostics.CodeAnalysis;

    [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "Vdf")]
    [SuppressMessage("Microsoft.Naming", "CA1710:IdentifiersShouldHaveCorrectSuffix")]
    public interface IVdfValue : IReadOnlyDictionary<string, IVdfValue>
    {
        string Value { get; }
        bool HasSubValues { get; }
    }
}