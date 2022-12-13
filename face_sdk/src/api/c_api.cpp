#include <cstring>
#include <stdexcept>

#include <tdv/data/Context.h>
#include <tdv/modules/DetectionModules/FaceDetectionModule.h>
#include <tdv/modules/MeshFitterModule.h>
#include <tdv/modules/FaceIdentificationModule.h>
#include <tdv/modules/MatcherModule.h>
#include <api/c_api.h>
#include <tdv/utils/rassert/RAssert.h>


#define CreatePB(x) \
	if (!ctx.get<std::string>("model_path", "").compare("")) \
		new_ctx["model_path"] = ctx["@sdk_path"].get<std::string>() + unitTypes.at(ctx["unit_type"].get<std::string>()); \
	handle_ = new internal::x(new_ctx); \
	return reinterpret_cast<HPBlock*>(handle_);


const std::map<std::string, std::string> unitTypes {
	{"FACE_DETECTOR", "/data/models/face_detector/face.onnx"},
	{"FACE_RECOGNIZER", "/data/models/recognizer/recognizer.onnx"},
	{"MESH_FITTER", "/data/models/mesh_fitter/mesh_fitter.onnx"},
	{"MATCHER_MODULE", ""},
};

namespace api {

namespace internal
{
	using Context = ::tdv::data::Context;
	using Error = ::tdv::utils::rassert::tdv_error;
	using namespace tdv::modules;
}

struct ContextEH {
	std::unique_ptr<internal::Error> ptr;
	const void* userdata;
	ContextEH(internal::Error* ptr, void* data=nullptr) :
		ptr(ptr), userdata(data) {}
};

const static size_t MAX_STR_SIZE=65535;

TDV_PUBLIC HContext* TDVContext_create(ContextEH ** eh)
{
	try {
		return reinterpret_cast<HContext*>(new internal::Context());
	} catch (std::exception& e) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x56234801, e.what()),
							nullptr);
		return nullptr;
	}
}

TDV_PUBLIC void TDVContext_destroy(HContext* ctx, ContextEH ** eh)
{
	try {
		delete reinterpret_cast<internal::Context*>(ctx);
	} catch (std::exception& e) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x2dc17586, e.what()),
							nullptr);
	}
}

TDV_PUBLIC HContext* TDVContext_getByIndex(HContext * ctx, const int index, ContextEH ** eh)
{
	try {
		return reinterpret_cast<HContext*>(&(reinterpret_cast<internal::Context*>(ctx)->operator[](index)));
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x6ba98764, e.what()),
							nullptr);
		return nullptr;
	}
}

TDV_PUBLIC HContext* TDVContext_getOrInsertByKey(HContext * ctx, const char* key, ContextEH ** eh)
{
	try {
		return reinterpret_cast<HContext*>(&(reinterpret_cast<internal::Context*>(ctx)->operator[](key)));
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0xcb1ec66f, e.what()),
							nullptr);
		return nullptr;
	}
}

TDV_PUBLIC HContext* TDVContext_getByKey(HContext * ctx, const char* key, ContextEH ** eh)
{
	try {
		return reinterpret_cast<HContext*>(&(reinterpret_cast<internal::Context*>(ctx)->at(key)));
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0xb2a4ab43, e.what()),
							nullptr);
		return nullptr;
	}
}

TDV_PUBLIC HContext* TDVContext_clone(HContext * ctx, ContextEH ** eh)
{
	try {
		return reinterpret_cast<HContext*>(new internal::Context(*reinterpret_cast<internal::Context*>(ctx)));
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0xf52416f4, e.what()),
							nullptr);
		return nullptr;
	}
}

TDV_PUBLIC void TDVContext_copy(HContext * src, HContext * dst, ContextEH ** eh)
{
	try {
		*reinterpret_cast<internal::Context*>(dst) = *reinterpret_cast<internal::Context*>(src);
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x06e0b1ce, e.what()),
							nullptr);
	}
}

TDV_PUBLIC void TDVContext_clear(HContext * ctx, ContextEH ** eh)
{
	try {
		return reinterpret_cast<internal::Context*>(ctx)->clear();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x06e0b1ce, e.what()),
							nullptr);
	}
}

