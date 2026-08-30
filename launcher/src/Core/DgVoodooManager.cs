namespace Windower.Core
{
    using System.IO;
    using System.Text;

    public static class DgVoodooManager
    {
        public static void DeployConfig(string targetDirectory, int vramMb, Profile.GraphicsEngine engine)
        {
            var conf = new StringBuilder();

            // Map the Launcher enum directly to dgVoodoo's expected Feature Level strings
            var outputApi = engine == Profile.GraphicsEngine.Direct3D12 ? "d3d12_fl12_0" : "d3d11_fl11_0";

            // General Settings
            conf.AppendLine("[General]");
            conf.AppendLine($"OutputAPI = {outputApi}"); // Dynamically forces DX11 or DX12
            conf.AppendLine("Adapters = all");
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
            conf.AppendLine("FastVideoMemoryAccess = false");
            conf.AppendLine("dgVoodooWatermark = false");     // Completely kills the watermark

            File.WriteAllText(Path.Combine(targetDirectory, "dgVoodoo.conf"), conf.ToString());
        }
    }
}
