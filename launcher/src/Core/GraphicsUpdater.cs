using System;
using System.IO;
using System.Net.Http;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.IO.Compression;

namespace Windower.Core
{
    public static class GraphicsUpdater
    {
        private static readonly HttpClient client = new HttpClient();

        public static void ApplyGraphicsEngine(Profile.GraphicsEngine engine, string polDirectory)
        {
            if (string.IsNullOrEmpty(polDirectory))
            {
                return;
            }

            string targetD3d8 = Path.Combine(polDirectory, "d3d8.dll");
            string targetD3d9 = Path.Combine(polDirectory, "d3d9.dll");
            string targetConf = Path.Combine(polDirectory, "dgVoodoo.conf");
            SafeDelete(targetD3d8);
            SafeDelete(targetD3d9);
            SafeDelete(targetConf);

            if (engine == Profile.GraphicsEngine.Direct3D12)
            {
                string sourceDir = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "res", "dgVoodoo2");
                string sourceD3d8 = Path.Combine(sourceDir, "d3d8.dll");
                string sourceConf = Path.Combine(sourceDir, "dgVoodoo.conf");

                if (File.Exists(sourceD3d8))
                {
                    try
                    {
                        File.Copy(sourceD3d8, targetD3d8, true);

                        if (File.Exists(sourceConf))
                        {
                            string confContent = File.ReadAllText(sourceConf);
                            // Permanently enforce DX12!
                            confContent = Regex.Replace(confContent, @"OutputAPI\s*=.*", "OutputAPI                            = d3d12_fl11_0");
                            File.WriteAllText(targetConf, confContent);
                        }
                    }
                    catch (IOException) { /* file in use by another FFXI instance, skip */ }
                    catch (UnauthorizedAccessException) { /* file locked, skip */ }
                }
            }
        }

        private static void SafeDelete(string path)
        {
            if (!File.Exists(path))
            {
                return;
            }

            try
            {
                File.SetAttributes(path, FileAttributes.Normal);
                File.Delete(path);
            }
            catch (UnauthorizedAccessException) { /* file locked by game process, skip */ }
            catch (IOException) { /* file in use, skip */ }
        }
    }
}
