namespace Windower.Core
{
    using Microsoft.Win32.SafeHandles;
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;

    internal class Injector : IDisposable
    {
        private bool disposed = false;
        private Process process;
        private SafeWaitHandle processHandle;
        private SafeWaitHandle threadHandle;
        private string path;
        private bool initialized;

        private readonly System.Collections.Generic.List<SafeProcessMemoryHandle> memoryHandles = new System.Collections.Generic.List<SafeProcessMemoryHandle>();
        public Injector(string path, params string[] args) :
            this(path, EscapeArguments(args))
        { }
        public Injector(string path, string argString)
        {
            path = Path.GetFullPath(path ?? throw new ArgumentNullException(nameof(path)));
            argString = string.Join(" ", EscapeArgument(path), argString ?? string.Empty).Trim();

            this.path = path;

            var directory = Path.GetDirectoryName(path);
            var info = default(NativeMethods.PROCESS_INFORMATION);
            var startup = default(NativeMethods.STARTUPINFO);
            startup.cb = (uint)Marshal.SizeOf(typeof(NativeMethods.STARTUPINFO));

            try
            {
                if (!NativeMethods.CreateProcess(path, argString, IntPtr.Zero, IntPtr.Zero, false, NativeMethods.CREATE_SUSPENDED,
                    IntPtr.Zero, directory, ref startup, out info))
                {
                    throw new Win32Exception();
                }
            }
            finally
            {
                try
                {
                    processHandle = new SafeWaitHandle(info.hProcess, true);
                }
                finally
                {
                    threadHandle = new SafeWaitHandle(info.hThread, true);
                }
            }

            process = Process.GetProcessById((int)info.dwProcessId);
            initialized = false;
        }
        public Injector(Process process)
        {
            processHandle = new SafeWaitHandle(process.Handle, false);
            threadHandle = new SafeWaitHandle(IntPtr.Zero, true);

            this.process = process;
            initialized = true;
        }
        public Process Process => Process.GetProcessById(process.Id);
        public async Task Inject(string dllPath)
        {
            if (dllPath == null)
            {
                throw new ArgumentNullException(nameof(dllPath));
            }

            if (disposed)
            {
                throw new ObjectDisposedException(null);
            }

            var function = GetRemoteFunctionAddress("kernel32.dll", "LoadLibraryW");
            if (function == IntPtr.Zero)
            {
                throw new InvalidOperationException("Unable to locate LoadLibrary function.");
            }

            var buffer = Encoding.Unicode.GetBytes(dllPath + '\0');
            var remoteBuffer = new SafeProcessMemoryHandle(processHandle, (uint)buffer.Length);
            memoryHandles.Add(remoteBuffer);

            Write(processHandle, remoteBuffer, buffer);

            if (!initialized)
            {
                if (NativeMethods.QueueUserAPC(function, threadHandle, remoteBuffer) == 0)
                {
                    throw new Win32Exception();
                }
            }
            else
            {
                var remoteThreadHandle = NativeMethods.CreateRemoteThread(processHandle, IntPtr.Zero, UIntPtr.Zero, function,
                    remoteBuffer, 0, IntPtr.Zero);
                using (var remoteThread = new SafeWaitHandle(remoteThreadHandle, true))
                {
                    if (remoteThread.IsInvalid)
                    {
                        throw new Win32Exception();
                    }

                    if (NativeMethods.WaitForSingleObject(remoteThread, NativeMethods.INFINITE) != NativeMethods.WAIT_OBJECT_0)
                    {
                        throw new Win32Exception();
                    }

                    if (!NativeMethods.GetExitCodeThread(remoteThread, out var hmodule))
                    {
                        throw new Win32Exception();
                    }

                    if (hmodule == 0)
                    {
                        throw new Win32Exception(6 /* ERROR_INVALID_HANDLE */, "Failed to inject " + Path.GetFileName(dllPath));
                    }
                }
            }
        }
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }
        protected virtual void Dispose(bool disposing)
        {
            if (!disposed && disposing)
            {
                try
                {
                    Resume();
                }
                catch
                {
                    if (!process.HasExited)
                    {
                        process.Kill();
                    }

                    throw;
                }
                finally
                {
                    foreach (var handle in memoryHandles)
                    {
                        handle.Dispose();
                    }
                    memoryHandles.Clear();

                    threadHandle.Dispose();
                    processHandle.Dispose();
                    process.Dispose();
                }
            }

            disposed = true;
        }
        private void Suspend()
        {
            if (!threadHandle.IsInvalid && NativeMethods.SuspendThread(threadHandle) == 0xFFFFFFFF)
            {
                throw new Win32Exception();
            }
        }
        public void ResumeProcess()
        {
            if (!initialized && !disposed)
            {
                Resume();
                initialized = true;
            }
        }

