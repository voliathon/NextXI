namespace Windower.UI
{
    using System;
    using System.Runtime.InteropServices;
    public class SafeIconHandle : SafeHandle
    {
        public SafeIconHandle() :
            this(IntPtr.Zero, true)
        { }
        public SafeIconHandle(IntPtr handle) :
            this(handle, true)
        { }
        public SafeIconHandle(IntPtr handle, bool ownsHandle) :
            base(IntPtr.Zero, ownsHandle) => this.handle = handle;
        public override bool IsInvalid
        {
            get => handle == IntPtr.Zero;
        }
        protected override bool ReleaseHandle() => NativeMethods.DestroyIcon(handle);
    }
}