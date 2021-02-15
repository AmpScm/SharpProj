using System;
using System.Threading;

namespace SharpProj.Implementation
{
    internal static class ProjImplementationExtensions
    {
        internal static Disposer WithReadLock(this ReaderWriterLockSlim rwls)
        {
            rwls.EnterReadLock();

            return new Disposer(() => rwls.ExitReadLock());
        }

        internal static Disposer WithWriteLock(this ReaderWriterLockSlim rwls)
        {
            rwls.EnterWriteLock();

            return new Disposer(() => rwls.ExitWriteLock());
        }

        internal sealed class Disposer : IDisposable
        {
            Action _dispose;

            public Disposer(Action onDispose)
            {
                _dispose = onDispose;
            }

            public void Dispose()
            {
                try
                {
                    _dispose?.Invoke();
                }
                finally
                {
                    _dispose = null;
                }
            }
        }
    }
}
