namespace Windower.Core
{
    using System;
    using System.IO;
    using System.Text;
    using System.Text.RegularExpressions;

    public static class DgVoodooManager
    {
        public static void DeployConfig(string targetDirectory, int vramMb, Profile.GraphicsEngine engine)
        {
            var confPath = Path.Combine(targetDirectory, "dgVoodoo.conf");
            var outputApi = engine == Profile.GraphicsEngine.Direct3D12 ? "d3d12_fl12_0" : "d3d11_fl11_0";

            if (File.Exists(confPath))
            {
                // Surgically update existing file so we don't nuke the Version stamp or user tweaks
                string content = File.ReadAllText(confPath);

                content = Regex.Replace(content, @"^(?i)(OutputAPI\s*=\s*).*?$", $"${{1}}{outputApi}", RegexOptions.Multiline);
                content = Regex.Replace(content, @"^(?i)(VRAM\s*=\s*).*?$", $"${{1}}{vramMb}", RegexOptions.Multiline);
                content = Regex.Replace(content, @"^(?i)(dgVoodooWatermark\s*=\s*).*?$", "${1}false", RegexOptions.Multiline);

                File.WriteAllText(confPath, content);
            }
            else
            {
                // Generate a fully forged config from scratch. 
                var conf = new StringBuilder();
                conf.AppendLine("[General]");
                conf.AppendLine("Version = 0x287");
                conf.AppendLine($"OutputAPI = {outputApi}");
                conf.AppendLine("Adapters = all");
                conf.AppendLine("ScalingMode = stretched_ar");
                conf.AppendLine();

                conf.AppendLine("[DirectX]");
                conf.AppendLine("DisableAndPassThru = false");
                conf.AppendLine($"VRAM = {vramMb}");
                conf.AppendLine("Filtering = appdriven");
                conf.AppendLine("Resolution = unforced");
                conf.AppendLine("Antialiasing = appdriven");
                conf.AppendLine("AppControlledScreenMode = true");
                conf.AppendLine("DisableAltEnterToToggleScreenMode = true");
                conf.AppendLine("BilinearAUMipZooming = false");
                conf.AppendLine("FastVideoMemoryAccess = false");
                conf.AppendLine("dgVoodooWatermark = false");
                conf.AppendLine();

                conf.AppendLine("[Glide]");
                conf.AppendLine("dgVoodooWatermark = false");

                File.WriteAllText(confPath, conf.ToString());
            }
        }
    }
}
