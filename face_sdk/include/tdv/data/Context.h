#ifndef TDV_DATA_CONTEXT_V2_CONTEXT_H_
#define TDV_DATA_CONTEXT_V2_CONTEXT_H_

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <type_traits>

namespace {

// because of https://wg21.cmeerw.net/cwg/issue1558 we ought to use more comlex void_t then usual
// template<class ...>
// using void_t = void;
template<typename... Ts> struct make_void { typedef void type;};
template<typename... Ts> using void_t = typename make_void<Ts...>::type;

template <typename C, typename _ =  void_t<>>
struct is_associative_container : std::false_type{};

template<typename T>
struct is_associative_container<T, void_t<typename T::key_type>> : std::true_type {};

template <typename C, typename = void_t<>>
struct is_pair : std::false_type{};

template<typename T>
struct is_pair<T, void_t<typename T::first_type, typename T::second_type>> : std::true_type {};

template<typename T> class Type2Type {};

template <template <typename...> class, template<typename...> class>
struct is_same_template : std::false_type{};

template <template <typename...> class T>
struct is_same_template<T, T> : std::true_type{};

template<typename T, typename U, typename = void_t<>>
struct is_comparable : std::false_type {};

template<typename T, typename U>
struct is_comparable<T, U, void_t<decltype(std::declval<T>() == std::declval<U>())>> : std::true_type {};

template<typename T, typename U, typename = void>
struct is_bool_comparable : std::false_type {};

template<typename T, typename U>
struct is_bool_comparable<T, U, typename std::enable_if<std::is_convertible<decltype(std::declval<T>() == std::declval<U>()), bool>::value>::type> : std::true_type {};

template <class... T>
struct enumeration_types
{
	constexpr static size_t max_size = 0;
	constexpr static size_t min_size = ~((size_t)0);
	using max_type = void;
	using min_type = void;
};

template <class Target_type, class... Types>
struct enumeration_types<Target_type, Types...>
{
	constexpr static size_t max_size = (sizeof(Target_type) > enumeration_types<Types...>::max_size) ? sizeof(Target_type) : enumeration_types<Types...>::max_size;
	constexpr static size_t min_size = (sizeof(Target_type) < enumeration_types<Types...>::min_size) ? sizeof(Target_type) : enumeration_types<Types...>::min_size;

	using max_type = typename std::conditional<(max_size < enumeration_types<Types...>::max_size), typename enumeration_types<Types...>::max_type, Target_type>::type;
	using min_type = typename std::conditional<(min_size > enumeration_types<Types...>::min_size), typename enumeration_types<Types...>::min_type, Target_type>::type;
};
}

namespace tdv
{
namespace data
{
class Context
{
	using key_type = std::string;
	using associative_container_type = std::map<key_type, Context>;
	using sequence_container_type = std::vector<Context>;

	class IteratorBase
	{
	public:
		virtual void operator++() = 0;
		virtual void operator--() = 0;
	};

	template <bool isConst>
	class ValueIteratorBase : public IteratorBase
	{
	public:
		using value_type = typename std::conditional<isConst, const typename sequence_container_type::value_type, typename sequence_container_type::value_type>::type;
		using difference_type = typename sequence_container_type::difference_type;

		virtual ~ValueIteratorBase(){}
		virtual value_type& operator*() const = 0;
		virtual ValueIteratorBase* clone() const = 0;
		virtual bool equalTo(const ValueIteratorBase& ) const = 0;
		virtual const key_type& key() const = 0;
		virtual value_type& value() const { return operator*();}

		virtual difference_type operator-(const ValueIteratorBase& ) const = 0;
		virtual bool operator<(const ValueIteratorBase& ) const = 0;
		virtual bool operator>(const ValueIteratorBase& ) const = 0;
		virtual void advance(const difference_type i) = 0;
	};

	template<bool isConst>
	class KeyValueIterator : public IteratorBase
	{
		using IterType = typename std::conditional<isConst, typename associative_container_type::const_iterator, typename associative_container_type::iterator>::type;

	public:
		using value_type = typename std::conditional<isConst, const typename associative_container_type::value_type, typename associative_container_type::value_type>::type;
		using difference_type = typename associative_container_type::difference_type;

		KeyValueIterator(IterType wrapped) : wrapped_(wrapped) {}

		KeyValueIterator(const KeyValueIterator& other) {
			wrapped_ = other.wrapped_;
		}

		KeyValueIterator& operator=(const KeyValueIterator& other) {
			if (this != &other)
				wrapped_ = other.wrapped_;
			return *this;
		}

		KeyValueIterator* clone() const {
			return new KeyValueIterator(*this);
		}

		virtual value_type& operator*() const {
			return *wrapped_;
		}

		virtual void operator++() override {
			++wrapped_;
		}

		virtual void operator--() override {
			--wrapped_;
		}

		IterType getIterator() { return wrapped_; }

		bool equalTo(const KeyValueIterator& other) const {
			return wrapped_ == other.wrapped_;
		}

		const key_type& key() const {return wrapped_->first;}

		typename std::conditional<isConst,
			const associative_container_type::mapped_type&,
			associative_container_type::mapped_type&>::type
		value() const { return wrapped_->second;}

	private:
		IterType wrapped_;
	};

public:
	template<bool isConst>
	class ValueContextIterator;

