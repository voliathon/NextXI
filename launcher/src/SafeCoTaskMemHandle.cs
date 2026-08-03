namespace Windower
{
    using System;
    using System.Runtime.InteropServices;
    internal class SafeCoTaskMemHandle : SafeHandle
    {
        public SafeCoTaskMemHandle()
            : base(IntPtr.Zero, true)
        { }
        public override bool IsInvalid
        {
            get => handle == IntPtr.Zero;
        }
        protected override bool ReleaseHandle()
        {
            Marshal.FreeCoTaskMem(handle);
            return true;
        }
    }
}