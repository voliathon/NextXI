namespace Boiler
{
    using Microsoft.Win32.SafeHandles;
    using System;
    using System.Runtime.InteropServices;
    using System.Security;

    internal static class NativeMethods
    {
        internal const uint EVENT_OBJECT_CREATE = 0x8000;

        internal const uint WINEVENT_OUTOFCONTEXT = 0x0000;
        internal const uint WINEVENT_SKIPOWNPROCESS = 0x0002;

        internal const uint PM_REMOVE = 1;

        internal const uint INFINITE = 0xFFFFFFFF;

        internal const uint QS_ALLINPUT = 0x04FF;

        internal const uint WAIT_OBJECT_0 = 0x00000000;
        internal const uint WAIT_ABANDONED = 0x00000080;
        internal const uint WAIT_TIMEOUT = 0x00000102;
        internal const uint WAIT_FAILED = 0xFFFFFFFF;

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        internal static extern SafeWinEventHookHandle SetWinEventHook(uint eventMin, uint eventMax, IntPtr hmodWinEventProc,
            WinEventProc lpfnWinEventProc, uint idProcess, uint idThread, uint dwflags);

        [SuppressUnmanagedCodeSecurity]
        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool UnhookWinEvent(IntPtr hWinEventHook);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool PeekMessage(out MSG lpMsg, IntPtr hWnd, uint wMsgFilterMin, uint wMsgFilterMax,
            uint wRemoveMsg);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool TranslateMessage(ref MSG lpMsg);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern IntPtr DispatchMessage(ref MSG lpmsg);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern uint MsgWaitForMultipleObjects(uint nCount, [In] ref SafeWaitHandle pHandles,
            [MarshalAs(UnmanagedType.Bool)] bool bWaitAll, uint dwMilliseconds, uint dwWakeMask);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        internal static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint lpdwProcessId);

        [StructLayout(LayoutKind.Sequential)]
        internal struct MSG
        {
            public IntPtr hwnd;
            public uint message;
            public IntPtr wParam;
            public IntPtr lParam;
            public uint time;
            public POINT pt;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct POINT
        {
            public int x;
            public int y;
        }

        [UnmanagedFunctionPointer(CallingConvention.Winapi)]
        internal delegate void WinEventProc(IntPtr hWinEventHook, uint @event, IntPtr hwnd, int idObject, int idChild,
            uint dwEventThread, uint dwmsEventTime);
    }
}