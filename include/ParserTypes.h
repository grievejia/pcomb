#ifndef PCOMB_PARSER_TYPE_H
#define PCOMB_PARSER_TYPE_H

#include "StringRef.h"

#include <type_traits>
#include <experimental/optional>

namespace pcomb
{

// The return type of our parser
template <typename A>
using ParseResult = std::experimental::optional<std::pair<A, StringRef>>;

template <typename Parser>
using ParserAttrType = typename std::remove_reference_t<Parser>::AttrType;

// The templates provided below are all used to deduce some kind of type informations. They could all be replaced if we have Concept support in the compiler.

// Check if the given type is callable with a single argument ArgType and a return type RetType
template <typename T, typename ArgType>
class IsCallableWithArgType
{
private:
	template<typename U> static auto calltest(int) -> decltype(std::declval<U>()(std::declval<ArgType>()), std::true_type());
	template<typename> static std::false_type calltest(...);
public:
	static constexpr bool value = decltype(calltest<T>(0))::value;
};

// Check if the given type is callable with a single argument ArgType and a return type RetType
template <typename T, typename RetType, typename ArgType>
class IsCallableWithRetAndArgType
{
private:
	template<typename U> static auto calltest(int) -> decltype(std::declval<U>()(std::declval<ArgType>()));
	template<typename> static std::false_type calltest(...);
public:
	static constexpr bool value = std::is_convertible<decltype(calltest<T>(0)), RetType>::value;
};

// For a type to be a Parser, we require it to have the parse() method and AttrType typedef. The following template is used to compute whether a given class is qualified to be called a Parser
template<typename T, typename A = ParserAttrType<T>>
class IsParser
{
private:
	template<typename U> static auto calltest(int) -> decltype(std::declval<U>().parse(std::declval<StringRef>()));
	template<typename> static std::false_type calltest(...);
public:
	static constexpr bool value = std::is_convertible<decltype(calltest<T>(0)), ParseResult<A>>::value;
};

// MoreGeneralType returns P2 if P1 is convertible to P2 and returns P1 if P2 is convertible to P1. 
template <typename P1, typename P2, typename = void>
struct MoreGeneralType { static constexpr bool exist = false; };
template <typename P1>
struct MoreGeneralType<P1, P1>
{
	static constexpr bool exist = true;
	using type = P1;
};
template <typename P1, typename P2>
struct MoreGeneralType <P1, P2, typename std::enable_if<std::is_convertible<P1, P2>::value && !std::is_same<P1, P2>::value>::type>
{
	static constexpr bool exist = true;
	using type = P2;
};
template <typename P1, typename P2>
struct MoreGeneralType <P1, P2, typename std::enable_if<std::is_convertible<P2, P1>::value && !std::is_same<P1, P2>::value>::type>
{
	static constexpr bool exist = true;
	using type = P1;
};

}

#endif
