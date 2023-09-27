#ifndef API_CONTEXT_H
#define API_CONTEXT_H

#include <iostream>
#include <memory>
#include <string>


#ifdef __cplusplus
extern "C" {
#endif
#include <api/c_api.h>
#ifdef __cplusplus
}
#endif

#include <api/Exception.h>


namespace api
{

class Service;

class ContextRef;

class Context
{
	friend class Service;
public:
	typedef ContextRef Ref;

	template<bool isConst=false>
	class ContextArrayIterator
	{

		class charsList {
		public:
			size_t length_;
			char** ptr_;
			charsList(size_t length, char** ptr) : length_(length), ptr_(ptr) {}
			char* operator[](size_t index) const {
				return (index<length_) ? *(ptr_+index) : nullptr;
			}
		};

		const Context& base_;
		size_t length_;
		size_t index_;
		Context* curr_;
		const bool isObj_;
		std::shared_ptr<charsList> keys_;

		static void charsList_deleter(charsList* ptr) {
			for(size_t indx=0; indx < ptr->length_; ++indx)
				TDVContext_freePtr(ptr->ptr_[indx]);
			TDVContext_freePtr(ptr->ptr_);
		}

	public:
		typedef std::ptrdiff_t																difference_type;
		typedef std::forward_iterator_tag													iterator_category;
		typedef Context																		value_type;
		typedef typename std::conditional<isConst, const value_type*, value_type*>::type	pointer;
		typedef typename std::conditional<isConst, const value_type&, value_type&>::type	reference;
		typedef const value_type&															const_reference;

		ContextArrayIterator(const Context& ctx, int64_t index);
		ContextArrayIterator(const Context& ctx, const std::string& key);

		ContextArrayIterator(const ContextArrayIterator& iter);
		ContextArrayIterator& operator=(const ContextArrayIterator& iter) = delete;
		ContextArrayIterator(ContextArrayIterator&& iter);
		ContextArrayIterator& operator=(ContextArrayIterator&& iter) = delete;


		~ContextArrayIterator() {
			if(curr_)
				delete curr_;
		}

		bool operator==(const ContextArrayIterator& other) const {
			return ((this->base_ == other.base_) && (this->curr_ == other.curr_));
		}

		bool operator!=(const ContextArrayIterator& other) const {
			return (!(this->base_ == other.base_) || !(this->curr_ == other.curr_));
		}

		ContextArrayIterator& operator++();
		ContextArrayIterator operator++(int);

		reference operator*() {
			return *curr_;
		}

		pointer operator->() {
			return curr_;
		}

		std::string key() const {
			if (keys_ && index_ < length_)
				return std::string(keys_->operator[](index_));
			return std::string();
		}
	};

	virtual ~Context();

	Context(const Context& other);
	Context& operator=(const Context& other);

	operator Context::Ref();

	Context(Context&& other);
	Context& operator=(Context&& other);

	template <typename T, typename = typename std::enable_if<!std::is_base_of<Context, typename std::decay<T>::type>::value>::type>
	explicit Context(T&& value);

	template <typename T, typename = typename std::enable_if<!std::is_base_of<Context, typename std::decay<T>::type>::value>::type>
	Context& operator=(T&& value);

	bool operator==(const Context& other) const;

	/**
	 * @brief Get array Context size
	 * 
	 * @return Context array size
	 */
	size_t size() const;

	/**
	 * @brief Check Context value is None
	 * 
	 * @return Is Context None 
	 */
	bool isNone() const;

	/**
	 * @brief Check Context value is array
	 * 
	 * @return Is Context array
	 */
	bool isArray() const;

	/**
	 * @brief Check Context value is object
	 * 
	 * @return Is Context object
	 */
	bool isObject() const;

	ContextArrayIterator<> begin();
	ContextArrayIterator<> end();

	ContextArrayIterator<true> begin() const;
	ContextArrayIterator<true> end() const;

	ContextArrayIterator<true> cbegin() const;
	ContextArrayIterator<true> cend() const;

	/**
	 * @brief Get Context from map by key
	 * 
	 * @param key Context key
	 * @return Context 
	 */
	Context operator[](const std::string& key);

	/**
	 * @brief Get Context from map by key
	 * 
	 * @param key Context key
	 * @return Context 
	 */
	Context operator[](const char* key);

