namespace Windower.Core
{
    using System;
    using System.Collections.Immutable;
    using System.Diagnostics.CodeAnalysis;

    public struct DisplayDevice : IEquatable<DisplayDevice>
    {
        public DisplayDevice(string deviceName, string friendlyName, bool isPrimary, Dimension currentResolution,
            Point currentPosition, int currentDpi, IImmutableList<Dimension> availableResolutions)
        {
            DeviceName = deviceName;
            FriendlyName = friendlyName;
            IsPrimary = isPrimary;
            CurrentResolution = currentResolution;
            CurrentPosition = currentPosition;
            CurrentDpi = currentDpi;
            AvailableResolutions = availableResolutions ?? ImmutableArray<Dimension>.Empty.Add(CurrentResolution);
        }

        public string DeviceName { get; }
        public string FriendlyName { get; }
        public bool IsPrimary { get; }
        public Dimension CurrentResolution { get; }
        public Point CurrentPosition { get; }
        public IImmutableList<Dimension> AvailableResolutions { get; }
        public int CurrentDpi { get; }

        [SuppressMessage("Microsoft.Design", "CA1006")]
        [SuppressMessage("Microsoft.Design", "CA1026")]
        [SuppressMessage("Microsoft.Naming", "CA1709")]
        public DisplayDevice With(
            Maybe<string> DeviceName = new Maybe<string>(),
            Maybe<string> FriendlyName = new Maybe<string>(),
            Maybe<bool> IsPrimary = new Maybe<bool>(),
            Maybe<Dimension> CurrentResolution = new Maybe<Dimension>(),
            Maybe<Point> CurrentPosition = new Maybe<Point>(),
            Maybe<int> CurrentDpi = new Maybe<int>(),
            Maybe<IImmutableList<Dimension>> AvailableResolutions = new Maybe<IImmutableList<Dimension>>())
        {
            if (DeviceName != this.DeviceName || FriendlyName != this.FriendlyName || IsPrimary != this.IsPrimary ||
                CurrentResolution != this.CurrentResolution || CurrentPosition != this.CurrentPosition ||
                CurrentDpi != this.CurrentDpi || AvailableResolutions != this.AvailableResolutions)
            {
                return new DisplayDevice(
                    DeviceName.Default(this.DeviceName),
                    FriendlyName.Default(this.FriendlyName),
                    IsPrimary.Default(this.IsPrimary),
                    CurrentResolution.Default(this.CurrentResolution),
                    CurrentPosition.Default(this.CurrentPosition),
                    CurrentDpi.Default(this.CurrentDpi),
                    AvailableResolutions.Default(this.AvailableResolutions));
            }

            return this;
        }

        public static bool operator ==(DisplayDevice left, DisplayDevice right) => left.DeviceName == right.DeviceName;

        public static bool operator !=(DisplayDevice left, DisplayDevice right) => !(left == right);

        public bool Equals(DisplayDevice other) => DeviceName == other.DeviceName;

        public override bool Equals(object obj) => obj is DisplayDevice other && Equals(other);

        public override int GetHashCode() => (DeviceName ?? string.Empty).GetHashCode();
    }
}