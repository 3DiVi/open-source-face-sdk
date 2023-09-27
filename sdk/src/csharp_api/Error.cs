using System;

namespace CSharpApi
{
    public class Error : Exception
    {
        private object code;
        public object _code
        {
            get
            {
                if (code is int)
                {
                    return $"0x{code:x}";
                }
                else
                return (string)code;
            }
            set
            {
                code = value;
            }
        }
        string _what{get; set;}
        public Error(object code, string what)
        {
            _code = code;
            _what = what;
        }
        public override string Message => $"{_code}: {_what}";

    }
}