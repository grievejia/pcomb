#ifndef PCOMB_PARSER_CONCEPT_H
#define PCOMB_PARSER_CONCEPT_H

#include "Type/InputStreamConcept.h"
#include "Type/ParseResult.h"

namespace pcomb
{

template <typename Parser>
using ParserAttrType = typename std::remove_reference_t<Parser>::AttrType;

// For a type to be a Parser, we require it to have the parse() method and AttrType typedef. The following template is used to compute whether a given class is qualified to be called a Parser with input type being I
template<typename T, typename I, typename A = ParserAttrType<T>>
class IsParser
{
private:
	template<typename U> static auto calltest(int) -> decltype(std::declval<U>().parse(std::declval<I>()));
	template<typename> static auto calltest(...) -> std::false_type;
public:
	static constexpr bool value = std::is_convertible<decltype(calltest<T>(0)), ParseResult<I, A>>::value;
};

}

#endif