	/**
	 * @brief Get Context from array by index
	 * 
	 * @param index Array index
	 * @return Context 
	 */
	Context operator[](const int index);

	/**
	 * @brief Get Context from map by key
	 * 
	 * @param key Context key
	 * @return Context 
	 */
	const Context operator[](const std::string& key) const;

	/**
	 * @brief Get Context from map by key
	 * 
	 * @param key Context key
	 * @return Context 
	 */
	const Context operator[](const char* key) const;

	/**
	 * @brief Get Context from array by index
	 * 
	 * @param index Array index
	 * @return Context 
	 */
	const Context operator[](const int index) const;

	/**
	 * @brief Get Context from map by key
	 * 
	 * @param key Context key
	 * @return Context 
	 */
	Context at(const char* key);

	/**
	 * @brief Get Context from map by key
	 * 
	 * @param key Context key
	 * @return Context 
	 */
	Context at(const std::string& key);

	/**
	 * @brief Get Context from array by index
	 * 
	 * @param index Array index
	 * @return Context 
	 */
	Context at(const int index);

	/**
	 * @brief Get Context from map by key
	 * 
	 * @param key Context key
	 * @return Context 
	 */
	const Context at(const char* key) const;

	/**
	 * @brief Get Context from map by key
	 * 
	 * @param key Context key
	 * @return Context 
	 */
	const Context at(const std::string& key) const;

	/**
	 * @brief Get Context from array by index
	 * 
	 * @param index Array index
	 * @return Context 
	 */
	const Context at(const int index) const;

	/**
	 * @brief Check is Context with key contains in Context map
	 * 
	 * @param key Context key
	 * @return Is Context with key contains in Context map
	 */
	bool contains(const std::string& key) const;

	/**
	 * @brief Clear Context value
	 * 
	 */
	void clear();

	/**
	 * @brief Find Context by key in Context map
	 * 
	 * @param key Context key
	 * @return Iterator to Context
	 */
	ContextArrayIterator<> find(const std::string& key);

	/**
	 * @brief Find Context by key in Context map
	 * 
	 * @param key Context key
	 * @return Const iterator to Context
	 */
	ContextArrayIterator<true> find(const std::string& key) const;

	/**
	 * @brief Add Context to Array
	 * 
	 * @param data New Context
	 */
	template<typename T>
	typename std::enable_if<!std::is_base_of<Context, typename std::decay<T>::type>::value>::type push_back(T&& data);

	/**
	 * @brief Add Context to Array
	 * 
	 * @param data New Context
	 */
	void push_back(const Context& data);

	/**
	 * @brief Add Context to Array
	 * 
	 * @param data New Context
	 */
	void push_back(Context&& data);

	/**
	 * @brief Get Context value
	 * 
	 * @return Bool value
	 */
	bool getBool() const;

	/**
	 * @brief Get Context value
	 * 
	 * @return Long value
	 */
	int64_t getLong() const;

	/**
	 * @brief Get Context value
	 * 
	 * @return Unsigned long value
	 */
	uint64_t getUnsignedLong() const;

	/**
	 * @brief Get Context value
	 * 
	 * @return Double value
	 */
	double getDouble() const;

	/**
	 * @brief Get Context value
	 * 
	 * @return String value
	 */
	std::string getString() const;

	/**
	 * @brief Get Context value
	 * 
	 * @return Pointer to bytes
	 */
	unsigned char* getDataPtr() const;

	/**
	 * @brief Set Context value
	 * 
	 * @param val Bool value
	 */
	void setBool(bool val);

	/**
	 * @brief Set Context value
	 * 
	 * @param val Long value
	 */
	void setLong(int64_t val);

	/**
	 * @brief Set Context value
	 * 
	 * @param val Double value
	 */
	void setDouble(double val);

	/**
	 * @brief Set Context value
	 * 
	 * @param val String value
	 */
	void setString(const char* str);

	/**
	 * @brief Set Context value
	 * 
	 * @param val String value
	 */
	void setString(const std::string& str);

	/**
	 * @brief Set Context value
	 * 
	 * @param val Array of bytes
	 */
	unsigned char* setDataPtr(void* ptr, int copy_sz = 0);

