#include <tdv/modules/BaseEstimationModule.h>


namespace tdv {

namespace modules {


BaseEstimationModule::BaseEstimationModule(const tdv::data::Context& config)
{}

void BaseEstimationModule::operator ()(tdv::data::Context& data) {
	if (data.contains("objects")){
		for(int i = 0; i < data["objects"].size(); i++){
			data["objects@current_id"] = i;
			(*block)(data);
		}
		data.erase("objects@current_id");
	}else{
		(*block)(data);
	}
}


}
}
