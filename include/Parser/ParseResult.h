#ifndef PCOMB_PARSE_RESULT_H
#define PCOMB_PARSE_RESULT_H

#include "InputStream/InputStream.h"

#include <cassert>
#include <experimental/optional>

namespace pcomb
{

template <typename Out>
class ParseResult
{
public:
	using OutputType = Out;
private:
	std::experimental::optional<OutputType> attr;
	InputStream input;
public:
	template <typename I>
	ParseResult(I&& i): input(std::forward<I>(i)) {}

	template <typename I, typename O>
	ParseResult(I&& i, O&& o): input(std::forward<I>(i)), attr(std::forward<O>(o)) {}

	bool success() const { return static_cast<bool>(attr); }
	bool hasError() const { return !success(); }

	template <typename O>
	void setOutput(O&& o)
	{
		attr = std::forward<O>(o);
	}

	const OutputType& getOutput() const&
	{
		assert(success());
		return (*attr);
	}
	OutputType&& getOutput() &&
	{
		assert(success());
		return std::move(*attr);
	}

	const InputStream& getInputStream() const&
	{
		return input;
	}
	InputStream&& getInputStream() &&
	{
		return std::move(input);
	}
};

}

#endif
