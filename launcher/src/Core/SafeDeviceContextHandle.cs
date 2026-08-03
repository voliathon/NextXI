namespace Windower.Core
{
    using System;
    using System.Runtime.InteropServices;
    internal class SafeDeviceContextHandle : SafeHandle
    {
        public SafeDeviceContextHandle()
            : base(IntPtr.Zero, true)
        { }
        public override bool IsInvalid
        {
            get => handle == IntPtr.Zero;
        }
        protected override bool ReleaseHandle() => NativeMethods.ReleaseDC(IntPtr.Zero, handle) == 1;
    }
}