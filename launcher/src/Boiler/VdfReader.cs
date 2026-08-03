namespace Boiler
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.Text;

    [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "Vdf")]
    public static class VdfReader
    {
        public static IVdfValue Load(Stream input) => Load(input, Encoding.UTF8);

        public static IVdfValue Load(Stream input, Encoding encoding)
        {
            if (input == null)
            {
                throw new ArgumentNullException(nameof(input));
            }

            var value = input.ReadByte();
            input.Seek(-1, SeekOrigin.Current);
            switch (value)
            {
                case -1: throw new EndOfStreamException();
                case 0:
                case 1:
                case 2:
                    return BinaryVdfReader.Load(input, encoding);
                default:
                    return TextVdfReader.Load(input, encoding);
            }
        }
    }
}