#ifndef PCOMB_SEQ_PARSER_H
#define PCOMB_SEQ_PARSER_H

#include "Type/InputStreamConcept.h"
#include "Type/ParserConcept.h"
#include "Type/ParseResult.h"

#include <tuple>

namespace pcomb
{

// The SeqParser combinator applies multiple parsers (p0, p1, p2, ...) consequtively. If p0 succeeds, it parse the rest of the input string with (p1, p2, ...). If one of the parsers fails, the entire combinator fails. Otherwise, return the result in a tuple
// This is literally the most brain-burning part of this library. You can see that there are LOTS of template metaprogramming going on here and making sure that all of them works is not easy. Even simple tasks like fmap on the type of a tuple require me to craft a recursion somehow... The good thing is that we have decltype and declval, which save some work.
template <typename ...Parsers>
class SeqParser
{
private:
	std::tuple<Parsers...> parsers;

	// AttrTypeImpl is a (recursive) template to compute the Attribute type of the seq parser
	template <typename Tuple, size_t I>
	struct AttrTypeImpl
	{
		using type = decltype(
			std::tuple_cat(
				std::declval<typename AttrTypeImpl<Tuple, I-1>::type>(),
				std::make_tuple(std::declval<ParserAttrType<std::tuple_element_t<I-1, Tuple>>>())
			)
		);
	};
	template <typename Tuple>
	struct AttrTypeImpl<Tuple, 1>
	{
		using type = std::tuple<ParserAttrType<std::tuple_element_t<0, Tuple>>>;
	};

	template <typename InputStream, typename Tuple, size_t I>
	struct SeqNParserImpl
	{
		static auto parse(const Tuple& t, const InputStream& input)
		{
			using CurrentParserType = decltype(std::get<I-1>(t));
			static_assert(IsParser<CurrentParserType, InputStream>::value, "SeqParser only accepts parser type");
			using CurrentParserAttrType = ParserAttrType<CurrentParserType>;

			auto prevRes = SeqNParserImpl<InputStream, Tuple, I-1>::parse(t, input);
			// The decay here is crucial because without it PrevParserAttrTypes will be a const reference, which causes subsequent troubles
			using PrevParserAttrTypes = std::decay_t<decltype(prevRes.getAttribute())>;
			using CurrentParserAttrTypes = decltype(std::tuple_cat(std::declval<PrevParserAttrTypes>(), std::make_tuple(std::declval<CurrentParserAttrType>())));

			auto ret = ParseResult<InputStream, CurrentParserAttrTypes>(input);
			if (prevRes.success())
			{
				auto curRes = std::get<I-1>(t).parse(prevRes.getInputStream());
				if (curRes.success())
					ret.setResult(curRes.moveInputStream(), std::tuple_cat(prevRes.moveAttribute(), std::make_tuple(curRes.moveAttribute())));
			}
			
			return ret;
		}
	};

	template <typename InputStream, typename Tuple>
	struct SeqNParserImpl<InputStream, Tuple, 1>
	{
		static auto parse(const Tuple& t, const InputStream& input)
		{
			using CurrentParserType = decltype(std::get<0>(t));
			static_assert(IsParser<CurrentParserType, InputStream>::value, "SeqParser only accepts parser type");
			using CurrentParserAttrType = ParserAttrType<CurrentParserType>;
			using CurrentParserAttrTypes = std::tuple<CurrentParserAttrType>;

			auto res = std::get<0>(t).parse(input);
			auto ret = ParseResult<InputStream, CurrentParserAttrTypes>(input);
			if (res.success())
				ret.setResult(res.moveInputStream(), std::make_tuple(std::move(res.moveAttribute())));
			
			return ret;
		}
	};
public:
	SeqParser(Parsers&&... ps): parsers(std::forward_as_tuple(ps...)) {}

	using AttrType = typename AttrTypeImpl<decltype(parsers), sizeof...(Parsers)>::type;

	template <typename InputStream>
	ParseResult<InputStream, AttrType> parse(const InputStream& input) const
	{
		static_assert(IsInputStream<InputStream>::value, "Parser's input type must be an InputStream!");

		return SeqNParserImpl<InputStream, std::tuple<Parsers...>, sizeof...(Parsers)>::parse(parsers, input);
	}
};

template <typename ...Parsers>
SeqParser<Parsers...> seq(Parsers&&... parsers)
{
	return SeqParser<Parsers...>(std::forward<Parsers>(parsers)...);
}

}

#endif