	template<bool isConst>
	class KeyValueContextIterator;

private:
	template<typename Impl, typename Base, bool isConst>
	class ContextIterator
	{
	public:
		typedef std::bidirectional_iterator_tag												iterator_category;
		typedef typename Base::value_type													value_type;
		typedef typename std::conditional<isConst, const value_type*, value_type*>::type	pointer;
		typedef typename std::conditional<isConst, const value_type&, value_type&>::type	reference;
		typedef const value_type&															const_reference;

		ContextIterator() = default;

		ContextIterator(const ContextIterator& other)
		{
			iteratorImpl.reset(other.iteratorImpl ?  other.iteratorImpl->clone() : nullptr);
		}

		ContextIterator& operator=(const ContextIterator& other)
		{
			if (this != &other)
				iteratorImpl.reset(other.iteratorImpl ? other.iteratorImpl->clone() : nullptr);
			return *this;
		}

		bool operator==(const Impl& other) const {
			if(iteratorImpl) {
				if(other.iteratorImpl)
					return iteratorImpl->equalTo(*other.iteratorImpl);
			}
			return (other.iteratorImpl == nullptr);
		}

		bool operator!=(const Impl& other) const {
			return !(this->operator==(other));
		}

		Impl& operator++() {
			if(iteratorImpl)
				iteratorImpl->operator++();
			return *static_cast<Impl*>(this);
		}

		Impl operator++(int) {
			Impl tmp = *static_cast<Impl*>(this);
			if(iteratorImpl)
				iteratorImpl->operator++();
			return tmp;
		}

		Impl& operator--() {
			if(iteratorImpl)
				iteratorImpl->operator--();
			return *static_cast<Impl*>(this);
		}

		Impl operator--(int) {
			Impl tmp = *static_cast<Impl*>(this);
			if(iteratorImpl)
				iteratorImpl->operator--();
			return tmp;
		}

		reference operator*() const {
			return *(*iteratorImpl);
		}

		pointer operator->() const {
			return &*(*iteratorImpl);
		}

		template<typename T>
		typename std::enable_if<std::is_same<Impl, ValueContextIterator<isConst>>::value, typename T::iterator>::type getIterator();
		template<typename T>
		typename std::enable_if<std::is_same<Impl, KeyValueContextIterator<isConst>>::value, typename T::iterator>::type getIterator();

		virtual const key_type& key() const {return iteratorImpl->key();}

		typename std::conditional<isConst, const typename sequence_container_type::value_type&, typename sequence_container_type::value_type&>::type value() {return iteratorImpl->value();}

	protected:
		void initIterator(Base* iter) {
			iteratorImpl.reset(iter);
		}

		std::unique_ptr<Base> iteratorImpl;
	};

public:
	template<bool isConst=false>
	class ValueContextIterator : public ContextIterator<ValueContextIterator<isConst>, ValueIteratorBase<isConst>, isConst>
	{
	public:

		using difference_type = typename ValueIteratorBase<isConst>::difference_type;

		ValueContextIterator() = delete;

		ValueContextIterator(const ValueContextIterator& other) : ContextIterator<ValueContextIterator<isConst>, ValueIteratorBase<isConst>, isConst>(other)
		{}

		ValueContextIterator& operator=(const ValueContextIterator& other) {
			ContextIterator<ValueContextIterator<isConst>, ValueIteratorBase<isConst>, isConst>::operator=(other);
			return *this;
		}

		ValueContextIterator(std::nullptr_t) {}

		ValueContextIterator(sequence_container_type::iterator iter) {
			this->initIterator(new Context::ValueIterator<sequence_container_type, false>(iter));
		}
		ValueContextIterator(associative_container_type::iterator iter) {
			this->initIterator(new Context::ValueIterator<associative_container_type, false>(iter));
		}
		ValueContextIterator(sequence_container_type::const_iterator iter) {
			this->initIterator(new Context::ValueIterator<sequence_container_type, true>(iter));
		}
		ValueContextIterator(associative_container_type::const_iterator iter) {
			this->initIterator(new Context::ValueIterator<associative_container_type, true>(iter));
		}

		bool operator<(const ValueContextIterator& other) const {
			if(this->iteratorImpl && other.iteratorImpl)
				return *(this->iteratorImpl) < (*other.iteratorImpl);
			return false;
		}

		bool operator>(const ValueContextIterator& other) const {
			if(this->iteratorImpl && other.iteratorImpl)
				return *(this->iteratorImpl) > (*other.iteratorImpl);
			return false;
		}

		bool operator<=(const ValueContextIterator& other) const {
			return this->operator<(other) || this->operator==(other);
		}

		bool operator>=(const ValueContextIterator& other) const {
			return this->operator>(other) || this->operator==(other);
		}

		ValueContextIterator operator+(const difference_type i) const {
			auto tmp = *this;
			if(tmp.iteratorImpl)
				tmp.iteratorImpl->advance(i);
			return tmp;
		}

		ValueContextIterator operator-(const difference_type i) const {
			auto tmp = *this;
			if(tmp.iteratorImpl)
				tmp.iteratorImpl->advance(-i);
			return tmp;
		}

		difference_type operator-(const ValueContextIterator& other) const {
			if(this->iteratorImpl && other.iteratorImpl)
				return *(this->iteratorImpl) - *other.iteratorImpl;
			return 0;
		}
	};

