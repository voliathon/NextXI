using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace Windower.Core
{
    public static class ResourceManager
    {
        private static readonly HttpClient client = new HttpClient();

        // Define our exact supply lines, now with specific UI statuses!
        private class SyncTarget
        {
            public string TreeUrl { get; set; }
            public string RawBaseUrl { get; set; }
            public string RepoPathPrefix { get; set; }
            public string LocalRelativeDir { get; set; }
            public LaunchStatus StatusEnum { get; set; } // CAVEMAN FIX: Links repo to UI text
        }

        private static readonly SyncTarget[] SyncTargets = new[]
        {
            // 1. addons/shared_libs -> Windower/Resources (master)
            new SyncTarget
            {
                TreeUrl = "https://api.github.com/repos/Windower/Resources/git/trees/master?recursive=1",
                RawBaseUrl = "https://raw.githubusercontent.com/Windower/Resources/master/",
                RepoPathPrefix = "resources_data/",
                LocalRelativeDir = "shared_libs",
                StatusEnum = LaunchStatus.UpdatingSharedLibs
            },
            // 2. addons/windower/libs -> Windower/Lua (dev)
            new SyncTarget
            {
                TreeUrl = "https://api.github.com/repos/Windower/Lua/git/trees/dev?recursive=1",
                RawBaseUrl = "https://raw.githubusercontent.com/Windower/Lua/dev/",
                RepoPathPrefix = "addons/libs/",
                LocalRelativeDir = @"windower\libs",
                StatusEnum = LaunchStatus.UpdatingWindowerLibs
            },
            // 3. addons/nextxi/libs -> Windower/packages (master)
            new SyncTarget
            {
                TreeUrl = "https://api.github.com/repos/Windower/packages/git/trees/master?recursive=1",
                RawBaseUrl = "https://raw.githubusercontent.com/Windower/packages/master/",
                RepoPathPrefix = "libraries/",
                LocalRelativeDir = @"nextxi\libs",
                StatusEnum = LaunchStatus.UpdatingNextXILibs
            }
        };

        static ResourceManager()
        {
            client.DefaultRequestHeaders.UserAgent.ParseAdd("NextXI-Launcher/1.0");
        }

        public static async Task<bool> CheckAndDownloadResourcesAsync(string rootPath, IProgress<ProgressDetail<LaunchStatus>> progress = null)
        {
            try
            {
                string addonsDir = Path.Combine(rootPath, "addons");
                string manifestPath = Path.Combine(addonsDir, "manifest.json");

                // --- 1. READ THE LOCAL LEDGER ---
                var localManifest = new Dictionary<string, string>();
                if (File.Exists(manifestPath))
                {
                    string json = File.ReadAllText(manifestPath);
                    localManifest = JsonConvert.DeserializeObject<Dictionary<string, string>>(json) ?? new Dictionary<string, string>();
                }

                var updatedManifest = new Dictionary<string, string>(localManifest);
                bool hasChanges = false;

                // --- 2. SCOUT EACH TARGET ---
                foreach (var target in SyncTargets)
                {
                    // Let the UI know we are actively checking this specific repository
                    progress?.Report(ProgressDetail.Create(0, 0, target.StatusEnum));

                    var response = await client.GetAsync(target.TreeUrl);
                    if (!response.IsSuccessStatusCode)
                    {
                        continue; // If one repo fails (e.g. rate limit), skip it and try the others
                    }

                    string treeJson = await response.Content.ReadAsStringAsync();

                    var treeDefinition = new { tree = new[] { new { path = "", type = "", sha = "" } } };
                    var treeData = JsonConvert.DeserializeAnonymousType(treeJson, treeDefinition);

                    var toDownload = new List<dynamic>();

                    foreach (var item in treeData.tree)
                    {
                        if (item.type == "blob" && item.path.StartsWith(target.RepoPathPrefix))
                        {
                            string remainingPath = item.path.Substring(target.RepoPathPrefix.Length);
                            string ledgerKey = $"{target.LocalRelativeDir}/{remainingPath}".Replace('\\', '/');

                            if (!localManifest.TryGetValue(ledgerKey, out string localSha) || localSha != item.sha)
                            {
                                toDownload.Add(new
                                {
                                    GitHubPath = item.path,
                                    RelativeLocalPath = remainingPath,
                                    LedgerKey = ledgerKey,
                                    Sha = item.sha
                                });
                            }
                        }
                    }

                    // --- 3. DOWNLOAD CHANGED FILES FOR THIS TARGET ---
                    int totalFiles = toDownload.Count;
                    int currentFile = 0;

                    if (totalFiles > 0)
                    {
                        foreach (var item in toDownload)
                        {
                            string fileUrl = target.RawBaseUrl + item.GitHubPath;
                            string destPath = Path.Combine(addonsDir, target.LocalRelativeDir, item.RelativeLocalPath);

                            Directory.CreateDirectory(Path.GetDirectoryName(destPath));

                            byte[] fileBytes = await client.GetByteArrayAsync(fileUrl);
                            File.WriteAllBytes(destPath, fileBytes);

                            updatedManifest[item.LedgerKey] = item.Sha;
                            hasChanges = true;

                            // Report detailed progress to the UI! (e.g., "Updating Windower libraries..." [2 / 14])
                            currentFile++;
                            progress?.Report(ProgressDetail.Create(currentFile, totalFiles, target.StatusEnum));
                        }
                    }
                }

                // --- 4. SAVE THE LEDGER ---
                if (hasChanges)
                {
                    string newJson = JsonConvert.SerializeObject(updatedManifest, Formatting.Indented);
                    File.WriteAllText(manifestPath, newJson);
                }

                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }
    }
}
