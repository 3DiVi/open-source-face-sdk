using System;
using System.Runtime.InteropServices;
namespace CSharpApi
{
    public class ExceptionDestroyer : IDisposable
    {
        unsafe public void* _exception;
        [DllImport("open_source_sdk.dll")]
        unsafe private static extern void TDVException_deleteException(void* eh);
        unsafe public ExceptionDestroyer(void* exception)
        {
            _exception = exception;
        }
        unsafe public void Dispose()
        {
            if (_exception != null)
            {
                TDVException_deleteException(_exception);
            }
        }
    }
}