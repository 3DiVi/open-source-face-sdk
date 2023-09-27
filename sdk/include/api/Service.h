#ifndef TDV_SERVICE_H
#define TDV_SERVICE_H

#include <api/ProcessingBlock.h>


namespace api
{

/**
 * @brief Provides ProcessingBlock and Context creation
 * 
 */
class Service
{

public:
	/**
	 * @brief Create a ProcessingBlock object
	 * 
	 * @param ctx Config context with unit_type
	 * @return ProcessingBlock 
	 */
	ProcessingBlock createProcessingBlock(const Context& ctx);

	/**
	 * @brief Create a Context object
	 * 
	 * @return Context 
	 */
	Context createContext();
	
	/**
	 * @brief Create a Service object
	 * 
	 * @param path_to_dir Path to directory with data/models
	 * @return Service 
	 */
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
