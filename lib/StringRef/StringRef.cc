#include "StringRef.h"

#include <bitset>

using namespace pcomb;

const size_t StringRef::npos = ~size_t(0);

size_t StringRef::find(const StringRef& str, size_t from) const
{
	size_t n = str.size();
	if (n > len)
	return npos;

	// For short haystacks or unsupported needles fall back to the naive algorithm
	if (len < 16 || n > 255 || n == 0)
	{
		for (size_t e = len - n + 1, i = std::min(from, e); i != e; ++i)
			if (substr(i, n).equals(str))
				return i;
		return npos;
	}
 
	if (from >= len)
		return npos;
 
	// Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
	uint8_t badCharSkip[256];
	std::memset(badCharSkip, n, 256);
	for (unsigned i = 0; i != n-1; ++i)
		badCharSkip[(uint8_t)str[i]] = n-1-i;
 
	unsigned curLen = len - from, pos = from;
	while (curLen >= n)
	{
		if (substr(pos, n).equals(str)) // See if this is the correct substring.
			return pos;

		// Otherwise skip the appropriate number of bytes.
		uint8_t skip = badCharSkip[(uint8_t)(*this)[pos+n-1]];
		curLen -= skip;
		pos += skip;
	}
 
	return npos;
}

size_t StringRef::findFirstNotOf(const StringRef& chars, size_t from) const
{
	std::bitset<1 << 8u> charBits;
	for (size_t i = 0, e = chars.size(); i < e; ++i)
		charBits.set((unsigned char)chars[i]);

	for (auto i = std::min(from, len), e = len; i != e; ++i)
		if (!charBits.test((unsigned char)data[i]))
			return i;
	return npos;
}

size_t StringRef::findLastNotOf(const StringRef& chars, size_t from) const
{
	std::bitset<1 << 8u> charBits;
	for (size_t i = 0, e = chars.size(); i < e; ++i)
		charBits.set((unsigned char)chars[i]);

	for (size_t i = std::min(from, len) - 1, e = -1; i != e; --i)
		if (!charBits.test((unsigned char)data[i]))
			return i;
	return npos;
}

std::vector<StringRef> StringRef::split(StringRef sep, int maxSplit, bool keepEmpty) const
{
	auto retVec = std::vector<StringRef>();

	auto rest = *this;
	// rest.data() is used to distinguish cases like "a," that splits into
	// "a" + "" and "a" that splits into "a" + 0.
	for (auto splits = 0; rest.data != nullptr && (maxSplit < 0 || splits < maxSplit); ++splits)
	{
		auto p = rest.splitOnce(sep);
		if (keepEmpty || p.first.size() != 0)
			retVec.push_back(p.first);
		rest = p.second;
	}
	// If we have a tail left, add it.
	if (rest.data != nullptr && (rest.size() != 0 || keepEmpty))
		retVec.push_back(rest);

	return retVec;
}
