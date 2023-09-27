#ifndef LIVENESSDETECTOR_H
#define LIVENESSDETECTOR_H

#include <tdv/modules/ONNXModule.h>
#include <tdv/modules/LivenessDetectionModule/LivenessBaseModule.h>

namespace tdv {

namespace modules {

class LivenessDetectionModule: public ProcessingBlock
{
public:
	LivenessDetectionModule(const tdv::data::Context& config);
	void operator()(tdv::data::Context& data);
private:

	void process(tdv::data::Context& data);
	std::shared_ptr<tdv::modules::LivenessBaseModule> model1;
	std::shared_ptr<tdv::modules::LivenessBaseModule> model2;
	const float scales[2] = {2.7, 4.0};
	std::vector<double> result_predict;

	int curr_face_id;
	const double LIVENESS_THRESH; //0.9

};

}
}

#endif