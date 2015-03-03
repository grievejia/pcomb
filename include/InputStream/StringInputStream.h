#ifndef PCOMB_STRING_STREAM_H
#define PCOMB_STRING_STREAM_H

#include <experimental/string_view>

namespace pcomb
{

class StringInputStream
{
private:
	std::experimental::string_view str;

	StringInputStream(std::experimental::string_view s): str(s) {}
public:
	StringInputStream(const char* s): str(s) {}
	StringInputStream(const std::string& s): str(s) {}

	bool isEOF() const
	{
		return str.empty();
	}

	const char* getRawBuffer() const
	{
		return str.data();
	}

	StringInputStream consume(size_t n) const
	{
		// throw std::out_of_range if n > str.size()
		return StringInputStream(str.substr(n));
	}
};

}

#endif
