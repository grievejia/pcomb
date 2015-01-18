#ifndef PCOMB_BASIC_PARSER_H
#define PCOMB_BASIC_PARSER_H

#include "ParserTypes.h"

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

	ParseResult<AttrType> parse(StringRef str) const
	{
		if (str.startswith(pattern))
			return std::experimental::make_optional(std::make_pair(pattern, str.dropFront(pattern.size())));
		else
			return ParseResult<AttrType>();
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

	ParseResult<AttrType> parse(StringRef str) const
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

PredicateCharParser<CharEqPredicate> ch(char c)
{
	return PredicateCharParser<CharEqPredicate>(CharEqPredicate(c));
}

PredicateCharParser<CharRangePredicate> range(char l, char h)
{
	return PredicateCharParser<CharRangePredicate>(CharRangePredicate(l, h));
}

StringParser str(const StringRef& s)
{
	return StringParser(s);
}

}

#endif
