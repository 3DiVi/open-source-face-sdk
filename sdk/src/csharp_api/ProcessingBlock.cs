using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using System.Runtime.InteropServices;
using CSharpApi;

namespace CSharpApi
{
    public class ProcessingBlock : ComplexObject
    {
        unsafe public ProcessingBlock(Dictionary<object,object> ctx, void* impl = null) : base(impl)
        {
            void* exception = ErrorMethods.MakeException();
            Context metaCtx = new Context();

            metaCtx.Invoke(ctx);
            impl = TDVProcessingBlock_createProcessingBlock(metaCtx._impl, ref exception);

            ErrorMethods.CheckException(exception);
            _impl = impl;
        }
        [DllImport("open_source_sdk.dll")]
        unsafe private static extern void TDVProcessingBlock_destroyBlock(void* handle_, void* eh);
        [DllImport("open_source_sdk.dll")]
        unsafe private static extern void* TDVProcessingBlock_createProcessingBlock(void* config, ref void* eh);
        [DllImport("open_source_sdk.dll")]
        unsafe private static extern void TDVProcessingBlock_processContext(void* handle_, void* ctx, void* eh);
 
        unsafe ~ProcessingBlock()
        {
            void* exception = ErrorMethods.MakeException();

            TDVProcessingBlock_destroyBlock(_impl, exception);

            ErrorMethods.CheckException(exception);
        }

        /**
	    * @brief Infer and put results in ctx
	    * 
	    * @param ctx Results of infer
	    */
        unsafe public void Invoke(object ctx)
        {
            if (ctx is Dictionary<object, object> dict)
            {
                InvokeWithDictionary(dict);
            }
            else if (ctx is Context newCtx) 
            {
                InvokeWithContext(newCtx);
            }
            else
            {
                throw new Error(0xa341de35, "Wrong type of ctx");
            }
        }

        /**
         * @brief Invoke variant with Dictionary
        */
        unsafe private void InvokeWithDictionary(Dictionary<object, object> ctx)
        {
            void* exception = ErrorMethods.MakeException();
            Context metaCtx = new Context();
            metaCtx.Invoke(ctx);

            TDVProcessingBlock_processContext(_impl, metaCtx._impl, exception);

            ErrorMethods.CheckException(exception);

            var newKeys = new HashSet<object>(metaCtx.Keys()).Except(ctx.Keys);
            foreach (var key in newKeys)
            {
                ctx[key] = GetOutputData(metaCtx[(string)key]);
            }
        }

        /**
         * @brief Invoke variant with Context
        */
        unsafe private void InvokeWithContext(Context ctx)
        {
            void* exception = ErrorMethods.MakeException();

            TDVProcessingBlock_processContext(_impl, ctx._impl, exception);
            
            ErrorMethods.CheckException(exception);
        }

        /**
         * @brief Get all data from Context
         * @return Context data
        */
        unsafe public object GetOutputData(Context metaCtx)
        {
            if (metaCtx.IsArray())
            {
                var output = new List<object>();
                for (int i = 0; i < (int)metaCtx.Length(); i++)
                {
                    output.Add(GetOutputData(metaCtx[i]));
                }
                return output;
            }

            if (metaCtx.IsObject())
            {
                var output = new Dictionary<object, object>();

                foreach (var key in metaCtx.Keys())
                {
                    output[key] = GetOutputData(metaCtx[(string)key]);
                }
                return output;
            }

            return metaCtx.GetValue();
        }
    }
}