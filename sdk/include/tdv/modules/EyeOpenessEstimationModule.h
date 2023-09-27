#ifndef EYEOPENNESSTIMATOR_H
#define EYEOPENNESSTIMATOR_H

#include <tdv/modules/ONNXModule.h>

namespace tdv {

namespace modules {

class EyeOpenessEstimationModule : public ONNXModule<EyeOpenessEstimationModule>
{
public:
    EyeOpenessEstimationModule(const tdv::data::Context& config);
    void operator()(tdv::data::Context& data);
private:
    friend class ONNXModule<EyeOpenessEstimationModule>;
    void virtual preprocess(tdv::data::Context& data) override;
    void virtual postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) override;
    void process(tdv::data::Context& data);
    std::vector<float> getOutputData(std::shared_ptr<uint8_t> buff) const;
    int eye_flag = 0; // 0 == left eye, 1 == right eye


    const double OPNS_THRESH; //0.5693
};

}
}

std::vector<cv::Mat> get_crops_of_eyes(cv::Mat face, cv::Point2f left_eye, cv::Point2f right_eye);

#endif