	/**
	 * @brief Check Context value is bool
	 * 
	 * @return Is Context bool
	 */
	bool isBool() const;

	/**
	 * @brief Check Context value is long
	 * 
	 * @return Is Context long
	 */
	bool isLong() const;

	/**
	 * @brief Check Context value is unsigned long
	 * 
	 * @return Is Context unsigned long
	 */
	bool isUnsignedLong() const;

	/**
	 * @brief Check Context value is double
	 * 
	 * @return Is Context double
	 */
	bool isDouble() const;

	/**
	 * @brief Check Context value is string
	 * 
	 * @return Is Context string
	 */
	bool isString() const;
	
	/**
	 * @brief Check Context value is array of bytes
	 * 
	 * @return Is Context array of bytes
	 */
	bool isDataPtr() const;

	HContext* getHandle();
	const HContext* getHandle() const;

protected:
	Context();
	Context(HContext* handle, bool weak = true);
	HContext* handle_;
	mutable ContextEH* eh_ = nullptr;
	bool weak_;

	void setValue(const char* str);
	void setValue(const std::string& str);

	template<typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
	void setValue(T val);
	void setValue(double val);
	void setValue(float val);
	void setValue(bool val);
	void setValue(void* ptr, int copy_sz = 0);

};


class ContextRef : public Context {
public:
	ContextRef(HContext* handle) : Context(handle, true) {}