        private void Resume()
        {
            if (!threadHandle.IsInvalid && NativeMethods.ResumeThread(threadHandle) == 0xFFFFFFFF)
            {
                throw new Win32Exception();
            }
        }
        private IntPtr GetRemoteFunctionAddress(string module, string function)
        {
            var hModule = NativeMethods.GetModuleHandle(module);
            if (hModule == IntPtr.Zero)
            {
                hModule = NativeMethods.LoadLibrary(module);
            }

            if (hModule != IntPtr.Zero)
            {
                return NativeMethods.GetProcAddress(hModule, function);
            }

            return IntPtr.Zero;
        }
        private static string EscapeArguments(IEnumerable<string> args)
        {
            var quotedArgs =
                from a in args ?? new string[0]
                where !string.IsNullOrEmpty(a)
                select EscapeArgument(a);
            return string.Join(" ", quotedArgs);
        }
        private static string EscapeArgument(string argument)
        {
            if (argument.Any(c => "\"\t ".Contains(c)))
            {
                return FormattableString.Invariant($"\"{Regex.Replace(argument, @"(\\*)(\\$|\"")", @"$1$1\$2")}\"");
            }

            return argument;
        }
        private static async Task<T> ReadAsync<T>(Stream stream)
        {
            var buffer = new byte[Marshal.SizeOf(typeof(T))];
            var count = 0;
            do
            {
                count += await stream.ReadAsync(buffer, count, buffer.Length);
            }
            while (count < buffer.Length);

            var handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
            try
            {
                return (T)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(T));
            }
            finally
            {
                handle.Free();
            }
        }
        private static T Read<T>(SafeWaitHandle process, IntPtr address)
        {
            var size = Marshal.SizeOf(typeof(T));
            var buffer = Marshal.AllocHGlobal(size);
            try
            {
                if (buffer != IntPtr.Zero && !NativeMethods.ReadProcessMemory(process, address, buffer, (UIntPtr)size, IntPtr.Zero))
                {
                    throw new Win32Exception();
                }

                return (T)Marshal.PtrToStructure(buffer, typeof(T));
            }
            finally
            {
                if (buffer != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(buffer);
                }
            }
        }
        private static byte[] Read(SafeWaitHandle process, IntPtr address, uint length)
        {
            var buffer = new byte[length];

            if (!NativeMethods.ReadProcessMemory(process, address, buffer, (UIntPtr)buffer.Length, IntPtr.Zero))
            {
                throw new Win32Exception();
            }

            return buffer;
        }
        private static void Write(SafeWaitHandle process, IntPtr address, byte[] buffer)
        {
            if (!NativeMethods.WriteProcessMemory(process, address, buffer, (UIntPtr)buffer.Length, IntPtr.Zero))
            {
                throw new Win32Exception();
            }
        }
        private static void Write(SafeWaitHandle process, SafeProcessMemoryHandle address, byte[] buffer)
        {
            if (!NativeMethods.WriteProcessMemory(process, address, buffer, (UIntPtr)buffer.Length, IntPtr.Zero))
            {
                throw new Win32Exception();
            }
        }
    }
}