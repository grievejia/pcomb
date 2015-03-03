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
	std::experimental::optional<std::pair<AttrType, InputType>> result;
public:
	ParseResult() = default;
	ParseResult(const InputType& i, const AttrType& a): result(std::make_pair(i, a)) {}
	ParseResult(InputType&& i, AttrType&& a): result(std::make_pair(std::move(i), std::move(a))) {}

	void setResult(const InputType& i, const AttrType& a)
	{
		result = std::make_pair(a, i);
	}
	void setResult(InputType&& i, AttrType&& a)
	{
		result = std::make_pair(std::move(a), std::move(i));
	}

	bool success() const { return static_cast<bool>(result); }
	bool hasError() const { return !success(); }

	const AttrType& getAttribute() const
	{
		assert(success());
		return (*result).first;
	}
	AttrType&& moveAttribute()
	{
		assert(success());
		return std::move((*result).first);
	}

	const InputType& getInputStream() const
	{
		assert(success());
		return (*result).second;
	}
	InputType&& moveInputStream()
	{
		assert(success());
		return std::move((*result).second);
	}
};

}

#endif
