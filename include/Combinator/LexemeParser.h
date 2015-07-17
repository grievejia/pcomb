#ifndef PCOMB_LEXEME_PARSER_H
#define PCOMB_LEXEME_PARSER_H

#include "Parser/Parser.h"

#include <bitset>

namespace pcomb
{

// LexemeParser takes a parser p and returns a parser remove all trailing whitespaces after p succeeds
template <typename ParserA>
class LexemeParser: public Parser<typename ParserA::OutputType>
{
private:
	static_assert(std::is_base_of<Parser<typename ParserA::OutputType>, ParserA>::value, "LexemeParser only accepts parser type");

	auto static constexpr CharSize = sizeof(char) * 8;

	ParserA pa;
	std::bitset<1 << CharSize> charBits;

	void setCharBits(const std::experimental::string_view& whitespaces)
	{
		for (auto ch: whitespaces)
			charBits.set(static_cast<unsigned char>(ch));
	}
public:
	static_assert(std::is_base_of<Parser<typename ParserA::OutputType>, ParserA>::value, "TokenParser only accepts parser type");

	using OutputType = typename Parser<typename ParserA::OutputType>::OutputType;
	using ResultType = typename Parser<typename ParserA::OutputType>::ResultType;

	LexemeParser(const ParserA& p, const std::experimental::string_view& w): pa(p)
	{
		setCharBits(w);
	}
	LexemeParser(ParserA&& p, const std::experimental::string_view& w): pa(std::move(p))
	{
		setCharBits(w);
	}

	ResultType parse(const InputStream& input) const override final
	{
		auto result = pa.parse(input);
		if (result.success())
		{
			auto resStream = result.getInputStream();
			while (!resStream.isEOF())
			{
				auto firstChar = resStream.getRawBuffer()[0];
				if (charBits.test(static_cast<unsigned char>(firstChar)))
					resStream = resStream.consume(1);
				else
					break;
			}
			return ResultType(std::move(resStream), std::move(result).getOutput());
		}
		else
			return std::move(result);
	}
};

template <typename ParserA>
auto lexeme(ParserA&& p, const std::experimental::string_view& w = " \t\n\v\f\r")
{
	using ParserType = std::remove_reference_t<ParserA>;
	return LexemeParser<ParserType>(std::forward<ParserA>(p), w);
}

}

#endif
