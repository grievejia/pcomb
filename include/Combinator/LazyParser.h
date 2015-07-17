#ifndef PCOMB_LAZY_PARSER_H
#define PCOMB_LAZY_PARSER_H

#include "Parser/Parser.h"

#include <memory>

namespace pcomb
{

// LazyParser allows the user to declare a parser first before giving its full definitions. This is useful for recursive grammar definitions.
// It is essentially a pointer to another parser.

template <typename O>
class LazyRefParser: public Parser<O>
{
private:
	const Parser<O>** parser;
public:
	using OutputType = O;
	using ResultType = typename Parser<O>::ResultType;

	LazyRefParser(const Parser<O>** p): parser(p) {}

	ResultType parse(const InputStream& input) const override final
	{
		assert(parser != nullptr);
		assert(*parser != nullptr);
		return (*parser)->parse(input);
	}
};

template <typename O>
class LazyParser: public Parser<O>
{
private:
	std::unique_ptr<const Parser<O>*> parser;
public:
	using OutputType = O;
	using ResultType = typename Parser<O>::ResultType;

	LazyParser(): parser(std::make_unique<const Parser<O>*>(nullptr)) {}

	LazyRefParser<O> setParser(const Parser<OutputType>& p)
	{
		*parser = &p;
		return getRef();
	}

	LazyRefParser<O> getRef() const
	{
		return LazyRefParser<O>(parser.get());
	}

	ResultType parse(const InputStream& input) const
	{
		assert(parser);
		assert(*parser != nullptr);
		return (*parser)->parse(input);
	}
};

}

#endif
