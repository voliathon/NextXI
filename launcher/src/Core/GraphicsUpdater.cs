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
            if (string.IsNullOrEmpty(polDirectory)) return;

            string targetD3d8 = Path.Combine(polDirectory, "d3d8.dll");
            string targetD3d9 = Path.Combine(polDirectory, "d3d9.dll");
            string targetConf = Path.Combine(polDirectory, "dgVoodoo.conf");

            // Cleanup previous
            if (File.Exists(targetD3d8)) File.Delete(targetD3d8);
            if (File.Exists(targetD3d9)) File.Delete(targetD3d9);
            if (File.Exists(targetConf)) File.Delete(targetConf);

            if (engine == Profile.GraphicsEngine.dgVoodoo2)
            {
                string sourceDir = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "res", "dgVoodoo2");
                if (File.Exists(Path.Combine(sourceDir, "d3d8.dll")))
                {
                    File.Copy(Path.Combine(sourceDir, "d3d8.dll"), targetD3d8, true);
                    File.Copy(Path.Combine(sourceDir, "dgVoodoo.conf"), targetConf, true);
                }
            }
        }
    }
}