	template<bool isConst=false>
	class KeyValueContextIterator : public ContextIterator<KeyValueContextIterator<isConst>, KeyValueIterator<isConst>, isConst>
	{
	public:
		using difference_type = typename KeyValueIterator<isConst>::difference_type;

		KeyValueContextIterator() = delete;
		KeyValueContextIterator(associative_container_type::iterator iter) {
			this->initIterator(new Context::KeyValueIterator<false>(iter));
		}
		KeyValueContextIterator(associative_container_type::const_iterator iter) {
			this->initIterator(new Context::KeyValueIterator<true>(iter));
		}
	};

private:
	class ContextBase
	{
	public:
		virtual ~ContextBase() = default;

		virtual std::unique_ptr<ContextBase> deep_copy_ptr() = 0;
		virtual int64_t type() const { return -1; }
		virtual void* data() { return nullptr; }
		virtual size_t size() const = 0;
		virtual Context& operator [](const std::string& key) { return at(key); }
		virtual Context& operator [](const int index) { return at(index); }
		virtual const Context& operator [](const std::string& key) const { return at(key); }
		virtual const Context& operator [](const int index) const { return at(index); }
		virtual Context& at(const std::string& key) { throw std::runtime_error("no item: " + key); }
		virtual Context& at(const int index) { throw std::runtime_error("no index: " + std::to_string(index)); }
		virtual const Context& at(const std::string& key) const { throw std::runtime_error("Context has no item " + key); }
		virtual const Context& at(const int index) const { throw std::runtime_error("Context has no index " + std::to_string(index)); }
		virtual bool compare(const Context& /*data*/) const { throw std::runtime_error("compare(const Context& data): error. is not applicable for the called Context."); }

		virtual void push_back(const Context& ) { throw std::runtime_error("push_back() is not applicable for the called Context"); }
		virtual void push_back(Context&& ) { throw std::runtime_error("push_back() is not applicable for the called Context"); }
		virtual bool contains(const key_type&) const { throw std::runtime_error("contains() is not applicable for the called Context"); }
		virtual size_t count(const key_type&) const { throw std::runtime_error("count() is not applicable for the called Context"); }

		virtual ValueContextIterator<> begin() { return ValueContextIterator<>(nullptr); } // { throw std::runtime_error("begin() is not applicable for the called Context"); }
		virtual ValueContextIterator<> end() { return ValueContextIterator<>(nullptr); } // { throw std::runtime_error("end() is not applicable for the called Context"); }
		virtual KeyValueContextIterator<> kvbegin() { throw std::runtime_error("kvbegin() is not applicable for the called Context"); }
		virtual KeyValueContextIterator<> kvend() { throw std::runtime_error("kvend() is not applicable for the called Context"); }
		virtual ValueContextIterator<true> cbegin() const { return ValueContextIterator<true>(nullptr); } // { throw std::runtime_error("cbegin() is not applicable for the called Context"); }
		virtual ValueContextIterator<true> cend() const { return ValueContextIterator<true>(nullptr); } // { throw std::runtime_error("cend() is not applicable for the called Context"); }
		virtual KeyValueContextIterator<true> kvcbegin() const { throw std::runtime_error("kvcbegin() is not applicable for the called Context"); }
		virtual KeyValueContextIterator<true> kvcend() const { throw std::runtime_error("kvcend() is not applicable for the called Context"); }
		virtual ValueContextIterator<> find(const key_type& /*key*/) { return ValueContextIterator<>(nullptr); } // { throw std::runtime_error("find() is not applicable for the called Context"); }
		virtual ValueContextIterator<true> cfind(const key_type& /*key*/) const { return ValueContextIterator<true>(nullptr); } // { throw std::runtime_error("cfind() is not applicable for the called Context"); }
		virtual KeyValueContextIterator<> kvfind(const key_type& /*key*/) { throw std::runtime_error("kvfind() is not applicable for the called Context"); }
		virtual KeyValueContextIterator<true> kvcfind(const key_type& /*key*/) const { throw std::runtime_error("kvcfind() is not applicable for the called Context"); }
		virtual ValueContextIterator<> erase(ValueContextIterator<> /*pos*/) { throw std::runtime_error("erase() is not valid for the called Context"); }
		virtual ValueContextIterator<> erase(ValueContextIterator<> /*first*/, ValueContextIterator<> /*last*/) { throw std::runtime_error("erase() is not valid for for the called Context"); }
		virtual KeyValueContextIterator<> erase(KeyValueContextIterator<> /*pos*/) { throw std::runtime_error("erase() is not valid for the called Context"); }
		virtual KeyValueContextIterator<> erase(KeyValueContextIterator<> /*first*/, KeyValueContextIterator<> /*last*/) { throw std::runtime_error("erase() is not valid for for the called Context"); }
		virtual size_t erase(const key_type& /*key*/ ) { throw std::runtime_error("erase() is not valid for the called Context"); }
	};

	class NoneContextBase : public ContextBase
	{
	public:
		virtual std::unique_ptr<ContextBase> deep_copy_ptr() override {
			return std::unique_ptr<NoneContextBase>(new NoneContextBase());
		}
		bool contains(const key_type&) const override { return false; }

		virtual bool compare(const Context& data) const override
		{
			return data.isNone();
		}

		size_t size() const override { return 0;}
	};

	template <typename T>
	class AnyContextBase : public ContextBase
	{
	public:
		AnyContextBase(const T& data) : _data(data) {}
		AnyContextBase(T&& data) : _data(std::move(data)) {}

