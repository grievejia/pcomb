#ifndef PCOMB_MANY_PARSER_H
#define PCOMB_MANY_PARSER_H

#include "Type/InputStreamConcept.h"
#include "Type/ParserConcept.h"
#include "Type/ParseResult.h"

#include <vector>

namespace pcomb
{

// The ManyParser combinator applies one parser p0 repeatedly. The result of each application of p0 is pushed into a vector, which is the result of the entire combinator. If nonEmpty is true, the combinator will fail if the vector is empty.
template <typename ParserA>
class ManyParser
{
private:
	ParserA pa;
	bool nonEmpty;
public:
	using AttrType = std::vector<ParserAttrType<ParserA>>;

	ManyParser(ParserA&& a, bool moreThanOne = false): pa(std::forward<ParserA>(a)), nonEmpty(moreThanOne) {}

	template <typename InputStream>
	ParseResult<InputStream, AttrType> parse(const InputStream& input) const
	{
		static_assert(IsInputStream<InputStream>::value, "Parser's input type must be an InputStream!");
		static_assert(IsParser<ParserA, InputStream>::value, "ManyParser only accepts parser type");

		auto retVec = AttrType();
		auto resStream = input;

		while (true)
		{
			auto paResult = pa.parse(resStream);
			if (!paResult.success())
				break;

			retVec.emplace_back(paResult.moveAttribute());
			resStream = paResult.moveInputStream();
		}

		ParseResult<InputStream, AttrType> ret(input);
		if (!(nonEmpty && retVec.empty()))
			ret.setResult(std::move(resStream), std::move(retVec));
		return ret;
	}
};

template <typename ParserA>
ManyParser<ParserA> many(ParserA&& p0, bool moreThanOne = false)
{
	return ManyParser<ParserA>(std::forward<ParserA>(p0), moreThanOne);
}

}

#endif
