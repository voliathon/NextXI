namespace Windower.Core
{
    using System;
    using System.Runtime.InteropServices;
    using Microsoft.Win32.SafeHandles;
    internal class SafeProcessMemoryHandle : SafeHandle
    {
        private readonly SafeWaitHandle processHandle;
        public SafeProcessMemoryHandle(SafeWaitHandle invalidHandleValue, uint size)
            : base(IntPtr.Zero, true)
        {
            processHandle = invalidHandleValue;
            handle = NativeMethods.VirtualAllocEx(processHandle, IntPtr.Zero, new UIntPtr(size),
                NativeMethods.MEM_COMMIT | NativeMethods.MEM_RESERVE, NativeMethods.PAGE_READWRITE);
        }
        public override bool IsInvalid
        {
            get => handle == IntPtr.Zero;
        }
        protected override bool ReleaseHandle() =>
            NativeMethods.VirtualFreeEx(processHandle, handle, UIntPtr.Zero, NativeMethods.MEM_RELEASE);
    }
}