TDV_PUBLIC void TDVContext_putStr(HContext* ctx, const char* str, ContextEH ** eh)
{
	try {
		if (memchr(str, '\0', MAX_STR_SIZE))
			*reinterpret_cast<internal::Context*>(ctx) = std::string(str);
		else
			throw std::runtime_error("arg is not null terminated c-string or longer then MAX_STR_SIZE");
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0xaa600d43, e.what()),
							nullptr);
	}
}


TDV_PUBLIC void TDVContext_putLong(HContext * ctx, long val, ContextEH ** eh)
{
	try {
		*reinterpret_cast<internal::Context*>(ctx) = val;
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x88df0572, e.what()),
							nullptr);
	}
}

TDV_PUBLIC void TDVContext_putDouble(HContext * ctx, double val, ContextEH ** eh)
{
	try {
		*reinterpret_cast<internal::Context*>(ctx) = val;
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x75a049e2, e.what()),
							nullptr);
	}
}

TDV_PUBLIC void TDVContext_putBool(HContext * ctx, bool val, ContextEH ** eh)
{
	try {
		*reinterpret_cast<internal::Context*>(ctx) = val;
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0xccc7e754, e.what()),
							nullptr);
	}
}

TDV_PUBLIC void TDVContext_freePtr(void * ptr)
{
	free(ptr);
}

TDV_PUBLIC unsigned char* TDVContext_allocDataPtr(HContext * ctx, unsigned long size, ContextEH ** eh)
{
	unsigned char* data = nullptr;
	try {
		if(size)
		{
			data = static_cast<unsigned char*>(malloc(size));
			if(data)
				*reinterpret_cast<internal::Context*>(ctx) = std::shared_ptr<unsigned char>(data, TDVContext_freePtr);
			else
				throw std::bad_alloc();
		}
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x4655ae44, e.what()),
						nullptr);
	}
	return data;
}


TDV_PUBLIC unsigned char* TDVContext_putDataPtr(HContext * ctx, unsigned char* val, unsigned long copy_sz, ContextEH ** eh)
{
	unsigned char* data = nullptr;
	try {
		if(copy_sz && val)
		{
			data = static_cast<unsigned char*>(malloc(copy_sz));
			if(data)
				std::memcpy(data, val, copy_sz);
			else
				throw std::bad_alloc();
		}
		else
			data = val;

		if(data)
		{
			if(copy_sz)
				// in C++17, shared_ptr can be used to manage a dynamically allocated array
				// data = new unsigned char[copy_sz];
				// std::shared_ptr<unsigned char[]>(data);
				*reinterpret_cast<internal::Context*>(ctx) = std::shared_ptr<unsigned char>(data, TDVContext_freePtr);
			else
				*reinterpret_cast<internal::Context*>(ctx) = std::shared_ptr<unsigned char>(data, [](unsigned char*){});
		}
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x4c551e44, e.what()),
						nullptr);
	}
	return data;
}

TDV_PUBLIC unsigned char* TDVContext_putConstDataPtr(HContext * ctx, const unsigned char* val, unsigned long copy_sz, ContextEH ** eh)
{
	unsigned char* data = nullptr;
	try {
		if(!copy_sz)
			throw std::runtime_error("const data should be copied but copy size is zero");
		else if(val)
		{
			data = static_cast<unsigned char*>(malloc(copy_sz));
			if(data)
				std::memcpy(data, val, copy_sz);
			else
				throw std::bad_alloc();
			*reinterpret_cast<internal::Context*>(ctx) = std::shared_ptr<unsigned char>(data, TDVContext_freePtr);
		}
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x4c551e44, e.what()), nullptr);
	}
	return data;
}

TDV_PUBLIC unsigned long TDVContext_getLength(HContext * ctx, ContextEH ** eh)
{
	try {
		return reinterpret_cast<internal::Context*>(ctx)->size();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0xd45f61aa, e.what()),
							nullptr);
		return 0;
	}
}

