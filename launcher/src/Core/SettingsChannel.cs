namespace Windower.Core
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.IO;
    using System.IO.MemoryMappedFiles;
    using System.Linq;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Xml.Linq;

    using static System.FormattableString;
    public class SettingsChannel : IDisposable
    {
        private bool disposed = false;
        private MemoryMappedFile file;
        private EventWaitHandle flag;
        [SuppressMessage("Microsoft.Design", "CA1006")]
        public SettingsChannel(Process process, IEnumerable<KeyValuePair<string, object>> settings)
        {
            if (process == null)
            {
                throw new ArgumentNullException(nameof(process));
            }

            if (settings == null)
            {
                throw new ArgumentNullException(nameof(settings));
            }

            var dataName = Invariant($"Windower.Settings[{process.Id:X8}].Data");
            var flagName = Invariant($"Windower.Settings[{process.Id:X8}].Flag");

            var document = new XDocument(
                new XElement("settings",
                    from pair in settings
                    select new XElement(pair.Key, Unwrap(pair.Value))));

            using (var buffer = new MemoryStream())
            {
                document.Save(buffer, SaveOptions.DisableFormatting);

                file = MemoryMappedFile.CreateNew(dataName, buffer.Length + sizeof(long));
                using (var stream = file.CreateViewStream(0, 0))
                {
                    stream.Write(BitConverter.GetBytes(buffer.Length), 0, sizeof(long));
                    buffer.WriteTo(stream);
                }
            }

            flag = new EventWaitHandle(false, EventResetMode.ManualReset, flagName);
        }
        public Task FinishAsync(CancellationToken token) =>
            Task.Run(() =>
            {
                do
                {
                    token.ThrowIfCancellationRequested();
                }
                while (!flag.WaitOne(100));
            }, token);
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }
        protected virtual void Dispose(bool disposing)
        {
            if (!disposed && disposing)
            {
                file.Dispose();
                flag.Dispose();
            }

            disposed = true;
        }

        private static object Unwrap(object value)
        {
            if (value is Enum e)
            {
                return Convert.ChangeType(e, Enum.GetUnderlyingType(e.GetType()), CultureInfo.InvariantCulture);
            }

            return value;
        }
    }
}