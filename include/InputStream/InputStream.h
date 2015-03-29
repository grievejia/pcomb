#ifndef PCOMB_INPUT_STREAM_H
#define PCOMB_INPUT_STREAM_H

#include <cassert>
#include <experimental/string_view>

namespace pcomb
{

class InputStream
{
private:
	struct Position
	{
		size_t row, col;
	};

	std::experimental::string_view str;
	Position pos;

	InputStream(const std::experimental::string_view& s, const Position& p): str(s), pos(p) {}
public:
	InputStream(std::experimental::string_view s): str(s), pos({1, 1}) {}

	bool isEOF() const
	{
		return str.empty();
	}

	const char* getRawBuffer() const
	{
		return str.data();
	}

	InputStream consume(size_t n) const
	{
		assert(n <= str.size());
		auto newPos = pos;
		for (auto i = 0u; i < n; ++i)
		{
			auto ch = str[i];
			if (ch == '\n')
			{
				++newPos.row;
				newPos.col = 1;
			}
			else
				++newPos.col;
		}
		return InputStream(str.substr(n), newPos);
	}

	size_t getLineNumber() const { return pos.row; }
	size_t getColumnNumber() const { return pos.col; }
};

}

#endif
