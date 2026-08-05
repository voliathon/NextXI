namespace Windower.Core
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    [Serializable]
    public struct Dimension : IEquatable<Dimension>
    {
        public Dimension(int width, int height)
        {
            Width = width;
            Height = height;
        }

        public int Width { get; }

        public int Height { get; }

        [SuppressMessage("Microsoft.Design", "CA1026")]
        [SuppressMessage("Microsoft.Naming", "CA1709")]
        public Dimension With(
            Maybe<int> Width = new Maybe<int>(),
            Maybe<int> Height = new Maybe<int>())
        {
            if (Width != this.Width || Height != this.Height)
            {
                return new Dimension(
                    Width.Default(this.Width),
                    Height.Default(this.Height));
            }

            return this;
        }

        public static bool operator ==(Dimension left, Dimension right) =>
            left.Width == right.Width && left.Height == right.Height;

        public static bool operator !=(Dimension left, Dimension right) => !(left == right);

        public bool Equals(Dimension other) => this == other;

        public override bool Equals(object obj) => obj is Dimension other && Equals(other);

        public override int GetHashCode() => Width.GetHashCode() * 17 + Height.GetHashCode();
    }
}