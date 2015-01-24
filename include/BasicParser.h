#ifndef PCOMB_BASIC_PARSER_H
#define PCOMB_BASIC_PARSER_H

#include "ParserTypes.h"

#include <regex>

namespace pcomb
{

// StringParser matches a string and returns that string as its attribute
class StringParser
{
private:
	StringRef pattern;
public:
	using AttrType = StringRef;

	StringParser(const StringRef& s): pattern(s) {}

	ParseResult<AttrType> parse(const StringRef& str) const
	{
		auto ret = ParseResult<AttrType>();
		if (str.startswith(pattern))
			ret = std::make_pair(pattern, str.dropFront(pattern.size()));
		
		return ret;
	}
};

// RegexParser takes a StringRef as regex and mathes the start of the input string against that regex.
// RegexParser is strictly more powerful than StringParser. But I would expect that StringParser is cheaper.
class RegexParser
{
private:
	StringRef regexStr;
public:
	using AttrType = StringRef;

	RegexParser(const StringRef& r): regexStr(r) {}

	ParseResult<AttrType> parse(const StringRef& str) const
	{
		auto ret = ParseResult<AttrType>();

		auto regex = std::regex(regexStr.raw_pointer());
		auto res = std::cmatch();

		if (std::regex_search(str.raw_pointer(), res, regex, std::regex_constants::match_continuous))
			ret = std::make_pair(StringRef(res[0]), str.dropFront(res.length(0)));
		
		return ret;
	}
};

// PredicateCharParser matches a char that satisfies a predicate and returns that char as its attribute
template <typename Pred>
class PredicateCharParser
{
private:
	Pred pred;
public:
	static_assert(IsCallableWithRetAndArgType<Pred, bool, char>::value, "PredicateCharParser only accepts functors as predicate");

	using AttrType = char;

	PredicateCharParser(Pred&& p): pred(std::move(p)) {}

	ParseResult<AttrType> parse(const StringRef& str) const
	{
		if (!str.empty() && pred(str[0]))
			return std::experimental::make_optional(std::make_pair(str[0], str.dropFront()));
		else
			return ParseResult<AttrType>();
	}
};

template <typename Pred>
PredicateCharParser<Pred> getCharParserWithPredicate(Pred p)
{
	return PredicateCharParser<Pred>(std::move(p));
}

class CharEqPredicate
{
private:
	char ch;
public:
	CharEqPredicate(char c): ch(c) {}

	bool operator()(char c) const
	{
		return c == ch;
	}
};

class CharRangePredicate
{
private:
	char lo, hi;
public:
	CharRangePredicate(char l, char h): lo(l), hi(h) {}

	bool operator()(char c) const
	{
		return c >= lo && c <= hi;
	}
};

inline PredicateCharParser<CharEqPredicate> ch(char c)
{
	return PredicateCharParser<CharEqPredicate>(CharEqPredicate(c));
}

// The enable_if here is to avoid instantiating this template when CustomPredicate is char
template <typename CustomPredicate>
std::enable_if_t<!std::is_same<CustomPredicate, char>::value, PredicateCharParser<CustomPredicate>> ch(CustomPredicate p)
{
	return PredicateCharParser<CustomPredicate>(std::move(p));
}

inline PredicateCharParser<CharRangePredicate> range(char l, char h)
{
	return PredicateCharParser<CharRangePredicate>(CharRangePredicate(l, h));
}

inline StringParser str(const StringRef& s)
{
	return StringParser(s);
}

inline RegexParser regex(const StringRef& s)
{
	return RegexParser(s);
}

}

#endif
