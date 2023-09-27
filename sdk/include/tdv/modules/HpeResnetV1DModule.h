#ifndef HPERESNETV1D_H
#define HPERESNETV1D_H

#include <tdv/modules/ONNXModule.h>

namespace tdv {
namespace modules {


class HpeResnetV1DModule : public ONNXModule<HpeResnetV1DModule>
{
public:
	HpeResnetV1DModule(const tdv::data::Context& config);
private:
	friend class ONNXModule<HpeResnetV1DModule>;

	const bool RAW_OUTPUT;
	std::map<int, std::string> LABEL_MAP;

	static void getEncriptionKey(int64_t &key_data_len, unsigned char const *&key_data, int model_version=1);

	void preprocess(tdv::data::Context& data) override;
	void postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) override;
	std::map<int, std::vector<std::vector<float>>> getOutputData(std::shared_ptr<uint8_t> buff, const tdv::data::Context& meta);

};

}
}

#endif // HPERESNETV1D_H
