#ifndef PCOMB_CALLABLE_CONCEPT_H
#define PCOMB_CALLABLE_CONCEPT_H

#include <type_traits>

namespace pcomb
{

// Check if the given type is callable with a single argument ArgType and a return type RetType
template <typename T, typename ArgType>
class IsCallableWithArgType
{
private:
	template<typename U>
	static auto calltest(int) -> decltype(std::declval<U>()(std::declval<ArgType>()), std::true_type());

	template<typename>
	static auto calltest(...) -> std::false_type;
public:
	static constexpr bool value = decltype(calltest<T>(0))::value;
};

// Check if the given type is callable with a single argument ArgType and a return type RetType
template <typename T, typename RetType, typename ArgType>
class IsCallableWithRetAndArgType
{
private:
	template<typename U>
	static auto calltest(int) -> decltype(std::declval<U>()(std::declval<ArgType>()));

	template<typename>
	static auto calltest(...) -> std::false_type;
public:
	static constexpr bool value = std::is_convertible<decltype(calltest<T>(0)), RetType>::value;
};

}

#endif
