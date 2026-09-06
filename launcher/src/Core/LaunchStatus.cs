namespace Windower.Core
{
    public enum LaunchStatus
    {
        Unknown,
        Elevating,
        CheckingDirectPlay,
        InstallingDirectPlay,
        UpdatingSharedLibs,
        UpdatingWindowerLibs,
        UpdatingNextXILibs,   
        Launching,
        Installing,
        TransferringSettings,
        WaitingForWindow,
        Complete
    }
}
