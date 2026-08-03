namespace Windower.Core
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    public struct Point : IEquatable<Point>
    {
        [SuppressMessage("Microsoft.Naming", "CA1704")]
        public Point(int x, int y)
        {
            X = x;
            Y = y;
        }

        [SuppressMessage("Microsoft.Naming", "CA1704")]
        public int X { get; }

        [SuppressMessage("Microsoft.Naming", "CA1704")]
        public int Y { get; }

        [SuppressMessage("Microsoft.Design", "CA1026")]
        [SuppressMessage("Microsoft.Naming", "CA1704")]
        [SuppressMessage("Microsoft.Naming", "CA1709")]
        public Point With(
            Maybe<int> X = new Maybe<int>(),
            Maybe<int> Y = new Maybe<int>())
        {
            if (X != this.X || Y != this.Y)
            {
                return new Point(
                    X.Default(this.X),
                    Y.Default(this.Y));
            }

            return this;
        }

        public static bool operator ==(Point left, Point right) => left.X == right.X && left.Y == right.Y;

        public static bool operator !=(Point left, Point right) => !(left == right);

        public bool Equals(Point other) => this == other;

        public override bool Equals(object obj) => obj is Point other && Equals(other);

        public override int GetHashCode() => X.GetHashCode() * 17 + Y.GetHashCode();
    }
}