		virtual std::unique_ptr<ContextBase> deep_copy_ptr() override {
			return std::unique_ptr<AnyContextBase<T>>(new AnyContextBase<T>(*this));
		}

		virtual size_t size() const override { throw std::runtime_error("size() is not applicable for the scalar Context"); }

		int64_t type() const override { return static_cast<int64_t>(typeid(T).hash_code()); }
		void* data() override { return &_data; }

		bool compare(const Context& data) const override
		{
			if(is_bool_comparable<T, T>::value && ContextAdapter<T>::isTypeMatch(*data._base))
			{
				return operator_eq_impl(data.as<T>());
			}
			return false;
		}

		template <typename U = T>
		bool operator_eq_impl(const U& val, typename std::enable_if<is_bool_comparable<T,U>::value, void>::type* = nullptr) const
		{
			return _data == val;
		}

		template <typename U = T>
		bool operator_eq_impl(const U& val, typename std::enable_if<!is_bool_comparable<T,U>::value, void>::type* = nullptr) const
		{
			return false;
		}

	private:

		T _data;
	};

	template <typename T, typename = void>
	struct ContextAdapter
	{
		using Base = AnyContextBase<typename std::remove_const<typename std::remove_reference<T>::type>::type>;

		static T value(ContextBase& base) {
			return ContextAdapter<T>::repr(base);
		}
		static T& repr(ContextBase& base) {
			if (!ContextAdapter<T>::isTypeMatch(base))
				throw std::runtime_error("Context type mismatch: " + std::string(typeid(T).name()) + " was requested");
			return *reinterpret_cast<T*>(base.data());
		}

		static std::unique_ptr<ContextBase> base(T&& data) {
			return std::unique_ptr<ContextBase>(new Base(std::forward<T>(data)));
		}

		static bool isTypeMatch(ContextBase& base)	{
			if (!base.data())
				return false;
			return static_cast<int64_t>(typeid(T).hash_code()) == base.type();
		}
	};

	class VectorContextBase;
	class MapContextBase;

public:
	template <class T>
	static Context create(T&& data) { return Context(std::forward<T>(data)); }

	Context() : _base(std::unique_ptr<ContextBase>(new NoneContextBase())) {}

	template <typename T, typename = typename std::enable_if<!std::is_same<typename std::decay<T>::type,Context>::value>::type>
	Context(T&& data) : _base(ContextAdapter<T>::base(std::forward<T>(data))) {
//		std::cout << " Context constructor " <<  typeid(Type2Type<T>).name() << " - " << typeid(Type2Type<decltype(data)>).name() << std::endl;
	}

	template <class T, typename = typename std::enable_if<!std::is_same<typename std::decay<T>::type,Context>::value>::type>
	Context& operator =(T&& value) {
//		std::cout << " Context assignment " <<  typeid(Type2Type<T>).name() << " - " << typeid(Type2Type<decltype(value)>).name() << std::endl;
		_base = ContextAdapter<T>::base(std::forward<T>(value));
		return *this;
	}

	template <typename T>
	Context(std::initializer_list<T>&& data) : _base(ContextAdapter<std::initializer_list<T>>::base(std::move(data))) {
//		std::cout << "Context constructor from initializer_list " <<  typeid(Type2Type<T>).name() << " - " << typeid(Type2Type<decltype(data)>).name() << std::endl;
	}

	template <class T>
	Context& operator =(std::initializer_list<T>&& value) {
		_base = ContextAdapter<std::initializer_list<T>>::base(std::move(value));
		return *this;
	}

	// for "literal" case
	Context(const char* str) : _base(ContextAdapter<std::string>::base(std::string(str))) { }

	Context& operator =(const char* str) {
		_base = ContextAdapter<std::string>::base(std::string(str));
		return *this;
	}

	Context(const Context& other) {
		_base = other._base->deep_copy_ptr();
	}

	Context& operator=(const Context& other) {
		if (&other != this)
			_base = other._base->deep_copy_ptr();
		return *this;
	}

	Context(Context&& other) noexcept {
		_base = std::move(other._base);
	}

	Context& operator=(Context&& other) noexcept {
		if (&other != this)
			_base = std::move(other._base);
		return *this;
	}

	bool operator==(const Context& other) const {
		return &other == this;
	}

	bool isNone() const;
	bool isScalar() const {return (_base->type() != -1);}
	bool isArray() const;
	bool isObject() const;
	bool empty() const {return !isScalar() && (isNone() || _base->size() == 0); }

	size_t size() const { return _base->size(); }

	Context& operator [](const std::string& key);
	Context& operator [](const int index);

	const Context& operator [](const std::string& key) const;
	const Context& operator [](const int index) const;

	Context& at(const std::string& key) { return _base->at(key); }
	Context& at(const int index) { return _base->at(index); }

	const Context& at(const std::string& key) const { return _base->at(key); }
	const Context& at(const int index) const { return _base->at(index); }

	template<typename T>
	void push_back(T&& data);

	inline static Context make_array();
	inline static Context make_object();

	ValueContextIterator<> begin() { return _base->begin(); }
	ValueContextIterator<> end() { return _base->end(); }

	ValueContextIterator<true> begin() const { return _base->cbegin(); }
	ValueContextIterator<true> end() const { return _base->cend(); }

