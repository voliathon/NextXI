namespace Windower.Core
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.Threading;
    using System.Threading.Tasks;

    public static class ExtensionMethods
    {
        [SuppressMessage("Microsoft.Design", "CA1006")]
        public static void Report(this IProgress<ProgressDetail> progress)
        {
            if (progress == null)
            {
                throw new ArgumentNullException(nameof(progress));
            }

            progress.Report(new ProgressDetail());
        }

        [SuppressMessage("Microsoft.Design", "CA1006")]
        public static void Report(this IProgress<ProgressDetail> progress, long current, long total)
        {
            if (progress == null)
            {
                throw new ArgumentNullException(nameof(progress));
            }

            progress.Report(ProgressDetail.Create(current, total));
        }

        [SuppressMessage("Microsoft.Design", "CA1006")]
        public static void Report<T>(this IProgress<ProgressDetail<T>> progress, T status)
        {
            if (progress == null)
            {
                throw new ArgumentNullException(nameof(progress));
            }

            progress.Report(ProgressDetail.Create(status));
        }

        [SuppressMessage("Microsoft.Design", "CA1006")]
        public static void Report<T>(this IProgress<ProgressDetail<T>> progress, long current, long total, T status)
        {
            if (progress == null)
            {
                throw new ArgumentNullException(nameof(progress));
            }

            progress.Report(ProgressDetail.Create(current, total, status));
        }

        [SuppressMessage("Microsoft.Design", "CA1006")]
        public static IProgress<ProgressDetail<T>> Wrap<T>(this IProgress<ProgressDetail> progress)
        {
            if (progress == null)
            {
                throw new ArgumentNullException(nameof(progress));
            }

            return new Progress<ProgressDetail<T>>(d => progress.Report(ProgressDetail.Create(d)));
        }

        [SuppressMessage("Microsoft.Design", "CA1006")]
        public static IProgress<ProgressDetail> Wrap<TBase>(this IProgress<ProgressDetail<TBase>> progress, TBase status)
        {
            if (progress == null)
            {
                throw new ArgumentNullException(nameof(progress));
            }

            return new Progress<ProgressDetail>(d => progress.Report(ProgressDetail.Create(d, status)));
        }

        [SuppressMessage("Microsoft.Design", "CA1006")]
        public static IProgress<ProgressDetail<T>> Wrap<T, TBase>(this IProgress<ProgressDetail<TBase>> progress, TBase status)
        {
            if (progress == null)
            {
                throw new ArgumentNullException(nameof(progress));
            }

            return new Progress<ProgressDetail<T>>(d => progress.Report(ProgressDetail.Create(d, status)));
        }

        [SuppressMessage("Microsoft.Design", "CA1006")]
        public static IProgress<ProgressDetail<T>> Wrap<T, TBase>(this IProgress<ProgressDetail<TBase>> progress,
            Func<T, TBase> map)
        {
            if (progress == null)
            {
                throw new ArgumentNullException(nameof(progress));
            }

            if (map == null)
            {
                throw new ArgumentNullException(nameof(map));
            }

            return new Progress<ProgressDetail<T>>(d => progress.Report(ProgressDetail.Create(d, map(d.Status))));
        }

        public static async Task CopyToAsync(this Stream source, Stream destination, IProgress<long> progress,
            CancellationToken cancellationToken = default(CancellationToken), int bufferSize = 0x1000)
        {
            var buffer = new byte[bufferSize];
            int bytesRead;
            long totalRead = 0;
            while ((bytesRead = await source.ReadAsync(buffer, 0, buffer.Length, cancellationToken)) > 0)
            {
                await destination.WriteAsync(buffer, 0, bytesRead, cancellationToken);
                cancellationToken.ThrowIfCancellationRequested();
                totalRead += bytesRead;
                progress.Report(totalRead);
            }
        }
    }
}