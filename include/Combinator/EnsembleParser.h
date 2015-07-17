#ifndef PCOMB_ENSEMBLE_PARSER_H
#define PCOMB_ENSEMBLE_PARSER_H

#include "Combinator/TokenParser.h"
#include "Combinator/LexemeParser.h"

namespace pcomb
{

template <typename ParserA>
class EndParser: public Parser<typename ParserA::OutputType>
{
private:
	static_assert(std::is_base_of<Parser<typename ParserA::OutputType>, ParserA>::value, "EndParser only accepts parser type");

	ParserA pa;
public:
	using OutputType = typename ParserA::OutputType;
	using ResultType = typename Parser<OutputType>::ResultType;

	EndParser(const ParserA& a): pa(a) {}
	EndParser(ParserA&& a): pa(std::move(a)) {}

	ResultType parse(const InputStream& input) const override final
	{
		auto result = pa.parse(input);
		if (result.success())
		{
			auto resStream = result.getInputStream();
			if (resStream.isEOF())
				return std::move(result);
			else
				return ResultType(resStream);
		}
		else
			return std::move(result);
	}
};

template <typename ParserA>
auto endp(ParserA&& pa)
{
	using ParserType = std::remove_reference_t<ParserA>;
	return EndParser<ParserType>(std::forward<ParserA>(pa));
}

template <typename ParserA>
auto file(ParserA&& pa)
{
	return rule(
		endp(seq(
			lexeme(token(pa)),
			ch(static_cast<char>(std::char_traits<char>::eof()))
		)),
		[] (auto&& pair)
		{
			using ParserType = std::remove_reference_t<ParserA>;
			return std::forward<typename ParserType::OutputType>(std::get<0>(pair));
		}
	);
}

template <typename ParserA>
auto line(ParserA&& pa)
{
	return rule(
		endp(seq(
			lexeme(token(pa)),
			ch('\n')
		)),
		[] (auto&& pair)
		{
			using ParserType = std::remove_reference_t<ParserA>;
			return std::forward<typename ParserType::OutputType>(std::get<0>(pair));
		}
	);
}

template <typename ParserA>
auto bigstr(ParserA&& pa)
{
	return endp(lexeme(token(pa)));
}

}

#endif
