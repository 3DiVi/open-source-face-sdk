#include <cstddef>
#include <stack>
#include <typeindex>
#include <utility>

#include <nlohmann/json.hpp>

#include "tdv/data/JSONSerializer.h"


using NJSON = nlohmann::json;

namespace tdv
{
namespace data
{

inline void jsonifyContext(const Context& ctx, NJSON& retJson);

template<typename Type>
struct PutCtxIntoObject
{
	static void put(NJSON& json, const std::string& field, const Context& ctx)
	{
		if (field.empty())
			json = ctx.get<Type>();
		else
			json[field] = ctx.get<Type>();
	}
};

template<>
struct PutCtxIntoObject<Context>
{
	static void put(NJSON& json, const std::string& field, const Context& ctx)
	{
		if(ctx.isNone())
			if (field.empty())
				json = NJSON::object();
			else
				json[field] = NJSON::object();
		else
			if (field.empty())
				jsonifyContext(ctx, json);
			else
				jsonifyContext(ctx, json[field]);
	}
};

template<typename Type>
struct PutCtxIntoArray
{
	static void put(NJSON& json, const std::string& /*field*/, const Context& ctx)
	{
		if(json.is_null() || json.is_array())
			json.push_back(ctx.get<Type>());
	}
};

template<>
struct PutCtxIntoArray<Context>
{
	static void put(NJSON& json, const std::string& field, const Context& ctx)
	{
		if(json.is_null() || json.is_array())
		{
			if(ctx.isNone())
				json.push_back(NJSON::object());
			else {
				NJSON subJson;
				jsonifyContext(ctx, subJson);
				json += subJson;
			}
		}
	}
};

template <template<typename> class transferFunc>
void putIntoJson(NJSON& json, const std::string& field, const Context& ctx)
{
	if(ctx.isArray() || ctx.isObject() || ctx.isNone())
	{
		transferFunc<Context>::put(json, field, ctx);
	}
	else if(ctx.is<double>())
		transferFunc<double>::put(json, field, ctx);
	else if(ctx.is<float>())
		transferFunc<float>::put(json, field, ctx);
	else if(ctx.is<long>())
		transferFunc<long>::put(json, field, ctx);
	else if(ctx.is<unsigned long>())
		transferFunc<unsigned long>::put(json, field, ctx);
	else if(ctx.is<short>())
		transferFunc<short>::put(json, field, ctx);
	else if(ctx.is<unsigned short>())
		transferFunc<unsigned short>::put(json, field, ctx);
	else if(ctx.is<int>())
		transferFunc<int>::put(json, field, ctx);
	else if(ctx.is<unsigned int>())
		transferFunc<unsigned int>::put(json, field, ctx);
	else if(ctx.is<std::string>())
		transferFunc<std::string>::put(json, field, ctx);
	else if(ctx.is<bool>())
		transferFunc<bool>::put(json, field, ctx);
	else if(ctx.is<unsigned char>())
		transferFunc<unsigned char>::put(json, field, ctx);
	else if(ctx.is<char>())
		transferFunc<char>::put(json, field, ctx);
	else if(ctx.is<NJSON>())
		transferFunc<NJSON>::put(json, field, ctx);
	else if(ctx.is<int64_t>())
		transferFunc<int64_t>::put(json, field, ctx);
	else if(ctx.is<uint64_t>())
		transferFunc<uint64_t>::put(json, field, ctx);
	else if(ctx.is<std::nullptr_t>())
		transferFunc<std::nullptr_t>::put(json, field, ctx);
}


inline void jsonifyContext(const Context& ctx, NJSON& retJson)
{
	if(!ctx.isNone())
	{
		if(ctx.isObject())
		{
			retJson = NJSON::object();
			for(auto iter = ctx.kvcbegin(); iter != ctx.kvcend(); ++iter) {
				putIntoJson<PutCtxIntoObject>(retJson, iter->first, iter->second);
			}
		}
		else if(ctx.isArray())
		{
			retJson = NJSON::array();
			for(auto iter = ctx.cbegin(); iter != ctx.cend(); ++iter) {
				putIntoJson<PutCtxIntoArray>(retJson, "", *iter);
			}
		}
		else
		{
			putIntoJson<PutCtxIntoObject>(retJson, "", ctx);
		}
	}
}

/*warning: recursive*/
std::string JSONSerializer::serialize(const Context& ctx, int indent, char indentChar, bool ensureASCII)
{
	NJSON retJson;
	jsonifyContext(ctx, retJson);
	return retJson.dump(indent, indentChar, ensureASCII);
}


template<typename Type>
struct PutIntoContext
{
	static void insert(const std::string& key, Context& ctx, const Type& data)
	{
		if(key.empty())
			ctx = data;
		else
			ctx[key] = data;
	}
};

template<typename Type>
struct PushBackToContext
{
	static void insert(const std::string& /*key*/, Context& ctx, const Type& data) {
		ctx.push_back(data);
	}
};

template<template<typename> class insertFunc>
inline void toContextFromNJson(const NJSON& subjson, const std::string& key, Context& ctx)
{
	if(subjson.is_number_float())
		insertFunc<double>::insert(key, ctx, subjson.get<double>());
	else if(subjson.is_number_integer())
		insertFunc<int64_t>::insert(key, ctx, subjson.get<int64_t>());
	else if(subjson.is_number_unsigned())
		insertFunc<uint64_t>::insert(key, ctx, subjson.get<uint64_t>());
	else if(subjson.is_boolean())
		insertFunc<bool>::insert(key, ctx, subjson.get<bool>());
	else if(subjson.is_string())
		insertFunc<std::string>::insert(key, ctx, subjson.get<std::string>());
	else if(subjson.is_null())
		insertFunc<std::nullptr_t>::insert(key, ctx, nullptr);
}

inline Context constructContextFromNJson(const NJSON& json)
{
	Context ctx = json.is_array() ? Context::make_array() :
			json.is_object() ? Context::make_object() : Context();

	if(json.empty())
		return ctx;

	if(json.is_primitive()) {
		toContextFromNJson<PutIntoContext>(json, "", ctx);
	}
	else
	{
		ctx = json.is_array() ? Context::make_array() : Context::make_object();
		for(auto it = json.cbegin(); it!=json.cend(); ++it)
		{
			if(it->is_primitive())
			{
				if(json.is_array())
					toContextFromNJson<PushBackToContext>(it.value(), "", ctx);
				else if(json.is_object())
					toContextFromNJson<PutIntoContext>(it.value(), it.key(), ctx);
			}
			else
				if(json.is_array())
					ctx.push_back(constructContextFromNJson(it.value()));
				else
					ctx[it.key()] = constructContextFromNJson(it.value());
		}
	}
	return ctx;
}

/*warning: recursive*/
Context JSONSerializer::deserialize(const std::string& json)
{
	NJSON jsonObj;
	if(json.empty())
		return Context();
	try
	{
		jsonObj = NJSON::parse(json);
	}
	catch(NJSON::exception e)
	{
		throw std::runtime_error(e.what());
	}
	return constructContextFromNJson(jsonObj);
}

} // namespace data
} // namespace tdv
