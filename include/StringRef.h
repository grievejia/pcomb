#ifndef PCOMB_STRING_REF_H
#define PCOMB_STRING_REF_H

#include <cstring>
#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

namespace pcomb
{

// StringRef is a constant reference to a character array and a length, which need not be null terminated.
// This class does not own the string data, it is expected to be used in situations where the character data resides in some other buffer, whose lifetime extends past that of the StringRef.
class StringRef
{
private:
	const char* data;
	size_t len;

	StringRef(const char* str, size_t n): data(str), len(n)
	{
		assert(str != nullptr || (str == nullptr && n == 0) && "StringRef(NULL, n) where n != 0 is not allowed");
	}

	static int compareMemory(const char *lhs, const char *rhs, size_t n)
	{
		if (n == 0)
			return 0;
		else
			return std::memcmp(lhs, rhs, n);
    }

    bool equals(const StringRef& rhs) const
    {
		return (len == rhs.len && compareMemory(data, rhs.data, len) == 0);
    }

    int compare(const StringRef& rhs) const
    {
		// Check the prefix for a mismatch.
		if (auto res = compareMemory(data, rhs.data, std::min(len, rhs.len)))
		return res < 0 ? -1 : 1;

		// Otherwise the prefixes match, so we only need to check the lengths.
		if (len == rhs.len)
			return 0;
		else
			return len < rhs.len ? -1 : 1;
	}

	size_t findFirstNotOf(const StringRef& chars, size_t from = 0) const;
	size_t findLastNotOf(const StringRef& chars, size_t from = 0) const;
public:
	using iterator = const char*;
	using const_iterator = const char*;
	using size_type = size_t;

	static const size_t npos;

	StringRef(): data(nullptr), len(0) {}
	StringRef(const char* str): data(str)
	{
		assert(str != nullptr && "StringRef(NULL) is not allowed");
		len = std::strlen(str);
	}
	StringRef(const std::string& str): data(str.data()), len(str.length()) {}

	iterator begin() const { return data; }
	iterator end() const { return data + len; }

	const char *raw_pointer() const { return data; }
	std::string str() const
	{
		if (data == nullptr)
			return std::string();
		else
			return std::string(data, len);
    }
	bool empty() const { return len == 0; }
	size_t size() const { return len; }
	size_t length() const { return size(); }

	operator std::string() const { return str(); }
	char operator[](size_t idx) const
	{
		assert(idx < len && "Invalid index!");
		return data[idx];
	}
	char at(size_t idx) const
	{
		if (idx >= len)
			throw std::out_of_range("StringRef::at() out of range");
		return data[idx];
	}
	char front() const
	{
		assert(!empty());
		return data[0];
	}
 
	char back() const
	{
		assert(!empty());
		return data[len-1];
	}

	StringRef substr(size_t start, size_t n = npos) const
	{
		start = std::min(start, len);
		return StringRef(data + start, std::min(n, len - start));
	}
	StringRef dropFront(size_t n = 1) const
	{
		assert(len >= n && "Dropping more elements than exist");
		return substr(n);
	}
	StringRef dropBack(size_t n = 1) const
	{
		assert(len >= n && "Dropping more elements than exist");
		return substr(0, len - n);
	}
	StringRef slice(size_t start, size_t end) const
	{
		start = std::min(start, len);
		end = std::min(std::max(start, end), len);
		return StringRef(data + start, end - start);
	}

	bool startswith(const StringRef& prefix) const
	{
		return len >= prefix.len && compareMemory(data, prefix.data, prefix.len) == 0;
	}
	bool endswith(const StringRef& suffix) const
	{
		return len >= suffix.len && compareMemory(end() - suffix.len, suffix.data, suffix.len) == 0;
	}
	StringRef lstrip(StringRef chars = " \t\n\v\f\r") const
	{
		return dropFront(std::min(len, findFirstNotOf(chars)));
	}
	StringRef rstrip(StringRef chars = " \t\n\v\f\r") const
	{
		return dropBack(len - std::min(len, findLastNotOf(chars) + 1));
	}
	StringRef strip(StringRef chars = " \t\n\v\f\r") const
	{
		return lstrip(chars).rstrip(chars);
	}

	size_t find(char c, size_t from = 0) const
	{
		for (size_t i = std::min(from, len), e = len; i != e; ++i)
			if (data[i] == c)
				return i;
		return npos;
	}
	size_t find(const StringRef& str, size_t from = 0) const;

	std::pair<StringRef, StringRef> splitOnce(char sep) const
	{
		auto idx = find(sep);
		if (idx == npos)
			return std::make_pair(*this, StringRef());
		else
			return std::make_pair(slice(0, idx), slice(idx+1, npos));
	}
	std::pair<StringRef, StringRef> splitOnce(StringRef sep) const
	{
		auto idx = find(sep);
		if (idx == npos)
			return std::make_pair(*this, StringRef());
		return std::make_pair(slice(0, idx), slice(idx + sep.size(), npos));
	}
	std::vector<StringRef> split(StringRef sep, int maxSplit = -1, bool keepEmpty = true) const;

	bool operator==(const StringRef& rhs) const
	{
		return equals(rhs);
	}
	bool operator!=(const StringRef& rhs) const
	{
		return !(*this == rhs);
	}
	bool operator<(const StringRef& rhs) const
	{
		return compare(rhs) == -1;
	}
	bool operator>(const StringRef& rhs) const
	{
		return compare(rhs) == 1;
	}
	bool operator<=(const StringRef& rhs) const
	{
		return !(*this > rhs);
	}
	bool operator>=(const StringRef& rhs) const
	{
		return !(*this < rhs);
	}
	
};

}

#endif
