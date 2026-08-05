namespace Windower.Core
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    public class PointBuilder
    {
        private Maybe<int> x;
        private Maybe<int> y;

        [SuppressMessage("Microsoft.Naming", "CA1704")]
        public int X
        {
            get => x.Default(0);
            set => x = value;
        }

        [SuppressMessage("Microsoft.Naming", "CA1704")]
        public int Y
        {
            get => y.Default(0);
            set => y = value;
        }

        public Point Get() => Get(new Point());

        public Point Get(Point baseValue)
        {
            return baseValue.With(x, y);
        }
    }
}