TDV_PUBLIC char** TDVContext_getKeys(HContext * ctx, unsigned long length, ContextEH ** eh)
{
	char** buff = nullptr;
	try {
		buff = (char**)malloc(length*sizeof(char*));
		internal::Context* cMap = reinterpret_cast<internal::Context*>(ctx);
		size_t index{0};
		for(auto keyValuePair = cMap->kvbegin(); (keyValuePair != cMap->kvend()) && (index < length); ++keyValuePair, ++index)
		{
			const std::string& key = (*keyValuePair).first;
			char * copy = (char*)malloc(strlen(key.c_str()) + 1);
			strcpy(copy, key.c_str());
			buff[index] = copy;
		}
		if (index < length)
			*eh = new ContextEH(new internal::Error(0xdada331a, "length exceeds current size"), nullptr);
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0xd48f61aa, e.what()), nullptr);
		return nullptr;
	}
	return buff;
}

TDV_PUBLIC bool TDVContext_isNone(HContext * ctx, ContextEH ** eh)
{
	try {
		return reinterpret_cast<internal::Context*>(ctx)->isNone();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x96fac41a, e.what()),
							nullptr);
		return 0;
	}
}

TDV_PUBLIC bool TDVContext_isArray(HContext * ctx, ContextEH ** eh)
{
	try {
		return reinterpret_cast<internal::Context*>(ctx)->isArray();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x96fac41b, e.what()),
							nullptr);
		return 0;
	}
}

TDV_PUBLIC bool TDVContext_isObject(HContext * ctx, ContextEH ** eh)
{
	try {
		return reinterpret_cast<internal::Context*>(ctx)->isObject();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x96fac41c, e.what()),
							nullptr);
		return 0;
	}
}

TDV_PUBLIC bool TDVContext_isBool(HContext * ctx, ContextEH ** eh)
{
	try {
		return reinterpret_cast<internal::Context*>(ctx)->is<bool>();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x96fac42c, e.what()),
							nullptr);
		return 0;
	}
}

TDV_PUBLIC bool TDVContext_isLong(HContext * ctx, ContextEH ** eh)
{
	try {
		return reinterpret_cast<internal::Context*>(ctx)->is<long>();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x96fac43c, e.what()),
							nullptr);
		return 0;
	}
}

TDV_PUBLIC bool TDVContext_isDouble(HContext * ctx, ContextEH ** eh)
{
	try {
		return reinterpret_cast<internal::Context*>(ctx)->is<double>();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x96fac44c, e.what()),
							nullptr);
		return 0;
	}
}

TDV_PUBLIC bool TDVContext_isString(HContext * ctx, ContextEH ** eh)
{
	try {
		return reinterpret_cast<internal::Context*>(ctx)->is<std::string>();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x96fac45c, e.what()),
							nullptr);
		return 0;
	}
}

TDV_PUBLIC bool TDVContext_isDataPtr(HContext * ctx, ContextEH ** eh)
{
	try {
		return reinterpret_cast<internal::Context*>(ctx)->is<std::shared_ptr<unsigned char>>();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x96fac46c, e.what()),
							nullptr);
		return 0;
	}
}

TDV_PUBLIC const char* TDVContext_getStr(HContext * ctx, char* buff, ContextEH ** eh)
{
	try {
		std::string& data = reinterpret_cast<internal::Context*>(ctx)->as<std::string>();
		if(buff)
		{
			data.copy(buff, data.length());
		}
		else
		{
			buff = static_cast<char*>(malloc(data.length() + 1));
			if(buff)
				strcpy(buff, data.c_str());
			else
				throw std::bad_alloc();
		}
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x9097d24a, e.what()),
							nullptr);
	}
	return buff;
}

TDV_PUBLIC unsigned long TDVContext_getStrSize(HContext * ctx, ContextEH ** eh)
{
	unsigned long size_s;
	try {
		size_s = reinterpret_cast<internal::Context*>(ctx)->as<std::string>().length();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x8fc91369, e.what()),
							nullptr);
	}
	return size_s;
}

