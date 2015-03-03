#ifndef PCOMB_INPUT_STREAM_CONCEPT_H
#define PCOMB_INPUT_STREAM_CONCEPT_H

#include <type_traits>

namespace pcomb
{

// InputStream is a type trait that specifies the required operations for an input type to be able to get parsed by pcomb
// Required interfaces:
// static bool isEOF(const T& stream);
// static const char* getRawBuffer(const T& stream);
// static T consume(const T& stream, size_t n);

namespace detail
{

template <typename T>
struct EOFCheck
{
private:
	template<typename U> static auto calltest(int) -> decltype(std::declval<U>().isEOF());
	template<typename> static auto calltest(...) -> std::false_type;
public:
	static constexpr bool value = std::is_same<decltype(calltest<T>(0)), bool>::value;
};

template <typename T>
struct RawBufferCheck
{
private:
	template<typename U> static auto calltest(int) -> decltype(std::declval<U>().getRawBuffer());
	template<typename> static auto calltest(...) -> std::false_type;
public:
	static constexpr bool value = std::is_convertible<decltype(calltest<T>(0)), const char*>::value;
};

template <typename T>
struct ConsumeCheck
{
private:
	template<typename U> static auto calltest(int) -> decltype(std::declval<U>().consume(std::declval<std::size_t>()));
	template<typename> static auto calltest(...) -> std::false_type;
public:
	static constexpr bool value = std::is_convertible<decltype(calltest<T>(0)), T>::value;
};

}	// end of namespace detail

template<typename T>
struct IsInputStream
{
	static constexpr bool value = detail::EOFCheck<T>::value && detail::RawBufferCheck<T>::value && detail::ConsumeCheck<T>::value;
};

}

#endif