	ValueContextIterator<true> cbegin() const { return _base->cbegin(); }
	ValueContextIterator<true> cend() const { return _base->cend(); }

	KeyValueContextIterator<> kvbegin() { return _base->kvbegin(); }
	KeyValueContextIterator<> kvend() { return _base->kvend(); }

	KeyValueContextIterator<true> kvbegin() const { return _base->kvcbegin(); }
	KeyValueContextIterator<true> kvend() const { return _base->kvcend(); }

	KeyValueContextIterator<true> kvcbegin() const { return _base->kvcbegin(); }
	KeyValueContextIterator<true> kvcend() const { return _base->kvcend(); }

	ValueContextIterator<> erase(ValueContextIterator<> pos) { return _base->erase(pos); }
	ValueContextIterator<> erase(ValueContextIterator<> first, ValueContextIterator<> last) { return _base->erase(first, last); }

	KeyValueContextIterator<> erase(KeyValueContextIterator<> pos) { return _base->erase(pos); }
	KeyValueContextIterator<> erase(KeyValueContextIterator<> first, KeyValueContextIterator<> last) { return _base->erase(first, last); }

	ValueContextIterator<> find(const key_type& key) { return _base->find(key); }
	ValueContextIterator<true> find(const key_type& key) const { return _base->cfind(key); }

	KeyValueContextIterator<> kvfind(const key_type& key) { return _base->kvfind(key); }
	KeyValueContextIterator<true> kvfind(const key_type& key) const { return _base->kvcfind(key); }

	size_t erase(const key_type& key) { return _base->erase(key); }
	bool contains(const key_type& key) const { return _base->contains(key); }
	size_t count( const key_type& key ) const { return _base->count(key); }

	void clear() noexcept { _base = std::unique_ptr<NoneContextBase>(new NoneContextBase()); }

	bool compare(const Context& data) const { return _base->compare(data); }
//	KeyIterator kbegin();
//	KeyIterator kend();
//	void pop_back();
//	void merge();

	template <class T>
	bool is() const { return ContextAdapter<T>::isTypeMatch(*_base); }

	template <class T>
	T get() const { return ContextAdapter<T>::value(*_base); }

	template <class T>
	T get(const T& defaultValue) const {
		return is<T>() ? get<T>() : defaultValue;
	}

	template<class T>
	inline typename std::enable_if<std::is_unsigned<T>::value && !std::is_same<T, bool>::value, T>::type
	get_as() const // unsigned integer conversion
	{
		return static_cast<T>(retype_as<
							uint64_t, 	uint32_t, 	uint16_t, 	uint8_t,
							int64_t, 	int32_t, 	int16_t, 	int8_t>());
	}

	template<class T>
	inline typename std::enable_if<std::is_signed<T>::value && std::is_integral<T>::value, T>::type
	get_as() const // signed integer conversion
	{
		return static_cast<T>(retype_as<
							int64_t, 	int32_t, 	int16_t, 	int8_t,
							uint64_t, 	uint32_t, 	uint16_t, 	uint8_t>());
	}

	template<class T>
	inline typename std::enable_if<std::is_signed<T>::value && !std::is_integral<T>::value, T>::type
	get_as() const
	{
		return static_cast<T>(retype_as<
						   	float, 		double,
							uint64_t, 	int64_t,
							uint32_t, 	int32_t,
							uint16_t, 	int16_t,
							uint8_t, 	int8_t>());
	}

	template<class T>
	inline typename std::enable_if<std::is_same<T, bool>::value, T>::type
	get_as() const // convert to boolean
	{
		return static_cast<T>(retype_as<
							bool,
							uint64_t, 	int64_t,
							uint32_t, 	int32_t,
							uint16_t, 	int16_t,
							uint8_t, 	int8_t,
							float, 		double>());
	}

	template<class T>
	inline typename std::enable_if<std::is_same<T, std::string>::value, T>::type
	get_as() const
	{
		if(is<const char*>())
			return static_cast<std::string>(as<const char*>());
		if(is<std::string>())
			return as<std::string>();
		if(is<bool>())
		{
			return get<bool>() ? std::string("true") : std::string("false");
		}

		// should we try to serialise the object from here?
		throw std::runtime_error("unknown context content type");
	}

	template<class T>
	inline typename std::enable_if<!std::is_arithmetic<T>::value && !std::is_same<T, std::string>::value, T>::type
	get_as() const
	{
		return get<T>();
	}

// v1::Context legacy
// Following methods are mainly for coherence with v1::Context
// Do not use them, as they can be replaced with native ones

// P.S. ^ this is highly doubtful, because you wouldn't want to write this v
// every time you're not sure if value is present in the context.
	template <class T>
	T get(const key_type& key, const T& defaultValue) const {
		auto iter = find(key);
		return iter != end() ? iter->get<T>() : defaultValue;
	}

	template <class T>
	inline T get_as(const key_type& key, const T& defaultValue) const {
		auto iter = find(key);
		return iter != end() ? iter->get_as<T>() : defaultValue;
	}

	template <class T>
	inline T get_as(const T& defaultValue) const {
		return get_as<T>();
	}

	template <class T>
	void put(const key_type& key, T&& value) {
		operator[](key) = std::forward<T>(value);
	}

	void put_child(const key_type& key, const Context& value) {
		operator[](key) = value;
	}

	void put_child(const key_type& key, Context&& value) {
		operator[](key) = std::move(value);
	}
//	v1::Context legacy

