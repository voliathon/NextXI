namespace Windower
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    public struct Maybe<T>
    {
        public Maybe(T value)
        {
            Value = value;
            HasValue = true;
        }

        public T Value { get; }

        public bool HasValue { get; }

        public bool Equals(Maybe<T> other) => this == other;

        public bool Equals(T other) => this == other;

        public override bool Equals(object obj)
        {
            if (obj is Maybe<T> other)
            {
                return this == other;
            }
            else if (obj is T)
            {
                return this == (T)obj;
            }

            return false;
        }

        public override int GetHashCode() => HasValue.GetHashCode() * 17 + (HasValue ? Value?.GetHashCode() ?? -1 : 0);

        public static bool operator ==(Maybe<T> left, T right) => left.HasValue && Equals(left.Value, right);

        public static bool operator !=(Maybe<T> left, T right) => !(left == right);

        public static bool operator ==(T left, Maybe<T> right) => right.HasValue && Equals(left, right.Value);

        public static bool operator !=(T left, Maybe<T> right) => !(left == right);

        public static bool operator ==(Maybe<T> left, Maybe<T> right) =>
            !left.HasValue && !right.HasValue || left.HasValue && right.HasValue && Equals(left.Value, right.Value);

        public static bool operator !=(Maybe<T> left, Maybe<T> right) => !(left == right);

        [SuppressMessage("Microsoft.Usage", "CA2225:OperatorOverloadsHaveNamedAlternates")]
        public static implicit operator Maybe<T>(T value) => new Maybe<T>(value);

        public T Default() => HasValue ? Value : default(T);

        public T Default(T defaultValue) => HasValue ? Value : defaultValue;

        [SuppressMessage("Microsoft.Design", "CA1006:DoNotNestGenericTypesInMemberSignatures")]
        public Maybe<TResult> Bind<TResult>(Func<T, Maybe<TResult>> apply)
        {
            if (apply == null)
            {
                throw new ArgumentNullException(nameof(apply));
            }

            return HasValue ? apply(Value) : default(Maybe<TResult>);
        }

        public Maybe<TResult> Bind<TResult>(Func<T, TResult> apply)
        {
            if (apply == null)
            {
                throw new ArgumentNullException(nameof(apply));
            }

            return HasValue ? new Maybe<TResult>(apply(Value)) : default(Maybe<TResult>);
        }
    }
}