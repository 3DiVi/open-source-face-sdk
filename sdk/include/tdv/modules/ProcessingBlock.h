#ifndef TDV_PROCESSINGBLOCK_H_
#define TDV_PROCESSINGBLOCK_H_

#ifdef _WIN32
#define TDV_PUBLIC extern "C" __declspec(dllexport)
#else
#define TDV_PUBLIC extern "C" __attribute__ ((visibility ("default")))
#endif

#ifdef __cplusplus

#include <memory>
#include <tdv/data/Context.h>


namespace tdv
{

namespace modules
{

class ProcessingBlock
{
public:
	using Ptr = std::shared_ptr<ProcessingBlock>;

	ProcessingBlock(const data::Context& config = data::Context()) { }
	virtual ~ProcessingBlock() = default;
	virtual void operator ()(data::Context&) = 0;
};

} // namespace modules

} // namespace tdv

extern "C"
{
#endif // __cplusplus


typedef struct TDVProcessingBlock TDVProcessingBlock;
TDV_PUBLIC void TDVProcessingBlock_destroy(TDVProcessingBlock*);

enum TDVImagePixelFormat
{
	TDV_RGB24,
	TDV_RGBA32,
	TDV_GRAY8
};

TDV_PUBLIC char* TDVProcessingBlock_processSparse(TDVProcessingBlock*, char* serializedContext);
TDV_PUBLIC void tdvFreeStr(char*);

#ifdef __cplusplus
} // extern "C"
#endif


TDV_PUBLIC TDVProcessingBlock* _tdv_ProcessingBlock_wrap(tdv::modules::ProcessingBlock*);
TDV_PUBLIC tdv::data::Context _tdv_ProcessingBlock_deserializeConfig(char*);

#define TDV_DECLARE_BLOCK_C(Block) \
	TDV_PUBLIC TDVProcessingBlock* TDV##Block##_createByConfig(char* serializedConfig);

#ifndef __cplusplus
#	define TDV_DECLARE_BLOCK(Block) TDV_DECLARE_BLOCK_C(Block)
#else

#	define TDV_DECLARE_BLOCK(Block) \
		extern "C" \
		{ \
			TDV_DECLARE_BLOCK_C(Block) \
		}

#	define TDV_DEFINE_BLOCK(Block) \
		extern "C" \
		{ \
			TDV_PUBLIC TDVProcessingBlock* TDV##Block##_createByConfig(char* serializedConfig) \
			{ return _tdv_ProcessingBlock_wrap(new Block(_tdv_ProcessingBlock_deserializeConfig(serializedConfig))); } \
		}

#endif // __cplusplus

#endif  // TDV_PROCESSINGBLOCK_H_
