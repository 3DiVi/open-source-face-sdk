using System;
using System.Runtime.InteropServices;

namespace CSharpApi
{
    public class ComplexObject
    {
        unsafe private void* impl;
        unsafe public void* _impl
        {
            get
            {   
                return GetImpl();
            }
            set
            {
                impl = value; 
            }
        }
        [DllImport("open_source_sdk.dll")]
        unsafe private static extern void TDVException_deleteException(void* eh);

        unsafe public ComplexObject(void* impl)
        {
            _impl = impl;
        }
        unsafe ~ComplexObject()
        {
            TDVException_deleteException(_impl);
            _impl = null;
        }
        unsafe public void* GetImpl()
        {
            if (impl == null)
            {
                throw new Error(0x0563958d, "Using of deleted object");
            }
            return impl;
        }
        

    }
}