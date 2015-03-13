#ifndef PCOMB_REGEX_PARSER_H
#define PCOMB_REGEX_PARSER_H

#include "Type/InputStreamConcept.h"
#include "Type/ParseResult.h"

#include <experimental/string_view>
#include <regex>

namespace pcomb
{

// RegexParser takes a StringRef as regex and mathes the start of the input string against that regex.
// RegexParser is strictly more powerful than StringParser. But I would expect that StringParser is cheaper.
class RegexParser
{
private:
	using StringView = std::experimental::string_view;
	StringView regexStr;
public:
	using AttrType = StringView;

	RegexParser(StringView r): regexStr(r) {}

	template <typename InputStream>
	ParseResult<InputStream, AttrType> parse(const InputStream& input) const
	{
		static_assert(IsInputStream<InputStream>::value, "Parser's input type must be an InputStream!");

		auto ret = ParseResult<InputStream, AttrType>(input);

		auto regex = std::regex(regexStr.data());
		auto res = std::cmatch();

		auto rawStr = input.getRawBuffer();
		if (std::regex_search(rawStr, res, regex, std::regex_constants::match_continuous))
		{
			auto matchLen = res.length(0);
			ret.setResult(input.consume(matchLen), StringView(rawStr, matchLen));
		}
		
		return ret;
	}
};

inline RegexParser regex(const std::experimental::string_view& s)
{
	return RegexParser(s);
}

}

#endif
