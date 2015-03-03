#ifndef PCOMB_STRING_PARSER_H
#define PCOMB_STRING_PARSER_H

#include "Type/InputStreamConcept.h"
#include "Type/ParseResult.h"

#include <experimental/string_view>

namespace pcomb
{

// StringParser matches a string and returns that string as its attribute
class StringParser
{
private:
	using StringView = std::experimental::string_view;
	StringView pattern;
public:
	using AttrType = StringView;

	StringParser(StringView s): pattern(s) {}

	template <typename InputStream>
	ParseResult<InputStream, AttrType> parse(const InputStream& input) const
	{
		static_assert(IsInputStream<InputStream>::value, "Parser's input type must be an InputStream!");

		auto ret = ParseResult<InputStream, AttrType>();

		auto inputView = std::experimental::string_view(input.getRawBuffer(), pattern.size());
		if (pattern.compare(inputView) == 0)
			ret.setResult(input.consume(pattern.size()), inputView);
		
		return ret;
	}
};

inline StringParser str(const std::experimental::string_view& s)
{
	return StringParser(s);
}

}

#endif
