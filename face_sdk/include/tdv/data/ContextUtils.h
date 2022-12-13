#ifndef TDV_DATA_CONTEXT_V2_CONTEXTUTILS_H_
#define TDV_DATA_CONTEXT_V2_CONTEXTUTILS_H_

#include <opencv2/core/mat.hpp>

#include <tdv/data/Context.h>


namespace tdv
{
namespace data
{

cv::Mat keypointsBasedCrop(Context& data, cv::Mat& image);
cv::Mat keypointsBasedCropOneObject(const Context& data, cv::Mat& image);
void cvMatToBsm(Context& bsmCtx, const cv::Mat& img, bool copy=false);
cv::Mat bsmToCvMat(const Context& bsmCtx, bool copy=false);

} // namespace utils
} // namespace tdv

#endif // TDV_DATA_CONTEXT_V2_CONTEXTUTILS_H_
