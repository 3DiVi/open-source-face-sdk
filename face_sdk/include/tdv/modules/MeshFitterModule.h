#ifndef MESHFITTER_H
#define MESHFITTER_H

#include <tdv/modules/ONNXModule.h>

namespace tdv {

namespace modules {

class MeshFitterModule : public ONNXModule<MeshFitterModule>
{
public:
	MeshFitterModule(const tdv::data::Context& config);
private:
	friend class ONNXModule<MeshFitterModule>;
	void virtual preprocess(tdv::data::Context& data) override;
	void virtual postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) override;
	std::vector<float> getOutputData(std::shared_ptr<uint8_t> buff) const;
	void objectFromPredict(std::vector<float> &predict, tdv::data::Context& obj, const int INPUT_SIZE);

	std::vector<int> r_idx { 385, 387, 380, 373 };
	std::vector<int> l_idx { 160, 158, 144, 153 };
	std::vector<int> mouth_idx { 13, 14 };
};


}
}


#endif // GENDERNEWESTIMATION_H
