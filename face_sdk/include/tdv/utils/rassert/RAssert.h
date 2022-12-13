#ifndef TDV_UTILS_RASSERT_H
#define TDV_UTILS_RASSERT_H

#include <stdexcept>
#include <string>


#ifdef __FSDK_ASSERTS_ALWAYS_LOG__
#include <iostream>
#include <sstream>
#endif


namespace tdv
{
namespace utils
{
namespace rassert
{


class tdv_error : public std::exception
{
public:

	virtual ~tdv_error() throw()
	{
		// nothing
	}

	tdv_error(unsigned int code, const char* what) :
		_code(code),
		_what(what)
	{
		#ifdef __FSDK_ASSERTS_ALWAYS_LOG__
		std::ostringstream oss;
		oss << "\n\n the Error was thrown '" << _what << "' code: 0x" << std::hex << code << " \n\n";
		std::cerr << oss.str();
		#endif
	}

	tdv_error(unsigned int code, std::string what) :
		_code(code),
		_what(what)
	{
		#ifdef __FSDK_ASSERTS_ALWAYS_LOG__
		std::ostringstream oss;
		oss << "\n\n the Error was thrown '" << _what << "' code: 0x" << std::hex << code << " \n\n";
		std::cerr << oss.str();
		#endif
	}

	virtual const char* what() const throw()
	{
		return _what.c_str();
	}

	unsigned int code() const throw()
	{
		return _code;
	}

	size_t getWhatLength() const throw()
	{
		return _what.length();
	}

private:
	unsigned int _code;
	std::string _what;
};


#ifdef __FSDK_ASSERTS_WITH_SRC_INFO__

#define __LINE_STRINGIZE(x) __LINE_STRINGIZE2(x)
#define __LINE_STRINGIZE2(x) #x
#define __LINE_STRING__ __LINE_STRINGIZE(__LINE__)

#define __PRIVATE_DEBUG_INFO__ " [ " __FILE__ " : " __LINE_STRING__ " ] "

#else

#define __PRIVATE_DEBUG_INFO__

#endif


// NOTE: for the code use unique unsigned integer number (run uuidgen ang cut 8 symbols)
//       code must be writen in hexadecimal like 0x12345678, so it will be easy to find

#define RCAssert( code, expr ) \
	do  \
	{  \
		if(!(expr))  \
		{ \
			throw tdv::utils::rassert::tdv_error( \
				code,  \
				"Assertion '" #expr "' failed, error code: " #code "." __PRIVATE_DEBUG_INFO__);  \
		}  \
	} while(0)


#define RHAssert( code, expr ) \
	do  \
	{  \
		if(!(expr))  \
		{ \
			throw tdv::utils::rassert::tdv_error( \
				code,  \
				"Assertion failed, error code: " #code "." __PRIVATE_DEBUG_INFO__);  \
		}  \
	} while(0)


#define RCAssert3( code, expr, description, possible_solution )  \
	do  \
	{  \
		if(!(expr))  \
		{  \
			throw tdv::utils::rassert::tdv_error(  \
				code,  \
				"Assertion '" #expr "' failed (" + std::string(description)+ " " + std::string(possible_solution) + \
					"), error code: " #code "." __PRIVATE_DEBUG_INFO__);  \
		}  \
	} while(0)


#define RFAssert3( code, expr, filepath, possible_solution )  \
	do  \
	{  \
		if(!(expr))  \
		{  \
			throw tdv::utils::rassert::tdv_error(  \
				code,  \
				"Assertion '" #expr "' failed (An error occurred while opening the file: " + \
					std::string(filepath) + ". System error description: " + std::string(strerror(errno)) +  \
					". " + #possible_solution + ") Error code: " #code "." __PRIVATE_DEBUG_INFO__);  \
		}  \
	} while(0)


#define RHAssert2( code, expr, description )  \
	do  \
	{  \
		if(!(expr))  \
		{  \
			throw tdv::utils::rassert::tdv_error(  \
				code,  \
				"Assertion failed (" + std::string(description) +  \
					"), error code: " #code "." __PRIVATE_DEBUG_INFO__);  \
		}  \
	} while(0)

#define RCError( code, description) \
	do \
	{ \
		throw tdv::utils::rassert::tdv_error( \
			code, \
			"Error '" + std::string(description) + "', error code: " #code "." \
			__PRIVATE_DEBUG_INFO__); \
	} while(0)


// the same but without error code - for compatibility with old code
#define RAssert( expr ) RCAssert(0x00000000, expr)
#define RAssert2( expr, description ) RCAssert2(0x00000000, expr, description)
#define RCAssert2( code, expr, description ) RCAssert3(code, expr, description, "")
#define RError( not_used, description) RCError(0x00000000, description)
#define RHError( code, description ) RCError( code, description)
#define RFAssert2( code, expr, filepath ) RFAssert3(code, expr, filepath, "")

}  // namespace rassert
}  // namespace utils
}  // namespace tdv

#endif
