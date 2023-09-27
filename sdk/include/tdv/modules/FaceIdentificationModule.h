#ifndef FACEREIDENTIFICATOR_H
#define FACEREIDENTIFICATOR_H

#include <tdv/modules/ONNXModule.h>

namespace tdv {

namespace modules {

class FaceIdentificationModule : public ONNXModule<FaceIdentificationModule>
{
public:
	FaceIdentificationModule(const tdv::data::Context& config);
private:
	friend class ONNXModule<FaceIdentificationModule>;
	void virtual preprocess(tdv::data::Context& data) override;
	void virtual postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) override;
	virtual void operator ()(tdv::data::Context& data) override;
	std::vector<float> getOutputData(std::shared_ptr<uint8_t> buff);
};


}
}


#endif // FACEREIDENTIFICATOR_H
