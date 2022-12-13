#ifndef MATCHERMODULE_H
#define MATCHERMODULE_H

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <functional>
#include <queue>
#include <memory>


#include <tdv/modules/ProcessingBlock.h>


namespace tdv {

namespace modules {


class MatcherModule : public ProcessingBlock
{
	public:
		MatcherModule(const tdv::data::Context& config);
		virtual void operator ()(tdv::data::Context& data) override;

	private:

		double threshold;
		virtual void verifyMatch(tdv::data::Context& data);
};


}

}

#endif //MATCHERMODULE_H
