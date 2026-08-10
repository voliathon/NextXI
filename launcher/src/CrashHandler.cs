namespace Windower
{
    using System;
    using System.Collections;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Security;
    using System.Text;

    public static class CrashHandler
    {
        private const int BasicMiniDump = 0x00081965;
        private const int FullMiniDump = 0x00021926;

        public static void InstallCrashLogger()
        {
            AppDomain.CurrentDomain.UnhandledException += LogCrash;
            AppDomain.CurrentDomain.UnhandledException -= HandleCrash;
        }

        public static void InstallCrashHandler()
        {
            AppDomain.CurrentDomain.UnhandledException += HandleCrash;
            AppDomain.CurrentDomain.UnhandledException -= LogCrash;
        }

        public static string PrepareStackTrace(object data)
        {
            var stackTrace = new StringBuilder();

            if (data is Exception exception)
            {
                _ = stackTrace.Append(exception.GetType().FullName).Append(": ").AppendLine(exception.Message);
                if (exception.Data.Count > 0)
                {
                    _ = stackTrace.AppendLine("Data:");
                    foreach (var entry in exception.Data.Cast<DictionaryEntry>())
                    {
                        _ = stackTrace.Append("   ").Append(entry.Key ?? "<null>").Append(": ")
                            .Append(entry.Value ?? "<null>").AppendLine();
                    }
                }
                _ = stackTrace.AppendLine("Stack Trace:");
                _ = stackTrace.Append(exception.StackTrace);
            }
            else
            {
                _ = stackTrace.Append(data);
            }

            return stackTrace.ToString();
        }

        private static string GetSignature(object data)
        {
            var signature = new StringBuilder();

            if (data is Exception exception)
            {
                _ = signature.Append(exception.GetType().Name);
                _ = signature.Append('@');
                var target = exception.TargetSite;
                if (target == null)
                {
                    _ = signature.Append("<Unknown>");
                }
                else
                {
                    _ = signature.Append(target.Module.Name.EndsWith(".dll", StringComparison.OrdinalIgnoreCase) ?
                        target.Module.Name : target.Module.ScopeName);
                    _ = signature.Append('!');
                    _ = signature.Append(target.ReflectedType?.FullName ?? "<Unknown>");
                    _ = signature.Append('.').Append(target.Name);
                    var parameters = target.GetParameters();
                    _ = signature.Append('(').Append(string.Join(", ", parameters.Select(p => p.ParameterType.Name))).Append(')');
                }
            }
            else
            {
                _ = signature.Append(data.GetType().FullName);
            }

            return signature.ToString();
        }

        [SuppressMessage("Microsoft.Usage", "CA2202")]
        [SecurityCritical]
        private static void LogCrash(object sender, UnhandledExceptionEventArgs e)
        {
            if (!Debugger.IsAttached)
            {
                var path = CrashReporter.CrashDumpPath;
                _ = Directory.CreateDirectory(path);
                using (var stream = Utilities.CreateTimestampedFileStream(path, ".md"))
                using (var writer = new StreamWriter(stream, new UTF8Encoding(), 1024, true))
                {
                    writer.Write(CrashReporter.PrepareCrashReport(PrepareStackTrace(e.ExceptionObject)));
                }
            }
        }

        [SecurityCritical]
        private static void HandleCrash(object sender, UnhandledExceptionEventArgs e)
        {
            if (!Debugger.IsAttached)
            {
                try
                {
                    var signature = GetSignature(e.ExceptionObject);
                    var stackTrace = PrepareStackTrace(e.ExceptionObject);
                    var dumpFile = Path.Combine(Path.GetTempPath(), "NextXI", Guid.NewGuid() + ".dmp");

                    _ = Directory.CreateDirectory(Path.GetDirectoryName(dumpFile));
                    using (var stream = new FileStream(dumpFile, FileMode.Create))
                    using (var process = Process.GetCurrentProcess())
                    {
                        var exceptionInfo = default(NativeMethods.MINIDUMP_EXCEPTION_INFORMATION);
                        exceptionInfo.ThreadId = NativeMethods.GetCurrentThreadId();
                        exceptionInfo.ExceptionPointers = Marshal.GetExceptionPointers();
                        _ = NativeMethods.MiniDumpWriteDump(process.Handle, process.Id, stream.SafeFileHandle, BasicMiniDump,
                            ref exceptionInfo, IntPtr.Zero, IntPtr.Zero);
                    }

                    stackTrace = Convert.ToBase64String(Encoding.UTF8.GetBytes(stackTrace));
                    var path = Environment.ProcessPath;
                    _ = Process.Start(path, "report-crash --signature \"" + signature +
                        "\" --stack-trace \"" + stackTrace + "\" \"" + dumpFile.Replace("\"", "\\\"") + "\"");

                    Environment.Exit(-1);
                }
                catch (Exception ex)
                {
                    LogCrash(null, new UnhandledExceptionEventArgs(ex, true));
                    throw;
                }
            }
        }
    }
}
