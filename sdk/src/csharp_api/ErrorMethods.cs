using System;
using System.Runtime.InteropServices;
namespace CSharpApi
{
    public class ErrorMethods
    {
        [DllImport("open_source_sdk.dll")]
        unsafe private static extern IntPtr TDVException_getMessage(void* eh);
        [DllImport("open_source_sdk.dll")]
        unsafe private static extern int TDVException_getErrorCode(void* eh);
        unsafe public static void CheckException(void* exception)
        {
            using (ExceptionDestroyer ex = new ExceptionDestroyer(exception))
            {
                if (exception != null || (IntPtr)exception != IntPtr.Zero)
                {
                    IntPtr whatPtr = TDVException_getMessage(exception);
                    string whatStr = Marshal.PtrToStringAnsi(whatPtr);

                    int code = TDVException_getErrorCode(exception);
                    string hexCode = $"0x{code:x}";
                    throw new Error(hexCode, whatStr);
                }
            }
        }
        unsafe public static void* MakeException()
        {
            void* pointer = null;
            return pointer;
        }
    }
}