#ifndef PCOMB_PARSER_ADAPTER_H
#define PCOMB_PARSER_ADAPTER_H

#include "Type/InputStreamConcept.h"
#include "Type/ParserConcept.h"
#include "Type/ParseResult.h"

namespace pcomb
{

// The ParserAdapter combinator takes another parser p0, fails if p0 fails, and convert p0's result to T using the function specified by the user if succeeds.
template <typename Converter, typename ParserA>
class ParserAdapter
{
private:
	using AdapterInputType = ParserAttrType<ParserA>;
	using AdapterOutputType = std::result_of_t<Converter(AdapterInputType)>;

	ParserA parser;
	Converter conv;
public:
	using AttrType = AdapterOutputType;

	ParserAdapter(ParserA&& p, Converter&& c): parser(std::forward<ParserA>(p)), conv(std::forward<Converter>(c)) {}

	template <typename InputStream>
	ParseResult<InputStream, AttrType> parse(const InputStream& input) const
	{
		static_assert(IsInputStream<InputStream>::value, "Parser's input type must be an InputStream!");
		static_assert(IsParser<ParserA, InputStream>::value, "ManyParser only accepts parser type");

		ParseResult<InputStream, AttrType> ret(input);
		auto pResult = parser.parse(input);
		if (pResult.success())
			ret.setResult(pResult.moveInputStream(), conv(std::move(pResult.moveAttribute())));
		return ret;
	}
};

template <typename Converter, typename ParserA>
ParserAdapter<Converter, ParserA> rule(ParserA&& p, Converter&& c)
{
	return ParserAdapter<Converter, ParserA>(std::forward<ParserA>(p), std::forward<Converter>(c));
}

}

#endif
