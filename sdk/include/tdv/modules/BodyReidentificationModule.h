#ifndef REIDENTIFICATOR_H
#define REIDENTIFICATOR_H

#include <tdv/modules/ONNXModule.h>

namespace tdv {

namespace modules {

class BodyReidentificationModule : public ONNXModule<BodyReidentificationModule>
{
public:
	BodyReidentificationModule(const tdv::data::Context& config);
private:
	friend class ONNXModule<BodyReidentificationModule>;
	void virtual preprocess(tdv::data::Context& data) override;
	void virtual postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) override;
	std::vector<float> getOutputData(std::shared_ptr<uint8_t> buff);
	static void getEncriptionKey(int64_t &key_data_len, unsigned char const* &key_data, int model_version=1);
};


}
}


#endif // REIDENTIFICATOR_H
