#ifndef PCOMB_ALT_PARSER_H
#define PCOMB_ALT_PARSER_H

#include "Type/InputStreamConcept.h"
#include "Type/MoreGeneralType.h"
#include "Type/ParserConcept.h"
#include "Type/ParseResult.h"

#include <tuple>

namespace pcomb
{

// The AltParser combinator applies multiple parser (p0, p1, p2, ...) in turn. If p0 succeeds, it returns what p0 returns; otherwise, it tries p1 and return what p1 returns if it succeeds; otherwise, try p2, and so on
// This class still needs lots of template metaprogramming. But it's easier than SeqParser becuase we have a stronger requirement on its template parameters (they must have compatible attribute types)
template <typename ...Parsers>
class AltParser
{
private:
	std::tuple<Parsers...> parsers;

	template <typename Tuple, size_t I>
	struct AttrTypeImpl
	{
	private:
		using PrevType = typename AttrTypeImpl<Tuple, I-1>::type;
		using ElemType = std::tuple_element_t<I-1, Tuple>;
		using CurrType = ParserAttrType<ElemType>;
	public:
		static_assert(MoreGeneralType<PrevType, CurrType>::exist, "AltParser's subparsers must be compatible!");
		using type = typename MoreGeneralType<PrevType, CurrType>::type;
	};
	template <typename Tuple>
	struct AttrTypeImpl<Tuple, 1>
	{
	private:
		using ElemType = std::tuple_element_t<0, Tuple>;
	public:
		using type = ParserAttrType<ElemType>;
	};

	template <typename InputStream, typename Tuple, size_t I>
	struct AltNParserImpl
	{
		static auto parse(const Tuple& t, const InputStream& input)
		{
			auto constexpr tupleId = std::tuple_size<Tuple>::value - I;
			static_assert(IsParser<decltype(std::get<tupleId>(t)), InputStream>::value, "AltParser only accepts parser type");
			auto res = std::get<tupleId>(t).parse(input);
			if (res.success())
				return res;
			else
				return AltNParserImpl<InputStream, Tuple, I-1>::parse(t, input);
		}
	};

	template <typename InputStream, typename Tuple>
	struct AltNParserImpl<InputStream, Tuple, 1>
	{
		static auto parse(const Tuple& t, const InputStream& input)
		{
			auto constexpr tupleId = std::tuple_size<Tuple>::value - 1;
			static_assert(IsParser<decltype(std::get<tupleId>(t)), InputStream>::value, "AltParser only accepts parser type");
			return std::get<tupleId>(t).parse(input);
		}
	};
public:
	AltParser(Parsers&&... ps): parsers(std::forward_as_tuple(ps...)) {}

	using AttrType = typename AttrTypeImpl<decltype(parsers), sizeof...(Parsers)>::type;

	template <typename InputStream>
	ParseResult<InputStream, AttrType> parse(const InputStream& input) const
	{
		static_assert(IsInputStream<InputStream>::value, "Parser's input type must be an InputStream!");

		return AltNParserImpl<InputStream, std::tuple<Parsers...>, sizeof...(Parsers)>::parse(parsers, input);
	}
};

template <typename ...Parsers>
AltParser<Parsers...> alt(Parsers&&... parsers)
{
	return AltParser<Parsers...>(std::forward<Parsers>(parsers)...);
}

}

#endif
