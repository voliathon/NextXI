#pragma warning disable SYSLIB0011

/*
 * Copyright © Windower Dev Team
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"),to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

namespace Windower
{
    using CommandLine;
    using Core;
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.IO.Pipes;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.ExceptionServices;
    using System.Text;
    using System.Text.Json;
    using System.Threading;
    using System.Threading.Tasks;
    using Windower.UI;

    using static System.FormattableString;

    /// <summary>
    /// The class containing the program entry point.
    /// </summary>
    [SuppressMessage("Microsoft.Maintainability", "CA1506")]
    public static class Program
    {
        public static bool IsMono => Type.GetType("Mono.Runtime") != null;
        public static string BuildTag => GetBuildTag();

        public static async Task<TResult> RemoteCallAsync<TResult>(Func<CancellationToken, TResult> method,
            CancellationToken token) =>
            (TResult)await RemoteCallAsync(false, method, token);

        public static async Task ElevateAsync<T1>(Action<T1, CancellationToken> method, T1 arg1, CancellationToken token) =>
            await RemoteCallAsync(true, method, token, arg1);

        public static async Task<TResult> ElevateAsync<T1, T2, T3, TResult>(Func<T1, T2, T3, CancellationToken, TResult> method,
            T1 arg1, T2 arg2, T3 arg3, CancellationToken token) =>
            (TResult)await RemoteCallAsync(true, method, token, arg1, arg2, arg3);

        /// <summary>
        /// The program entry point.
        /// </summary>
        private static void Main(string[] args)
        {
            CrashHandler.InstallCrashLogger();
            NativeMethods.AttachConsole(NativeMethods.ATTACH_PARENT_PROCESS);
            Shell.Initialize();
            Paths.Initialize();

            ParseInternalArguments(args);
        }

        private static string GetBuildTag()
        {
#if WINDOWER_RELEASE_BUILD
            const string tag = ""; // <-- FIXED: Changed from string.Empty
#else
            const string tag = "Development Build";
#endif

            var attributes = typeof(Program).Assembly.GetCustomAttributes<AssemblyMetadataAttribute>();
            return attributes.FirstOrDefault(a => a.Key == "BuildTag")?.Value ?? tag;
        }

        private static void ParseInternalArguments(string[] args)
        {
            using (var parser = new Parser(s => s.HelpWriter = null))
            {
                parser.ParseArguments<RemoteCallOptions, ReportCrashOptions, UpdateCleanupOptions>(args)
                    .WithParsed<RemoteCallOptions>(o => ExecuteAndExit(o.ProcessId))
                    .WithParsed<ReportCrashOptions>(ReportCrash)
                    .WithParsed<UpdateCleanupOptions>(UpdateCleanup)
                    .WithNotParsed(o => ParseArguments(args));
            }
        }

        private static void ParseArguments(string[] args)
        {
            CrashHandler.InstallCrashHandler();

            if (args.Length == 0 && !IsMono)
            {
                UserInterface.Run();
            }
            else
            {
                if (!IsMono)
                {
                    // Windows doesn't block the console for non-console
                    // executables, so clear the current line so things
                    // don't look too out of place.
                    try
                    {
                        Console.CursorLeft = 0;
                        Console.Write(new string(' ', Console.BufferWidth));
                        Console.CursorLeft = 0;
                    }
                    catch (IOException) { }
                }

                Parser.Default.ParseArguments<LaunchOptions, SaveOptions, DeleteOptions, GetArgsOptions>(args)
                    .WithParsed<LaunchOptions>(Launch)
                    .WithParsed<SaveOptions>(SaveProfile)
                    .WithParsed<DeleteOptions>(DeleteProfile)
                    .WithParsed<GetArgsOptions>(GetArgs);

                if (!IsMono)
                {
                    // Print out a dummy prompt.
                    Console.Write(Invariant($"{Directory.GetCurrentDirectory()}>"));
                }
            }
        }

        private static void Launch(LaunchOptions options)
        {
            try
            {
                if (options.NoGui || IsMono)
                {
                    Updater.Update().GetAwaiter().GetResult();
                    Launcher.Launch(options.GetProfile(), CancellationToken.None);
                }
                else
                {
                    UserInterface.Run(options.GetProfile());
                }
            }
            catch (KeyNotFoundException e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }

        private static void SaveProfile(SaveOptions options)
        {
            try
            {
                var profile = options.GetProfile();
                if (Launcher.ProfileManager.TryGetValue(profile.Name, out var oldProfile))
                {
                    if (options.Overwrite)
                    {
                        Launcher.ProfileManager.Remove(oldProfile);
                    }
                    else
                    {
                        Console.Error.WriteLine("A profile with the name \"{0}\" already exists. (Use --overwrite to replace it.)",
                            profile.Name);
                        return;
                    }
                }
                Launcher.ProfileManager.Add(profile);
                Launcher.ProfileManager.Save();
            }
            catch (KeyNotFoundException e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }

        private static void DeleteProfile(DeleteOptions options)
        {
            try
            {
                var profile = Launcher.ProfileManager[options.ProfileName];
                Launcher.ProfileManager.Remove(profile);
                Launcher.ProfileManager.Save();
            }
            catch (KeyNotFoundException e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }

        private static void GetArgs(GetArgsOptions options)
        {
            try
            {
                Console.WriteLine(Launcher.ProfileManager[options.ProfileName].ArgString);
            }
            catch (KeyNotFoundException e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }

        private static void ReportCrash(ReportCrashOptions options)
        {
            if (!IsMono)
            {
                string stackTrace = null;
                if (!string.IsNullOrWhiteSpace(options.StackTrace))
                {
                    var encoded = options.StackTrace.Trim();
                    stackTrace = Encoding.UTF8.GetString(Convert.FromBase64String(encoded));
                }

                UserInterface.RunCrashReporter(options.Signature, options.CrashDumpPath, stackTrace);
            }
            else
            {
                // TODO: Implement crash reporter for Linux and macOS.
            }
        }

        private static void UpdateCleanup(UpdateCleanupOptions options) => Updater.CleanUp();

        [SuppressMessage("Microsoft.Design", "CA1031")]
        private static void ExecuteAndExit(int processId)
        {
            var name = Invariant($"Windower.RPC[{processId}]");
            using (var pipe = new NamedPipeClientStream(".", name, PipeDirection.InOut, PipeOptions.Asynchronous))
            {
                pipe.Connect();
                var result = default(ResultDescriptor);
                var jsonOptions = new JsonSerializerOptions { IncludeFields = true };

                try
                {
                    // .NET 10 Fix: Read exactly one line to avoid EOF deadlock
                    using var reader = new StreamReader(pipe, Encoding.UTF8, false, 1024, leaveOpen: true);
                    var json = reader.ReadLine();
                    var call = JsonSerializer.Deserialize<CallDescriptor>(json, jsonOptions);

                    var type = Type.GetType(call.TypeName);
                    var method = type?.GetMethod(call.MethodName, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);

                    if (method == null)
                    {
                        throw new InvalidOperationException(Invariant($"Method \"{call.MethodName}\" could not be found."));
                    }

                    var assembly = Assembly.GetExecutingAssembly();
                    if (method.ReflectedType.Assembly != assembly)
                    {
                        throw new InvalidOperationException(
                            Invariant($"Method \"{method.Name}\" is not in assembly \"{assembly.FullName}\"."));
                    }

                    if (method.GetCustomAttribute(typeof(RemoteCallableAttribute)) == null)
                    {
                        throw new InvalidOperationException(
                            Invariant($"Method \"{method.Name}\" does not have \"{nameof(RemoteCallableAttribute)}\"."));
                    }

                    // JSON deserializes objects to JsonElements. Convert them back to real types based on the method signature.
                    var parameters = method.GetParameters();
                    var finalArgs = new object[call.Arguments.Length];
                    for (int i = 0; i < call.Arguments.Length; i++)
                    {
                        if (call.Arguments[i] is JsonElement elem)
                        {
                            finalArgs[i] = elem.Deserialize(parameters[i].ParameterType, jsonOptions);
                        }
                        else
                        {
                            finalArgs[i] = call.Arguments[i];
                        }
                    }

                    var cancellationSignalName = Invariant($"Windower.RPC.CancellationSignal[{processId}]");
                    var cancellationSignal = new EventWaitHandle(false, EventResetMode.ManualReset, cancellationSignalName);
                    var source = new CancellationTokenSource();
                    new Thread(() =>
                    {
                        cancellationSignal.WaitOne();
                        source.Cancel();
                    }).Start();

                    result.Result = method.Invoke(null, finalArgs.Concat(new object[] { source.Token }).ToArray());
                    cancellationSignal.Set();
                }
                catch (TargetInvocationException e)
                {
                    result.ErrorMessage = e.InnerException?.ToString() ?? e.ToString();
                }
                catch (Exception e)
                {
                    result.ErrorMessage = e.ToString();
                }
                finally
                {
                    // .NET 10 Fix: Write exactly one line and instantly flush it through the pipe
                    using var writer = new StreamWriter(pipe, Encoding.UTF8, 1024, leaveOpen: true);
                    writer.WriteLine(JsonSerializer.Serialize(result, jsonOptions));
                    writer.Flush();

                    Environment.Exit(0);
                }
            }
        }

        private static async Task<object> RemoteCallAsync(bool elevate, Delegate method, CancellationToken token,
            params object[] args)
        {
            var processId = Process.GetCurrentProcess().Id;
            var name = Invariant($"Windower.RPC[{processId}]");
            using (var pipe = new NamedPipeServerStream(name, PipeDirection.InOut, 1, PipeTransmissionMode.Byte,
                PipeOptions.Asynchronous))
            {
                var info = new ProcessStartInfo()
                {
                    FileName = Environment.ProcessPath,
                    Arguments = Parser.Default.FormatCommandLine(new RemoteCallOptions { ProcessId = processId }),
                    Verb = elevate ? "runas" : null,
                    UseShellExecute = true //.NET8/10 at default disables ShellExecute, but we need it for elevation, so re-enable it if necessary.
                };
                using (var process = Process.Start(info))
                {
                    pipe.WaitForConnection();
                    var call = default(CallDescriptor);
                    var jsonOptions = new JsonSerializerOptions { IncludeFields = true };

                    call.TypeName = method.Method.DeclaringType.AssemblyQualifiedName;
                    call.MethodName = method.Method.Name;
                    call.Arguments = args;

                    // .NET 10 Fix: Write exactly one line and instantly flush it through the pipe
                    using (var writer = new StreamWriter(pipe, Encoding.UTF8, 1024, leaveOpen: true))
                    {
                        writer.WriteLine(JsonSerializer.Serialize(call, jsonOptions));
                        writer.Flush();
                    }

                    var result = default(ResultDescriptor);
                    if (token.CanBeCanceled)
                    {
                        var completed = new EventWaitHandle(false, EventResetMode.ManualReset);
                        var canceller = Task.Run(() =>
                        {
                            if (WaitHandle.WaitAny(new[] { token.WaitHandle, completed }) == 0)
                            {
                                var cancellationSignalName = Invariant($"Windower.RPC.CancellationSignal[{processId}]");
                                var cancellationSignal = new EventWaitHandle(false, EventResetMode.ManualReset,
                                    cancellationSignalName);
                                cancellationSignal.Set();
                            }
                        });
                        var runner = Task.Run(() =>
                        {
                            // .NET 10 Fix: Read exactly one line to avoid EOF deadlock
                            using var reader = new StreamReader(pipe, Encoding.UTF8, false, 1024, leaveOpen: true);
                            var responseJson = reader.ReadLine();
                            result = JsonSerializer.Deserialize<ResultDescriptor>(responseJson, jsonOptions);

                            completed.Set();
                        });
                        await Task.Run(() => Task.WaitAll(runner, canceller));
                    }
                    else
                    {
                        // .NET 10 Fix: Read exactly one line to avoid EOF deadlock
                        using var reader = new StreamReader(pipe, Encoding.UTF8, false, 1024, leaveOpen: true);
                        var responseJson = await Task.Run(() => reader.ReadLine());
                        result = JsonSerializer.Deserialize<ResultDescriptor>(responseJson, jsonOptions);
                    }

                    if (result.ErrorMessage != null)
                    {
                        // Throw a brand new exception using the text string we sent over the pipe
                        throw new Exception("Elevated Process Error:\n" + result.ErrorMessage);
                    }

                    // .NET 10 Fix: Unpack the JsonElement back into the original primitive type (like bool)
                    if (result.Result is JsonElement elem && method.Method.ReturnType != typeof(void))
                    {
                        return elem.Deserialize(method.Method.ReturnType, jsonOptions);
                    }

                    return result.Result;
                }
            }
        }

        private struct CallDescriptor
        {
            public string TypeName;
            public string MethodName;
            public object[] Arguments;
        }

        private struct ResultDescriptor
        {
            public object Result;
            public string ErrorMessage;
        }

        [Verb("remote-call")]
        private class RemoteCallOptions
        {
            [Value(0, Required = true)]
            public int ProcessId { get; set; }
        }

        [Verb("report-crash")]
        [SuppressMessage("Microsoft.Performance", "CA1812")]
        private class ReportCrashOptions
        {
            [Value(0, Required = true)]
            public string CrashDumpPath { get; set; }

            [Option("signature", Required = true)]
            public string Signature { get; set; }

            [Option("stack-trace")]
            public string StackTrace { get; set; }
        }
    }
}