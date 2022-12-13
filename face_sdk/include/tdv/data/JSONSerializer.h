#ifndef TDV_DATA_CONTEXT_V2_JSONSERIALIZER_H
#define TDV_DATA_CONTEXT_V2_JSONSERIALIZER_H

#include <tdv/data/Context.h>

namespace tdv
{
namespace data
{

class JSONSerializer
{
public:
	static std::string serialize(const Context&, int indent = -1, char indentChar = ' ', bool ensureASCII = false);
	static Context deserialize(const std::string&);
};

} // namespace data
} // namespace tdv

#endif // TDV_DATA_CONTEXT_V2_JSONSERIALIZER_H
