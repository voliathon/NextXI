namespace Boiler
{
    using System;
    using System.Runtime.InteropServices;
    internal sealed class SafeWinEventHookHandle : SafeHandle
    {
        public SafeWinEventHookHandle()
          : base(IntPtr.Zero, true)
        { }
        public override bool IsInvalid
        {
            get => handle == IntPtr.Zero;
        }
        protected override bool ReleaseHandle() => NativeMethods.UnhookWinEvent(handle);
    }
}