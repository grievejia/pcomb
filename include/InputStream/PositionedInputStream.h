#ifndef PCOMB_POSITIONED_INPUT_STREAM_H
#define PCOMB_POSITIONED_INPUT_STREAM_H

#include <experimental/string_view>

namespace pcomb
{

class PositionedInputStream
{
private:
	struct Position
	{
		size_t row, col;
	};

	std::experimental::string_view str;
	Position pos;

	PositionedInputStream(const std::experimental::string_view& s, const Position& p): str(s), pos(p) {}
public:
	PositionedInputStream(const char* s): str(s), pos({1, 1}) {}
	PositionedInputStream(const std::string& s): str(s), pos({1, 1}) {}

	bool isEOF() const
	{
		return str.empty();
	}

	const char* getRawBuffer() const
	{
		return str.data();
	}

	PositionedInputStream consume(size_t n) const
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
		return PositionedInputStream(str.substr(n), newPos);
	}

	size_t getLineNumber() const { return pos.row; }
	size_t getColumnNumber() const { return pos.col; }
};

}

#endif
