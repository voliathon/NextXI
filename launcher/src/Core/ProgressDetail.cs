namespace Windower.Core
{
    using System.Collections.Generic;

    public struct ProgressDetail
    {
        public static ProgressDetail Create(long progress, long total) => new ProgressDetail(progress, total);

        public static ProgressDetail Create<T>(ProgressDetail<T> detail) => new ProgressDetail(detail.Progress, detail.Total);

        public static ProgressDetail<T> Create<T>(T status) => new ProgressDetail<T>(0, 0, status);

        public static ProgressDetail<T> Create<T>(long progress, long total, T status) =>
            new ProgressDetail<T>(progress, total, status);

        public static ProgressDetail<T> Create<T>(ProgressDetail detail, T status) =>
            new ProgressDetail<T>(detail.Progress, detail.Total, status);

        public static ProgressDetail<T> Create<T, TOther>(ProgressDetail<TOther> detail, T status) =>
            new ProgressDetail<T>(detail.Progress, detail.Total, status);

        public ProgressDetail(long progress, long total)
        {
            Progress = progress;
            Total = total;
        }

        public long Progress { get; }

        public long Total { get; }

        public bool Equals(ProgressDetail other) => this == other;

        public override bool Equals(object obj) => obj is ProgressDetail other && Equals(other);

        public override int GetHashCode() => Progress.GetHashCode() * 17 + Total.GetHashCode();

        public static bool operator ==(ProgressDetail left, ProgressDetail right) =>
            left.Progress == right.Progress && left.Total == right.Total;

        public static bool operator !=(ProgressDetail left, ProgressDetail right) => !(left == right);
    }

    public struct ProgressDetail<T>
    {
        public ProgressDetail(long progress, long total, T status)
        {
            Progress = progress;
            Total = total;
            Status = status;
        }

        public long Progress { get; }

        public long Total { get; }

        public T Status { get; }

        public bool Equals(ProgressDetail<T> other) => this == other;

        public override bool Equals(object obj) => obj is ProgressDetail<T> other && Equals(other);

        public override int GetHashCode()
        {
            var result = Progress.GetHashCode();
            result = result * 17 + Total.GetHashCode();
            result = result * 17 + Status.GetHashCode();
            return result;
        }

        public static bool operator ==(ProgressDetail<T> left, ProgressDetail<T> right) =>
            left.Progress == right.Progress && left.Total == right.Total &&
            EqualityComparer<T>.Default.Equals(left.Status, right.Status);

        public static bool operator !=(ProgressDetail<T> left, ProgressDetail<T> right) => !(left == right);
    }
}