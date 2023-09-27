#ifndef EMOTIONCLASSIFIER_H
#define EMOTIONCLASSIFIER_H

#include <tdv/modules/ONNXModule.h>
#include <tdv/modules/BaseEstimationModule.h>

namespace tdv {

namespace modules {

class EmotionsEstimationModule : public BaseEstimationModule
{
public:
	EmotionsEstimationModule(const tdv::data::Context& config);
private:
	friend class ONNXModule<EmotionsEstimationModule>;
};


}
}


#endif // EMOTIONCLASSIFIER_H
