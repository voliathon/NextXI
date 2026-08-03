namespace Windower
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.Threading.Tasks;

    internal static class Utilities
    {
        [SuppressMessage("Microsoft.Reliability", "CA2000")]
        public static FileStream CreateTimestampedFileStream(string directory, string extension) =>
            CreateTimestampedFileStreamAsync(directory, extension).GetAwaiter().GetResult();

        public static async Task<FileStream> CreateTimestampedFileStreamAsync(string directory, string extension)
        {
            Directory.CreateDirectory(directory);
            while (true)
            {
                var filename = Path.ChangeExtension(DateTime.UtcNow.ToString("yyyy-MM-ddTHH-mm-ss-fffffffK"), extension);
                var path = Path.Combine(directory, filename);
                try
                {
                    return new FileStream(path, FileMode.CreateNew, FileAccess.Write, FileShare.Read | FileShare.Delete);
                }
                catch (IOException) when (File.Exists(path)) { }
                await Task.Yield();
            }
        }
    }
}