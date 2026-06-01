using System;
using System.IO;
using System.IO.Compression;
using System.Net.Http;
using System.Threading.Tasks;

namespace Windower.Core
{
    public static class ResourceManager
    {
        private static readonly HttpClient client = new HttpClient();

        public static string GetAddonLibsPath(string rootPath)
        {
            var libsDir = Path.Combine(rootPath, "addons", "libs");
            Directory.CreateDirectory(libsDir);
            return libsDir;
        }

        public static async Task<bool> CheckAndDownloadResourcesAsync(string rootPath)
        {
            try
            {
                string targetDir = Path.Combine(GetAddonLibsPath(rootPath), "resources");
                string versionFile = Path.Combine(GetAddonLibsPath(rootPath), "resources.version");

                // Check if resources exist, to prevent re-downloading every launch
                // For a robust system, we would check the GitHub commit hash via API.
                // For this implementation, we just check if the directory exists.
                if (Directory.Exists(targetDir) && File.Exists(versionFile))
                {
                    // Basic sanity check, return early if it's already pulled
                    return true;
                }

                client.DefaultRequestHeaders.UserAgent.ParseAdd("NextXI-Launcher/1.0");
                
                // GitHub provides a zipball of the latest master branch
                string downloadUrl = "https://github.com/Windower/Resources/archive/refs/heads/master.zip";
                
                var response = await client.GetAsync(downloadUrl);
                response.EnsureSuccessStatusCode();

                string tempZipPath = Path.Combine(Path.GetTempPath(), "WindowerResources.zip");
                using (var fs = new FileStream(tempZipPath, FileMode.Create, FileAccess.Write, FileShare.None))
                {
                    await response.Content.CopyToAsync(fs);
                }

                if (Directory.Exists(targetDir))
                {
                    Directory.Delete(targetDir, true);
                }

                string extractDir = Path.Combine(Path.GetTempPath(), "WindowerResourcesExtract");
                if (Directory.Exists(extractDir))
                {
                    Directory.Delete(extractDir, true);
                }

                Directory.CreateDirectory(extractDir);
                ZipFile.ExtractToDirectory(tempZipPath, extractDir);

                // The zip contains a top level directory like "Resources-master"
                var dirs = Directory.GetDirectories(extractDir);
                if (dirs.Length > 0)
                {
                    string sourceDir = dirs[0];
                    Directory.Move(sourceDir, targetDir);
                }

                // Cleanup
                File.Delete(tempZipPath);
                Directory.Delete(extractDir, true);

                File.WriteAllText(versionFile, DateTime.UtcNow.ToString("O"));
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }
    }
}
