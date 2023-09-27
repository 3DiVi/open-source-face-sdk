#ifndef PROCESSINGBLOCK_H
#define PROCESSINGBLOCK_H

#include <api/Context.h>


namespace api
{

class Service;

class ProcessingBlock
{
protected:
	HPBlock* handle_;

public:

	ProcessingBlock(ProcessingBlock&& other);

	ProcessingBlock& operator=(ProcessingBlock&& other);

	/**
	 * @brief Infer and put results in ctx
	 * 
	 * @param ctx Results of infer
	 */
	virtual void operator()(Context& ctx);

	virtual ~ProcessingBlock();

protected:
	ProcessingBlock() {};

private:
	ProcessingBlock(const Context& ctx);

	friend class Service;

};


inline ProcessingBlock::ProcessingBlock(ProcessingBlock&& other) {
	handle_ = other.handle_;
	other.handle_ = nullptr;
}

inline ProcessingBlock& ProcessingBlock::operator=(ProcessingBlock&& other) {
	if (&other != this)
	{
		handle_ = other.handle_;
		other.handle_ = nullptr;
	}
	return *this;
}

inline void ProcessingBlock::operator()(Context& ctx) {
	ContextEH* out_exception = nullptr;
	TDVProcessingBlock_processContext(handle_, ctx.getHandle() , &out_exception);
	checkException(out_exception);
}

inline ProcessingBlock::~ProcessingBlock() {
	ContextEH* out_exception = nullptr;
	TDVProcessingBlock_destroyBlock(handle_, &out_exception);
	// N.B. deprecated in c++17 - move to std::uncaught_exceptions()
	if (out_exception && std::uncaught_exception())
		std::cerr << Error(TDVException_getErrorCode(out_exception), TDVException_getMessage(out_exception)).what();
	else
		checkException(out_exception);
}

inline ProcessingBlock::ProcessingBlock(const Context& ctx){
	ContextEH* out_exception = nullptr;
	handle_ = TDVProcessingBlock_createProcessingBlock(ctx.getHandle(), &out_exception);
	checkException(out_exception);
}

}

#endif // PROCESSINGBLOCK_H
