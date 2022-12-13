#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <iomanip>
#include <memory>
#include <sstream>

#include <api/c_api.h>


namespace api
{

class Error : public std::exception
{
public:

	virtual ~Error() noexcept
	{
		// nothing
	}

//! @cond IGNORED

	Error(const uint32_t code, const std::string what)
	{
		code_ = code;
		std::stringstream ss;
		ss << "errcode 0x" << std::setfill('0') << std::setw(2) << std::hex
			<< code << " : " << what << std::endl;
		what_ = ss.str();
	}

	virtual const char* what() const noexcept
	{
		return what_.c_str();
	}

	uint32_t code() const noexcept
	{
		return code_;
	}

private:
	uint32_t code_;
	std::string what_;
};

inline void checkException(ContextEH*& out_exception) {
	if(out_exception)
	{
		auto err = Error(TDVException_getErrorCode(out_exception), TDVException_getMessage(out_exception));
		TDVException_deleteException(out_exception);
		out_exception = nullptr;
		throw err;
	}
}

}

#endif // EXCEPTION_H
