#ifndef PCOMB_STRING_PARSER_H
#define PCOMB_STRING_PARSER_H

#include "Parser/Parser.h"

#include <experimental/string_view>

namespace pcomb
{

// StringParser matches a string and returns that string as its attribute
class StringParser: public Parser<std::experimental::string_view>
{
private:
	using StringView = std::experimental::string_view;
	StringView pattern;
public:
	using OutputType = StringView;
	using ResultType = typename Parser<StringView>::ResultType;

	StringParser(StringView s): pattern(s) {}

	ResultType parse(const InputStream& input) const override
	{
		auto ret = ResultType(input);

		auto inputView = std::experimental::string_view(input.getRawBuffer(), pattern.size());
		if (pattern.compare(inputView) == 0)
			ret = ResultType(input.consume(pattern.size()), inputView);
		
		return ret;
	}
};

inline StringParser str(const std::experimental::string_view& s)
{
	return StringParser(s);
}

}

#endif
