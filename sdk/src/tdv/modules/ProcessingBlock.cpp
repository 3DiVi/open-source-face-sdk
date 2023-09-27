#include <tdv/modules/ProcessingBlock.h>

#include <tdv/data/JSONSerializer.h>
#include <cstring>
#include <string>

using namespace tdv;
using namespace tdv::data;

struct TDVProcessingBlock
{
	tdv::modules::ProcessingBlock* ptr;
};

void TDVProcessingBlock_destroy(TDVProcessingBlock* block)
{
	delete block->ptr;
	delete block;
}

char* TDVProcessingBlock_processSparse(TDVProcessingBlock* block, char* serializedContext)
{
	Context ctx = _tdv_ProcessingBlock_deserializeConfig(serializedContext); 
	(*block->ptr)(ctx);
	std::string resultString = JSONSerializer::serialize(ctx);
	char* ans = new char[resultString.length() + 1];
	strcpy(ans, resultString.c_str());
	return ans;
}

void tdvFreeStr(char* str) { delete[] str; }

Context _tdv_ProcessingBlock_deserializeConfig(char* serializedConfig)
{
	if (!serializedConfig)
		return Context();
	return JSONSerializer::deserialize(serializedConfig);
}

TDVProcessingBlock* _tdv_ProcessingBlock_wrap(tdv::modules::ProcessingBlock* block)
{
	TDVProcessingBlock* ans = new TDVProcessingBlock;
	ans->ptr = block;
	return ans;
}
