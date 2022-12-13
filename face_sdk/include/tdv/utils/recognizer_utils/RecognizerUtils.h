#ifndef TDV_DATA_RECOGNIZER_UTILS_H_
#define TDV_DATA_RECOGNIZER_UTILS_H_

#include <opencv2/core/mat.hpp>
#include <tdv/data/Context.h>


namespace tdv
{
namespace utils
{
namespace recognizer_utils
{

cv::Matx23f makeCrop2ImageByPoints(const tdv::data::Context& fitter, cv::Mat& image, const int base_crop_size);

void warpAffine(cv::Mat &src, cv::Mat &dst, const cv::Matx23f &transform_m_input_, const cv::Size &dsize);

} // recognizer_utils
} // namespace utils
} // namespace tdv

#endif // TDV_DATA_CONTEXT_V2_CONTEXTUTILS_H_
