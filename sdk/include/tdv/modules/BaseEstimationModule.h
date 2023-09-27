#ifndef BASE_ESTIMATION_H
#define BASE_ESTIMATION_H

#include <memory>

#include <tdv/data/Context.h>
#include <tdv/modules/ProcessingBlock.h>


namespace tdv {

namespace modules {

class BaseEstimationModule : public ProcessingBlock
{
public:
	BaseEstimationModule(const tdv::data::Context& config);
	virtual void operator ()(tdv::data::Context& data) override;

protected:
	virtual void process(tdv::data::Context& data) {return;}
	std::unique_ptr<ProcessingBlock> block;

	int module_version = 0;
};


}
}


#endif // BASE_ESTIMATION_H