TDV_PUBLIC long TDVContext_getLong(HContext * ctx, ContextEH ** eh)
{
	long data;
	try {
		internal::Context* i_ctx = reinterpret_cast<internal::Context*>(ctx);
		if(i_ctx->is<int>())
			data = i_ctx->get<int>();
		else
			data = i_ctx->get<long>();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x2353ead7, e.what()),
							nullptr);
	}
	return data;
}

TDV_PUBLIC double TDVContext_getDouble(HContext * ctx, ContextEH ** eh)
{
	double data;
	try {
		data = reinterpret_cast<internal::Context*>(ctx)->get<double>();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0xd4fbd56c, e.what()),
							nullptr);
	}
	return data;
}

TDV_PUBLIC bool TDVContext_getBool(HContext * ctx, ContextEH ** eh)
{
	bool data;
	try {
		data = reinterpret_cast<internal::Context*>(ctx)->get<bool>();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0xfe8d07c6, e.what()),
							nullptr);
	}
	return data;
}

TDV_PUBLIC unsigned char* TDVContext_getDataPtr(HContext * ctx, ContextEH ** eh)
{
	unsigned char* data = nullptr;
	try {
		data = reinterpret_cast<internal::Context*>(ctx)->get<std::shared_ptr<unsigned char>>().get();
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x51228370, e.what()),
							nullptr);
	}
	return data;
}

TDV_PUBLIC void TDVContext_pushBack(HContext * handle_, HContext * data, bool copy, ContextEH ** eh)
{
	try {
		if(copy)
			reinterpret_cast<internal::Context*>(handle_)->push_back(*reinterpret_cast<internal::Context*>(data));
		else
			reinterpret_cast<internal::Context*>(handle_)->push_back(std::move(*reinterpret_cast<internal::Context*>(data)));
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x6b8d124a, e.what()),
							nullptr);
	}
}

TDV_PUBLIC HPBlock* TDVProcessingBlock_createProcessingBlock(const HContext * config, ContextEH ** eh) {
	try {
		const internal::Context &ctx = *reinterpret_cast<const internal::Context*>(config);
		internal::Context new_ctx = ctx;
		internal::ProcessingBlock* handle_;
		std::string unit_type = ctx.get<std::string>("unit_type", "");

		if (unit_type == ""){
			throw std::invalid_argument("not unit_type");
		}else if ( unit_type == "FACE_DETECTOR"){
			CreatePB(FaceDetectionModule);
		}else if (unit_type == "MESH_FITTER"){
			CreatePB(MeshFitterModule);
		}else if(unit_type == "FACE_RECOGNIZER"){
			CreatePB(FaceIdentificationModule);
		}else if (unit_type == "MATCHER_MODULE"){
			CreatePB(MatcherModule);
		}else{
			throw std::invalid_argument("not correct unit_type");
		}
	} catch (std::exception& e) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x10d504a0, e.what()), nullptr);
		return nullptr;
	}
}

TDV_PUBLIC void TDVProcessingBlock_destroyBlock(HPBlock * handle_, ContextEH ** eh) {
	try {
		delete reinterpret_cast<internal::ProcessingBlock*>(handle_);
	} catch (std::exception& e) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0xfa7b73ff, e.what()),
							nullptr);
	}
}

TDV_PUBLIC void TDVProcessingBlock_processContext(HPBlock * handle_, HContext * ctx, ContextEH ** eh) {
	try {
		reinterpret_cast<internal::ProcessingBlock*>(handle_)->operator()(*reinterpret_cast<internal::Context*>(ctx));
	} catch (std::exception& e ) {
		if(!eh) throw;
		*eh = new ContextEH(new internal::Error(0x9398017a, e.what()),
							nullptr);
	}
}

TDV_PUBLIC const char* TDVException_getMessage(ContextEH * eh) {
	if (eh && eh->ptr)
		return eh->ptr->what();
	else throw std::invalid_argument("nullptr error handler");
}

TDV_PUBLIC unsigned int TDVException_getErrorCode(ContextEH * eh) {
	if (eh && eh->ptr)
		return eh->ptr->code();
	else throw std::invalid_argument("nullptr error handler");
}

TDV_PUBLIC void TDVException_deleteException(ContextEH * eh) {
	if (eh)
		delete eh;
}

}