	template <class T>
	T& as() { return ContextAdapter<T>::repr(*_base); }

	template <class T>
	const T& as() const { return ContextAdapter<T>::repr(*_base); }

private:

	template <class Target_type, class... Types>
	inline typename enumeration_types<Target_type, Types...>::max_type
	retype_as(const typename std::enable_if<!(std::is_same<void, typename enumeration_types<Types...>::max_type>::value), void*>::type = nullptr) const
	{
		using retype = typename enumeration_types<Target_type, Types...>::max_type;

		return is<Target_type>() ?
					static_cast<retype>(get<Target_type>()) :
					retype_as<Types...>();
	}

	template <class Target_type, class... Types>
	inline typename enumeration_types<Target_type, Types...>::max_type
	retype_as(const typename std::enable_if<(std::is_same<void, typename enumeration_types<Types...>::max_type>::value), void*>::type = nullptr) const
	{
		using retype = typename enumeration_types<Target_type, Types...>::max_type;
		if(is<Target_type>())
			return static_cast<retype>(get<Target_type>());
		throw std::runtime_error("(retype_as): Bad conversion error!");
	}

	Context(std::unique_ptr<ContextBase>&& base) : _base(std::move(base)) {}

	template<typename T>
	typename std::enable_if<!std::is_same<typename std::decay<T>::type, Context>::value>::type push_back_impl(T&& data)
	{
		_base->push_back(Context(std::forward<T>(data)));
	}
	void push_back_impl(const Context& data) {
		_base->push_back(data);
	}
	void push_back_impl(Context&& data) {
		_base->push_back(std::move(data));
	}

	std::unique_ptr<ContextBase> _base;

	template<typename T, bool isConst>
	class ValueIterator : public ValueIteratorBase<isConst>
	{
	using value_type=typename ValueIteratorBase<isConst>::value_type;
	using difference_type=typename ValueIteratorBase<isConst>::difference_type;

	public:
		using IterType = typename std::conditional<isConst, typename T::const_iterator, typename T::iterator>::type;

		ValueIterator(IterType wrapped) : wrapped_(wrapped) {}

		virtual ~ValueIterator() override {}

		ValueIterator* clone() const override {
			return new ValueIterator(*this);
		}

		virtual value_type& operator*() const override {
			return getValue<T>(wrapped_);
		}

		virtual void operator++() override {
			++wrapped_;
		}

		virtual void operator--() override {
			--wrapped_;
		}

		virtual difference_type operator-(const ValueIteratorBase<isConst>& other) const override {
			return distance<T>(dynamic_cast<const ValueIterator<T, isConst>&>(other).wrapped_);
		}

		virtual bool operator<(const ValueIteratorBase<isConst>& other) const override {
			return compareLess<T>(dynamic_cast<const ValueIterator<T, isConst>&>(other).wrapped_);
		}

		virtual bool operator>(const ValueIteratorBase<isConst>& other) const override {
			return compareMore<T>(dynamic_cast<const ValueIterator<T, isConst>&>(other).wrapped_);
		}

		virtual void advance(const difference_type i) override {
			advanceImpl<T>(i);
		}

		IterType getIterator() { return wrapped_; }

		virtual bool equalTo(const ValueIteratorBase<isConst>& other) const override {
			return wrapped_ == dynamic_cast<const ValueIterator<T, isConst>&>(other).wrapped_;
		}

		const key_type& key() const override {
			return getKey<T>(wrapped_);
		}

	private:

		template<typename C>
		typename std::enable_if<is_associative_container<C>::value, const key_type&>::type getKey(typename C::iterator iter) const { return iter->first;}

		template<typename C>
		typename std::enable_if<!is_associative_container<C>::value, const key_type&>::type getKey(typename C::iterator /*iter*/) const {
			throw std::runtime_error("no keys for non-associative container");
		}

		template<typename C>
		typename std::enable_if<is_associative_container<C>::value, const key_type&>::type getKey(typename C::const_iterator iter) const { return iter->first;}

		template<typename C>
		typename std::enable_if<!is_associative_container<C>::value, const key_type&>::type getKey(typename C::const_iterator /*iter*/) const {
			throw std::runtime_error("no keys for non-associative container");
		}

		template<typename C> typename std::enable_if<is_associative_container<C>::value, value_type&>::type getValue(typename C::iterator iter) const
		{
			typename C::reference tmp = *iter;
			return tmp.second;
		}

		template<typename C> typename std::enable_if<!is_associative_container<C>::value, value_type&>::type getValue(typename C::iterator iter) const
		{
			return *iter;
		}

		template<typename C> typename std::enable_if<is_associative_container<C>::value, value_type&>::type getValue(typename C::const_iterator iter) const
		{
			typename C::const_reference tmp = *iter;
			return tmp.second;
		}

		template<typename C> typename std::enable_if<!is_associative_container<C>::value, value_type&>::type getValue(typename C::const_iterator iter) const
		{
			return *iter;
		}

		template<typename C>
		typename std::enable_if<!is_associative_container<C>::value, difference_type>::type distance(IterType iter) const
		{
			return wrapped_ - iter;
		}

		template<typename C>
		typename std::enable_if<is_associative_container<C>::value, difference_type>::type distance(IterType /*iter*/) const {
			throw std::runtime_error("no random access iterator for associative container");
		}

