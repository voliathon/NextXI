namespace Windower.Core
{
    using System.IO;
    using System.Text;

    public static class DgVoodooManager
    {
        // We default to 1024MB. This is the optimal safe budget for FFXI multi-boxing. 
        // 2048MB is only needed if the user loads aggressive HD texture packs (like Ashenbubs).
        public static void DeployConfig(string targetDirectory, int vramMb = 1024)
        {
            var conf = new StringBuilder();

            // General Settings
            conf.AppendLine("[General]");
            conf.AppendLine("OutputAPI = d3d11_fl11_0"); // Force DirectX 11 Feature Level 11
            conf.AppendLine("Adapters = all");         // 'all' safely defaults to the primary GPU in DX11
            conf.AppendLine("ScalingMode = stretched_ar");
            conf.AppendLine();

            // DirectX Settings
            conf.AppendLine("[DirectX]");
            conf.AppendLine("DisableAndPassThru = false");
            conf.AppendLine($"VRAM = {vramMb}");
            conf.AppendLine("Filtering = appdriven");
            conf.AppendLine("Resolution = unforced");
            conf.AppendLine("Antialiasing = appdriven");
            conf.AppendLine("AppControlledScreenMode = true");
            conf.AppendLine("DisableAltEnterToToggleScreenMode = true");
            conf.AppendLine("BilinearAUMipZooming = false");
            conf.AppendLine("FastVideoMemoryAccess = false"); // Must be false or screenshots/ImGui corrupt
            conf.AppendLine("dgVoodooWatermark = false");     // Kills the watermark

            // Write the config directly to the PlayOnline folder
            File.WriteAllText(Path.Combine(targetDirectory, "dgVoodoo.conf"), conf.ToString());
        }
    }
}
