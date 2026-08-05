namespace Windower.Core
{
    public enum LaunchStatus
    {
        Unknown,
        Elevating,
        CheckingDirectPlay,
        InstallingDirectPlay,
        Launching,
        Installing,
        TransferringSettings,
        WaitingForWindow,
        Complete
    }
}