		template<typename C>
		typename std::enable_if<!is_associative_container<C>::value>::type advanceImpl(const difference_type i)
		{
			std::advance(wrapped_, i);
		}

		template<typename C>
		typename std::enable_if<is_associative_container<C>::value>::type advanceImpl(const difference_type /*i*/) {
			throw std::runtime_error("no random access iterator for associative container");
		}

		template<typename C>
		typename std::enable_if<!is_associative_container<C>::value, bool>::type compareLess(IterType iter) const
		{
			return wrapped_ < iter;
		}

		template<typename C>
		typename std::enable_if<is_associative_container<C>::value, bool>::type compareLess(IterType /*iter*/) const {
			throw std::runtime_error("no random access iterator for associative container");
		}

		template<typename C>
		typename std::enable_if<!is_associative_container<C>::value, bool>::type compareMore(IterType iter) const
		{
			return wrapped_ > iter;
		}

		template<typename C>
		typename std::enable_if<is_associative_container<C>::value, bool>::type compareMore(IterType iter) const {
			throw std::runtime_error("no random access iterator for associative container");
		}

		IterType wrapped_;
	};
};

template<typename Impl, typename Base, bool isConst>
template<typename T>
typename std::enable_if<std::is_same<Impl, Context::ValueContextIterator<isConst>>::value, typename T::iterator>::type Context::ContextIterator<Impl, Base, isConst>::getIterator()
{
	return dynamic_cast<Context::ValueIterator<T, isConst>*>(iteratorImpl.get())->getIterator();
}

template<typename Impl, typename Base, bool isConst>
template<typename T>
typename std::enable_if<std::is_same<Impl, Context::KeyValueContextIterator<isConst>>::value, typename T::iterator>::type Context::ContextIterator<Impl, Base, isConst>::getIterator()
{
	return iteratorImpl->getIterator();
}

class Context::VectorContextBase : public Context::ContextBase
{
public:
	VectorContextBase(const sequence_container_type& data) : _data(data) {}
	VectorContextBase(sequence_container_type&& data) : _data(std::move(data)) {}

	template <class T>
	VectorContextBase(std::initializer_list<T>&& data)
	{
		_data.resize(data.size());
		size_t i = 0;
		for (const auto& item : data)
		{
			_data[i] = Context(item);
			++i;
		}
	}

	virtual std::unique_ptr<ContextBase> deep_copy_ptr() override {
		return std::unique_ptr<VectorContextBase>(new VectorContextBase(*this));
	}

	virtual ValueContextIterator<> begin() override {
		return ValueContextIterator<>(_data.begin());
	}

	virtual ValueContextIterator<> end() override {
		return ValueContextIterator<>(_data.end());
	}

	virtual ValueContextIterator<true> cbegin() const override {
		return ValueContextIterator<true>(_data.begin());
	}

	virtual ValueContextIterator<true> cend() const override {
		return ValueContextIterator<true>(_data.end());
	}

	virtual ValueContextIterator<> erase(ValueContextIterator<> pos) override {
		return _data.erase(pos.getIterator<sequence_container_type>());
	}

	virtual ValueContextIterator<> erase(ValueContextIterator<> first, ValueContextIterator<> last) override {
		return _data.erase(first.getIterator<sequence_container_type>(), last.getIterator<sequence_container_type>());
	}

	size_t size() const override { return _data.size(); }
	Context& at(int index) override {
		return _data.at((index > -1) ? static_cast<size_t>(index) : static_cast<size_t>(_data.size() + index) );
	}

	const Context& at(int index) const override {
		return _data.at((index > -1) ? static_cast<size_t>(index) : static_cast<size_t>(_data.size() + index) );
	}

	void push_back(const Context& data) override {_data.push_back(data); }

	void push_back(Context&& data) override {_data.push_back(std::move(data));}

	bool compare(const Context& data) const override
	{
		return ((data.isArray() && data.size() == size()) ? std::equal(cbegin(), cend(), data.cbegin(),
		[](const Context &a, const Context &b)
		{ return a.compare(b); }) : false);
	}

private:
	sequence_container_type _data;
};

template <typename T>
struct Context::ContextAdapter<std::initializer_list<T>, typename std::enable_if<!is_pair<T>::value>::type>
{
	using Base = Context::VectorContextBase;

	static std::vector<T> value(ContextBase&) { throw std::runtime_error("not applicable"); }
	static std::vector<T>& repr(ContextBase&) { throw std::runtime_error("not applicable"); }

	static std::unique_ptr<ContextBase> base(std::initializer_list<T>&& data) {		//takes r-value only!! not perfect forward
		return std::unique_ptr<ContextBase>(new Base(std::move(data)));
	}

	static bool isTypeMatch(ContextBase&) { return false; }
};

class Context::MapContextBase : public Context::ContextBase
{

public:
	MapContextBase(const associative_container_type& data) : _data(data) {}
	MapContextBase(associative_container_type&& data) : _data(std::move(data)) {}

	template <class T>
	MapContextBase(std::initializer_list<T>&& data)
	{
		for (auto& item: data)
			_data[item.first] = Context(item.second);
	}

	virtual std::unique_ptr<ContextBase> deep_copy_ptr() override {
		return std::unique_ptr<MapContextBase>(new MapContextBase(*this));
	}

	virtual ValueContextIterator<> begin() override {
		return ValueContextIterator<>(_data.begin());
	}

