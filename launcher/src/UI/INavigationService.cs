namespace Windower.UI
{
    using System.Threading.Tasks;

    public interface INavigationService
    {
        Task<object> Open(object caller, string name, params object[] args);

        Task<T> Open<T>(object caller, string name, params object[] args);

        void Close();

        void Close(object result);

        Task Uninterruptible(Task task);

        Task<T> Uninterruptible<T>(Task<T> task);
    }
}