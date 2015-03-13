#ifndef PCOMB_PARSE_RESULT_H
#define PCOMB_PARSE_RESULT_H

#include <cassert>
#include <experimental/optional>

namespace pcomb
{

template <typename InputType, typename AttrType>
class ParseResult
{
private:
	std::experimental::optional<AttrType> attr;
	InputType input;
public:
	template <typename I>
	ParseResult(I&& i): input(std::forward<I>(i)) {}
	template <typename I, typename A>
	ParseResult(I&& i, A&& a): input(std::forward<I>(i)), attr(std::forward<A>(a)) {}

	template <typename I, typename A>
	void setResult(I&& i, A&& a)
	{
		input = std::forward<I>(i);
		attr = std::forward<A>(a);
	}

	bool success() const { return static_cast<bool>(attr); }
	bool hasError() const { return !success(); }

	const AttrType& getAttribute() const
	{
		assert(success());
		return (*attr);
	}
	AttrType&& moveAttribute()
	{
		assert(success());
		return std::move(*attr);
	}

	const InputType& getInputStream() const
	{
		return input;
	}
	InputType&& moveInputStream()
	{
		return std::move(input);
	}
};

}

#endif
