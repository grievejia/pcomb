#ifndef PCOMB_TOKEN_PARSER_H
#define PCOMB_TOKEN_PARSER_H

#include "Type/InputStreamConcept.h"
#include "Type/ParserConcept.h"
#include "Type/ParseResult.h"

namespace pcomb
{

// TokenParser takes a parser p and strip the whitespace of the input string before feed it into p
template <typename ParserA>
class TokenParser
{
private:
	ParserA parser;
public:

	using AttrType = ParserAttrType<ParserA>;

	TokenParser(ParserA&& p): parser(std::forward<ParserA>(p)) {}

	template <typename InputStream>
	ParseResult<InputStream, AttrType> parse(const InputStream& input) const
	{
		static_assert(IsInputStream<InputStream>::value, "Parser's input type must be an InputStream!");
		static_assert(IsParser<ParserA, InputStream>::value, "ManyParser only accepts parser type");

		auto resStream = input;
		while (!resStream.isEOF())
		{
			auto firstChar = resStream.getRawBuffer()[0];
			switch (firstChar)
			{
				case ' ':
				case '\t':
				case '\n':
				case '\v':
				case '\f':
				case '\r':
					resStream = resStream.consume(1);
					continue;
				default:
					break;
			}
			break;
		}
		return parser.parse(resStream);
	}
};

template <typename ParserA>
TokenParser<ParserA> token(ParserA&& p)
{
	return TokenParser<ParserA>(std::forward<ParserA>(p));
}

}

#endif
