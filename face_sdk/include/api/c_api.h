#ifndef C_API_H
#define C_API_H

#ifdef _WIN32
#define TDV_PUBLIC extern "C" __declspec(dllexport)
#else
#define TDV_PUBLIC extern "C" __attribute__ ((visibility ("default")))
#endif


namespace api {

typedef struct HContext HContext;
typedef struct ContextEH ContextEH;

TDV_PUBLIC HContext* TDVContext_create(ContextEH ** eh);
TDV_PUBLIC void TDVContext_destroy(HContext* ctx, ContextEH ** eh);

TDV_PUBLIC HContext* TDVContext_getByIndex(HContext * ctx, int key, ContextEH ** eh);
TDV_PUBLIC HContext* TDVContext_getByKey(HContext * ctx, const char* key, ContextEH ** eh);
TDV_PUBLIC HContext* TDVContext_getOrInsertByKey(HContext * ctx, const char* key, ContextEH ** eh);

TDV_PUBLIC void TDVContext_copy(HContext * src, HContext * dst, ContextEH ** eh);
TDV_PUBLIC HContext* TDVContext_clone(HContext * ctx, ContextEH ** eh);
TDV_PUBLIC void TDVContext_clear(HContext * ctx, ContextEH ** eh);

TDV_PUBLIC void TDVContext_putStr(HContext * ctx, const char* str, ContextEH ** eh);
TDV_PUBLIC void TDVContext_putLong(HContext * ctx, long val, ContextEH ** eh);
TDV_PUBLIC void TDVContext_putDouble(HContext * ctx, double val, ContextEH ** eh);
TDV_PUBLIC void TDVContext_putBool(HContext * ctx, bool val, ContextEH ** eh);

TDV_PUBLIC unsigned char* TDVContext_allocDataPtr(HContext * ctx, unsigned long size, ContextEH ** eh);
TDV_PUBLIC unsigned char* TDVContext_putDataPtr(HContext * ctx, unsigned char* val, unsigned long copy_sz, ContextEH ** eh);
TDV_PUBLIC unsigned char* TDVContext_putConstDataPtr(HContext * ctx, const unsigned char* val, unsigned long copy_sz, ContextEH ** eh);

TDV_PUBLIC void TDVContext_pushBack(HContext * handle_, HContext * data, bool copy, ContextEH ** eh);

TDV_PUBLIC unsigned long TDVContext_getLength(HContext * ctx, ContextEH ** eh);
TDV_PUBLIC char** TDVContext_getKeys(HContext * ctx, unsigned long length, ContextEH ** eh);

TDV_PUBLIC bool TDVContext_isNone(HContext * ctx, ContextEH ** eh);
TDV_PUBLIC bool TDVContext_isArray(HContext * ctx, ContextEH ** eh);
TDV_PUBLIC bool TDVContext_isObject(HContext * ctx, ContextEH ** eh);

TDV_PUBLIC bool TDVContext_isBool(HContext * ctx, ContextEH ** eh);
TDV_PUBLIC bool TDVContext_isLong(HContext * ctx, ContextEH ** eh);
TDV_PUBLIC bool TDVContext_isDouble(HContext * ctx, ContextEH ** eh);
TDV_PUBLIC bool TDVContext_isString(HContext * ctx, ContextEH ** eh);
TDV_PUBLIC bool TDVContext_isDataPtr(HContext * ctx, ContextEH ** eh);

TDV_PUBLIC const char* TDVContext_getStr(HContext * ctx, char* buff, ContextEH ** eh);
TDV_PUBLIC unsigned long TDVContext_getStrSize(HContext * ctx, ContextEH ** eh);
TDV_PUBLIC void TDVContext_freePtr(void* ptr);
TDV_PUBLIC long TDVContext_getLong(HContext * ctx, ContextEH ** eh);
TDV_PUBLIC double TDVContext_getDouble(HContext * ctx, ContextEH ** eh);
TDV_PUBLIC bool TDVContext_getBool(HContext * ctx, ContextEH ** eh);
TDV_PUBLIC unsigned char* TDVContext_getDataPtr(HContext * ctx, ContextEH ** eh);



typedef struct HPBlock HPBlock;

TDV_PUBLIC HPBlock* TDVProcessingBlock_createProcessingBlock(const HContext * config, ContextEH ** eh);

TDV_PUBLIC void TDVProcessingBlock_destroyBlock(HPBlock * handle_, ContextEH ** eh);
TDV_PUBLIC void TDVProcessingBlock_processContext(HPBlock * handle_, HContext * config, ContextEH ** eh);

TDV_PUBLIC const char* TDVException_getMessage(ContextEH * eh);
TDV_PUBLIC unsigned int TDVException_getErrorCode(ContextEH * eh);
TDV_PUBLIC void TDVException_deleteException(ContextEH * eh);

}

#endif