	template <typename T, typename = typename std::enable_if<!std::is_base_of<Context, typename std::decay<T>::type>::value>::type>
	void operator=(T&& value) {
		setValue(std::forward<T>(value));
	}
};

template<bool isConst>
Context::ContextArrayIterator<isConst>::ContextArrayIterator(const Context& ctx, int64_t index) :
	base_(ctx), curr_(nullptr), isObj_(ctx.isObject())
{
	length_ = TDVContext_getLength(base_.handle_, &(base_.eh_));
	checkException(base_.eh_);
	index_ = (index > -1) ? (std::min<size_t>)(static_cast<size_t>(index), length_) : length_;
	if(index_ < length_) {
		if(isObj_)
		{
			auto keys = TDVContext_getKeys(base_.handle_, length_, &(base_.eh_));
			checkException(base_.eh_);
			keys_ = std::shared_ptr<charsList>(new charsList(length_, keys), charsList_deleter);
			curr_ = new Context(base_[keys_->operator[](index)]);
		}
		else
			curr_ = new Context(base_[index]);
	}
}

template<bool isConst>
Context::ContextArrayIterator<isConst>::ContextArrayIterator(const Context& ctx, const std::string& key) :
	base_(ctx), curr_(nullptr), isObj_(ctx.isObject())
{
	length_ = TDVContext_getLength(base_.handle_, &(base_.eh_));
	checkException(base_.eh_);
	index_ = length_;
	if(isObj_ && length_) {
		auto keys = TDVContext_getKeys(base_.handle_, length_, &(base_.eh_));
		checkException(base_.eh_);
		keys_ = std::shared_ptr<charsList>(new charsList(length_, keys), charsList_deleter);
		size_t i=0;
		do {
			if(!key.compare(keys_->operator[](i))) {
				index_ = i;
				curr_ = new Context(base_[key]);
				break;
			}
		} while((++i)<length_);
	}
}

template<bool isConst>
Context::ContextArrayIterator<isConst>::ContextArrayIterator(const ContextArrayIterator& iter) :
	base_(iter.base_), length_(iter.length_), index_(iter.index_), curr_(iter.curr_ ? new Context(*(iter.curr_)) : nullptr), isObj_(iter.isObj_), keys_(iter.keys_) {}

template<bool isConst>
Context::ContextArrayIterator<isConst>::ContextArrayIterator(ContextArrayIterator&& iter) :
	base_(iter.base_), length_(iter.length_), index_(iter.index_), curr_(iter.curr_), isObj_(iter.isObj_), keys_(std::move(iter.keys_)) {
	iter.curr_ = nullptr;
}


template<bool isConst>
Context::ContextArrayIterator<isConst>& Context::ContextArrayIterator<isConst>::operator++() {
	if(curr_)
		delete curr_;
	index_ = std::min(index_+1, length_);
	if(isObj_)
		curr_ = (index_ < length_) ? new Context(base_[keys_->operator[](index_)]) : nullptr;
	else
		curr_ = (index_ < length_) ? new Context(base_[index_]) : nullptr;
	return *this;
}

template<bool isConst>
Context::ContextArrayIterator<isConst> Context::ContextArrayIterator<isConst>::operator++(int) {
	ContextArrayIterator tmp = *this;
	index_ = std::min(index_+1, length_);
	if(isObj_)
		curr_ = (index_ < length_) ? new Context(base_[keys_->operator[](index_)]) : nullptr;
	else
		curr_ = (index_ < length_) ? new Context(base_[index_]) : nullptr;
	return tmp;
}

inline Context::Context() : weak_(false) {
	handle_ = TDVContext_create(&eh_);
	checkException(eh_);
}

inline Context::~Context()
{
	if(!weak_) {
		TDVContext_destroy(handle_, &eh_);
		// N.B. deprecated in c++17 - move to std::uncaught_exceptions()
		if (eh_ && std::uncaught_exception())
			std::cerr << Error(TDVException_getErrorCode(eh_), TDVException_getMessage(eh_)).what();
		else
			checkException(eh_);
	}
}

inline Context::Context(const Context& other) : weak_(false) {
	handle_ = TDVContext_clone(other.handle_, &eh_);
	checkException(eh_);
}

inline Context& Context::operator=(const Context& other) {
	if (&other != this)
	{
		if(weak_) {
			TDVContext_copy(other.handle_, handle_, &eh_);
		}
		else {
			HContext* copy = TDVContext_clone(other.handle_, &eh_); // copy-swap
			checkException(eh_);
			std::swap(handle_, copy);
			TDVContext_destroy(copy, &eh_);
		}
		checkException(eh_);
	}
	return *this;
}

inline Context::operator ContextRef()
{
	return ContextRef(handle_);
}

inline Context::Context(Context&& other) : weak_(false) {
	if(other.weak_)
	{
		handle_ = TDVContext_clone(other.handle_, &eh_);
		checkException(eh_);
	}
	else
	{
		handle_ = other.handle_;
		other.handle_ = nullptr;
		other.weak_ = true;
	}
}

inline Context& Context::operator=(Context&& other){
	if (&other != this)
	{
		if(weak_) {
			TDVContext_copy(other.handle_, handle_, &eh_);
			checkException(eh_);
		}
		else
		{
			if(other.weak_)
			{
				HContext* copy = TDVContext_clone(other.handle_, &eh_);
				checkException(eh_);
				std::swap(handle_, copy);
				TDVContext_destroy(copy, &eh_);
				checkException(eh_);
			}
			else
			{
				handle_ = other.handle_;
				other.handle_ = nullptr;
				other.weak_ = true;
			}
		}
	}
	return *this;
}

template <typename T, typename >
Context::Context(T&& value) : weak_(false) {
	handle_ = TDVContext_create(&eh_);
	checkException(eh_);
	setValue(std::forward<T>(value));
}

template <typename T, typename >
Context& Context::operator=(T&& value) {
	setValue(std::forward<T>(value));
	return *this;
}

template<typename T>
typename std::enable_if<!std::is_base_of<Context, typename std::decay<T>::type>::value>::type Context::push_back(T&& data) {
	Context elem;
	elem.setValue(std::forward<T>(data));
	// use r-value version of push_back without a copy
	push_back(std::move(elem));
}

template<typename T, typename >
void Context::setValue(T val) {
	TDVContext_putLong(handle_, val, &eh_);
	checkException(eh_);
}


inline bool Context::operator==(const Context& other) const {
	return this->handle_ == other.handle_;
}

inline size_t Context::size() const	{
	size_t lenght = TDVContext_getLength(handle_, &eh_);
	checkException(eh_);
	return lenght;
}

inline bool Context::isNone() const {
	bool value = TDVContext_isNone(handle_, &eh_);
	checkException(eh_);
	return value;
}

inline bool Context::isArray() const {
	bool value = TDVContext_isArray(handle_, &eh_);
	checkException(eh_);
	return value;
}

inline bool Context::isObject() const {
	bool value = TDVContext_isObject(handle_, &eh_);
	checkException(eh_);
	return value;
}

inline Context Context::operator[](const std::string& key) {
	HContext* handle = TDVContext_getOrInsertByKey(handle_, key.c_str(), &eh_);
	checkException(eh_);
	return Context(handle, true);
}

inline Context Context::operator[](const char* key) {
	HContext* handle = TDVContext_getOrInsertByKey(handle_, key, &eh_);
	checkException(eh_);
	return Context(handle, true);
}

inline Context Context::operator[](const int index) {
	HContext* handle = TDVContext_getByIndex(handle_, index, &eh_);
	checkException(eh_);
	return Context(handle, true);
}

inline const Context Context::operator[](const std::string& key) const {
	HContext* handle = TDVContext_getByKey(handle_, key.c_str(), &eh_);
	checkException(eh_);
	return Context(handle, true);
}

inline const Context Context::operator[](const char* key) const {
	HContext* handle = TDVContext_getByKey(handle_, key, &eh_);
	checkException(eh_);
	return Context(handle, true);
}

inline const Context Context::operator[](const int index) const {
	HContext* handle = TDVContext_getByIndex(handle_, index, &eh_);
	checkException(eh_);
	return Context(handle, true);
}

inline Context Context::at(const char* key) {
	HContext* handle = TDVContext_getByKey(handle_, key, &eh_);
	checkException(eh_);
	return Context(handle, true);
}

inline Context Context::at(const std::string& key) {
	HContext* handle = TDVContext_getByKey(handle_, key.c_str(), &eh_);
	checkException(eh_);
	return Context(handle, true);
}

inline Context Context::at(const int index) {
	HContext* handle = TDVContext_getByIndex(handle_, index, &eh_);
	checkException(eh_);
	return Context(handle, true);
}

inline const Context Context::at(const char* key) const {
	HContext* handle = TDVContext_getByKey(handle_, key, &eh_);
	checkException(eh_);
	return Context(handle, true);
}

inline const Context Context::at(const std::string& key) const {
	HContext* handle = TDVContext_getByKey(handle_, key.c_str(), &eh_);
	checkException(eh_);
	return Context(handle, true);
}

inline const Context Context::at(const int index) const {
	HContext* handle = TDVContext_getByIndex(handle_, index, &eh_);
	checkException(eh_);
	return Context(handle, true);
}

inline bool Context::contains(const std::string& key) const {
	if (isObject())
	{
		HContext* handle = TDVContext_getByKey(handle_, key.c_str(), &eh_);
		if(eh_)
		{
			TDVException_deleteException(eh_);
			eh_ = nullptr;
		}
		else
			return true;
	}
	return false;
}

inline Context::ContextArrayIterator<> Context::find(const std::string& key) {
	return ContextArrayIterator<>(*this, key);
}

inline Context::ContextArrayIterator<true> Context::find(const std::string& key) const {
	return ContextArrayIterator<true>(*this, key);
}

inline void Context::clear() {
	TDVContext_clear(handle_, &eh_);
	checkException(eh_);
}

inline Context::ContextArrayIterator<> Context::begin(){
	return ContextArrayIterator<>(*this, 0);
}

inline Context::ContextArrayIterator<> Context::end(){
	return ContextArrayIterator<>(*this, -1);
}

inline Context::ContextArrayIterator<true> Context::begin() const {
	return ContextArrayIterator<true>(*this, 0);
}

inline Context::ContextArrayIterator<true> Context::end() const {
	return ContextArrayIterator<true>(*this, -1);
}

inline Context::ContextArrayIterator<true> Context::cbegin() const {
	return ContextArrayIterator<true>(*this, 0);
}

inline Context::ContextArrayIterator<true> Context::cend() const{
	return ContextArrayIterator<true>(*this, -1);
}


inline void Context::push_back(const Context& data) {
	TDVContext_pushBack(handle_, data.handle_, true, &eh_);
	checkException(eh_);
}

inline void Context::push_back(Context&& data) {
	TDVContext_pushBack(handle_, data.handle_, weak_, &eh_);  // cannot move weak objects
	checkException(eh_);
}

inline double Context::getDouble() const {
	double ret = TDVContext_getDouble(handle_, &eh_);
	checkException(eh_);
	return ret;
}

inline int64_t Context::getLong() const {
	int64_t ret = TDVContext_getLong(handle_, &eh_);
	checkException(eh_);
	return ret;
}

inline uint64_t Context::getUnsignedLong() const {
	uint64_t ret = TDVContext_getUnsignedLong(handle_, &eh_);
	checkException(eh_);
	return ret;
}

inline bool Context::getBool() const {
	bool ret = TDVContext_getBool(handle_, &eh_);
	checkException(eh_);
	return ret;
}

inline std::string Context::getString() const {
	unsigned long str_size = TDVContext_getStrSize(handle_, &eh_);
	checkException(eh_);
	std::string ret;
	ret.resize(str_size);
	TDVContext_getStr(handle_, &ret[0], &eh_);
	checkException(eh_);
	return ret; // copy elision (NRVO)
}

inline unsigned char* Context::getDataPtr() const {
	unsigned char* ret = TDVContext_getDataPtr(handle_, &eh_);
	checkException(eh_);
	return ret;
}

inline void Context::setString(const char* str) {
	TDVContext_putStr(handle_, str, &eh_);
	checkException(eh_);
}

inline void Context::setString(const std::string& str) {
	TDVContext_putStr(handle_, str.c_str(), &eh_);
	checkException(eh_);
}

inline void Context::setLong(int64_t val) {
	TDVContext_putLong(handle_, val, &eh_);
	checkException(eh_);
}

inline void Context::setDouble(double val) {
	TDVContext_putDouble(handle_, val, &eh_);
	checkException(eh_);
}

inline void Context::setBool(bool val) {
	TDVContext_putBool(handle_, val, &eh_);
	checkException(eh_);
}

inline unsigned char* Context::setDataPtr(void* ptr, int copy_sz) {
	unsigned char* ret{nullptr};
	if(copy_sz && !ptr)
		ret = TDVContext_allocDataPtr(handle_, copy_sz, &eh_);
	else
		ret = TDVContext_putDataPtr(handle_, static_cast<unsigned char*>(ptr), copy_sz, &eh_);
	checkException(eh_);
	return ret;
}

inline bool Context::isBool() const {
	bool val = TDVContext_isBool(handle_, &eh_);
	checkException(eh_);
	return val;
}

inline bool Context::isLong() const {
	bool val = TDVContext_isLong(handle_, &eh_);
	checkException(eh_);
	return val;
}

inline bool Context::isUnsignedLong() const {
	bool val = TDVContext_isUnsignedLong(handle_, &eh_);
	checkException(eh_);
	return val;
}

inline bool Context::isDouble() const {
	bool val = TDVContext_isDouble(handle_, &eh_);
	checkException(eh_);
	return val;
}

inline bool Context::isString() const {
	bool val = TDVContext_isString(handle_, &eh_);
	checkException(eh_);
	return val;
}

inline bool Context::isDataPtr() const {
	bool val = TDVContext_isDataPtr(handle_, &eh_);
	checkException(eh_);
	return val;
}

inline HContext* Context::getHandle() {return handle_;}
inline const HContext* Context::getHandle() const {return handle_;}

inline Context::Context(HContext* handle, bool weak) : weak_(weak) {
	if(weak_)
		handle_ = handle;
	else
	{
		handle_ = TDVContext_clone(handle, &eh_);
		checkException(eh_);
	}
}

inline void Context::setValue(const char* str) {
	TDVContext_putStr(handle_, str, &eh_);
	checkException(eh_);
}

inline void Context::setValue(const std::string& str) {
	TDVContext_putStr(handle_, str.c_str(), &eh_);
	checkException(eh_);
}

inline void Context::setValue(double val) {
	TDVContext_putDouble(handle_, val, &eh_);
	checkException(eh_);
}

inline void Context::setValue(float val) {
	TDVContext_putDouble(handle_, val, &eh_);
	checkException(eh_);
}

inline void Context::setValue(bool val) {
	TDVContext_putBool(handle_, val, &eh_);
	checkException(eh_);
}

inline void Context::setValue(void* ptr, int copy_sz) {
	TDVContext_putDataPtr(handle_, static_cast<unsigned char*>(ptr), copy_sz, &eh_);
	checkException(eh_);
}


}
#endif
