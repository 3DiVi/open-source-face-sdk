#ifndef TDV_SERVICE_H
#define TDV_SERVICE_H

#include <api/ProcessingBlock.h>


namespace api
{

class Service
{

public:
	ProcessingBlock createProcessingBlock(const Context& ctx);
	Context createContext();
	static Service createService(std::string path_to_dir);

private:
	Service(std::string path_to_dir):path_to_dir(path_to_dir){}
	std::string path_to_dir;
};

inline ProcessingBlock Service::createProcessingBlock(const Context& ctx) {
	Context new_ctx = ctx;
	new_ctx["@sdk_path"] = path_to_dir;
	return ProcessingBlock(new_ctx);
}

inline Service Service::createService(std::string path_to_dir)
{
	return Service(path_to_dir);
}


inline Context Service::createContext() {
	return Context();
}

}

#endif // TDV_SERVICE_H
