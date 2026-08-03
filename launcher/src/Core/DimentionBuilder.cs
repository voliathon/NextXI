namespace Windower.Core
{
    using System;

    public class DimensionBuilder
    {
        private Maybe<int> width;
        private Maybe<int> height;

        public int Width
        {
            get => width.Default(0);
            set => width = value;
        }

        public int Height
        {
            get => height.Default(0);
            set => height = value;
        }

        public Dimension Get() => Get(new Dimension());

        public Dimension Get(Dimension baseValue)
        {
            return baseValue.With(width, height);
        }
    }
}