	virtual ValueContextIterator<> end() override {
		return ValueContextIterator<>(_data.end());
	}

	virtual KeyValueContextIterator<> kvbegin() override {
		return KeyValueContextIterator<>(_data.begin());
	}
	virtual KeyValueContextIterator<> kvend() override {
		return KeyValueContextIterator<>(_data.end());
	}

	virtual ValueContextIterator<true> cbegin() const override {
		return ValueContextIterator<true>(_data.begin());
	}

	virtual ValueContextIterator<true> cend() const override {
		return ValueContextIterator<true>(_data.end());
	}

	virtual KeyValueContextIterator<true> kvcbegin() const override {
		return KeyValueContextIterator<true>(_data.begin());
	}
	virtual KeyValueContextIterator<true> kvcend() const override {
		return KeyValueContextIterator<true>(_data.end());
	}

	virtual ValueContextIterator<> erase(ValueContextIterator<> pos) override {
		return _data.erase(pos.getIterator<associative_container_type>());
	}

	virtual ValueContextIterator<> erase(ValueContextIterator<> first, ValueContextIterator<> last) override {
		return _data.erase(first.getIterator<associative_container_type>(), last.getIterator<associative_container_type>());
	}

	virtual KeyValueContextIterator<> erase(KeyValueContextIterator<> pos) override {
		return _data.erase(pos.getIterator<associative_container_type>());
	}

	virtual KeyValueContextIterator<> erase(KeyValueContextIterator<> first, KeyValueContextIterator<> last) override {
		return _data.erase(first.getIterator<associative_container_type>(), last.getIterator<associative_container_type>());
	}

	virtual size_t erase(const key_type& key) override
	{ return _data.erase(key); }

	virtual bool contains(const key_type& key) const override
	{ return (_data.find(key)!=_data.cend());}

	virtual size_t count(const key_type& key) const override
	{ return _data.count(key);}

	virtual ValueContextIterator<> find(const key_type& key) override
	{
		return ValueContextIterator<>(_data.find(key));
	}

	virtual ValueContextIterator<true> cfind(const key_type& key) const override
	{
		return ValueContextIterator<true>(_data.find(key));
	}

	virtual KeyValueContextIterator<> kvfind(const key_type& key) override
	{
		return KeyValueContextIterator<>(_data.find(key));
	}

	virtual KeyValueContextIterator<true> kvcfind(const key_type& key) const override
	{
		return KeyValueContextIterator<true>(_data.find(key));
	}

	size_t size() const override { return _data.size(); }
	Context& operator [](const key_type& key) override { return _data[key]; }
	const Context& operator [](const key_type& key) const override { return _data.at(key); }
	Context& at(const key_type& key) override { return _data.at(key); }
	const Context& at(const key_type& key) const override { return _data.at(key); }

	bool compare(const Context& data) const override
	{
		return ((data.isObject() && data.size() == size()) ? std::equal(kvcbegin(), kvcend(), data.kvbegin(),
		[](const associative_container_type::value_type &a, const associative_container_type::value_type &b)
		{return (a.first == b.first && a.second.compare(b.second));}) : false);
	}
private:
	associative_container_type _data;
};

template <typename T>
struct Context::ContextAdapter<std::initializer_list<T>, typename std::enable_if<is_pair<T>::value>::type>
{
	using Base = Context::MapContextBase;
	using value_type = typename T::second_type;

	static std::map<key_type, value_type> value(ContextBase& /*base*/) { throw std::runtime_error("not applicable"); }
	static std::map<key_type, value_type>& repr(ContextBase& /*base*/) { throw std::runtime_error("not applicable"); }

	static std::unique_ptr<ContextBase> base(std::initializer_list<T>&& data) {		//takes r-value only!! not perfect forward
		return std::unique_ptr<ContextBase>(new Base(std::move(data)));
	}

	static bool isTypeMatch(ContextBase& /*base*/) { return false; }
};

inline bool Context::isNone() const { return dynamic_cast<NoneContextBase*>(_base.get()); }
inline bool Context::isArray() const { return dynamic_cast<VectorContextBase*>(_base.get()); }
inline bool Context::isObject() const { return dynamic_cast<MapContextBase*>(_base.get()); }

inline Context& Context::operator [](const key_type& key) {
	if (isNone())
		_base = std::unique_ptr<ContextBase>(new MapContextBase(associative_container_type()));
	return _base->operator[](key);
}

inline Context& Context::operator [](const int index) {
	if (isNone() && (index == 0))
	{
		_base = std::unique_ptr<ContextBase>(new VectorContextBase(sequence_container_type()));
		_base->push_back(Context());
	}
	return _base->operator[](index);
}

inline const Context& Context::operator [](const key_type& key) const {
	return _base->operator[](key);
}

inline const Context& Context::operator [](const int index) const {
	return _base->operator[](index);
}

template<typename T>
void Context::push_back(T&& data)
{
	if (isNone())
		_base = std::unique_ptr<ContextBase>(new VectorContextBase(sequence_container_type()));
	push_back_impl(std::forward<T>(data));
}

inline Context Context::make_array()
{
	return Context(std::initializer_list<Context>());
}

inline Context Context::make_object()
{
	return Context(std::initializer_list<std::pair<key_type, Context>>());
}
} // data

} // tdv

#endif // TDV_DATA_CONTEXT_V2_CONTEXT_H_
