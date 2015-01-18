#include "StringRef.h"
#include "BasicParser.h"
#include "Combinator.h"
#include "gtest/gtest.h"

#include <iostream>

using namespace pcomb;

TEST(StringRefTest, BasicTest)
{
	std::string str = "abcdefg12345";

	auto sRef0 = StringRef(str);
	EXPECT_FALSE(sRef0.empty());
	EXPECT_EQ(sRef0.size(), str.size());
	EXPECT_EQ(sRef0[0], 'a');
	EXPECT_EQ(sRef0[1], 'b');
	EXPECT_EQ(sRef0[2], 'c');
	EXPECT_EQ(sRef0.front(), 'a');
	EXPECT_EQ(sRef0.back(), '5');

	EXPECT_EQ(sRef0.substr(1, 1).size(), 1u);
	EXPECT_EQ(sRef0.substr(2, 2).size(), 2u);
	EXPECT_EQ(sRef0.dropFront(), "bcdefg12345");
	EXPECT_EQ(sRef0.dropFront(5), "fg12345");
	EXPECT_EQ(sRef0.dropBack(), "abcdefg1234");
	EXPECT_EQ(sRef0.dropBack(5), "abcdefg");
	EXPECT_EQ(sRef0.substr(2, 4), sRef0.slice(2, 6));
	EXPECT_TRUE(sRef0.startswith("a"));
	EXPECT_TRUE(sRef0.endswith("5"));

	EXPECT_EQ(sRef0.find('a'), 0u);
	EXPECT_EQ(sRef0.find('b'), 1u);
	EXPECT_EQ(sRef0.find('1'), 7u);
	EXPECT_EQ(sRef0.find('z'), StringRef::npos);
	EXPECT_EQ(sRef0.find("cde"), 2u);
	EXPECT_EQ(sRef0.find("5"), 11u);
	EXPECT_EQ(sRef0.find("zzzzzzzzzzzzzzz"), StringRef::npos);
	EXPECT_EQ(sRef0.find("g124"), StringRef::npos);

	EXPECT_EQ(sRef0.splitOnce('1').first, "abcdefg");
	EXPECT_EQ(sRef0.splitOnce('1').second, "2345");
	EXPECT_EQ(sRef0.splitOnce('z').first, "abcdefg12345");
	EXPECT_EQ(sRef0.splitOnce('z').second, "");
	EXPECT_EQ(sRef0.splitOnce("g123").first, "abcdef");
	EXPECT_EQ(sRef0.splitOnce("g123").second, "45");
	EXPECT_EQ(sRef0.splitOnce("g124").first, "abcdefg12345");
	EXPECT_EQ(sRef0.splitOnce("g124").second, "");
}

TEST(StringRefTest, OperatorTest)
{
	EXPECT_EQ(StringRef(""), StringRef());
	EXPECT_TRUE(StringRef("aab") < StringRef("aad"));
	EXPECT_FALSE(StringRef("aab") < StringRef("aab"));
	EXPECT_TRUE(StringRef("aab") <= StringRef("aab"));
	EXPECT_FALSE(StringRef("aab") <= StringRef("aaa"));
	EXPECT_TRUE(StringRef("aad") > StringRef("aab"));
	EXPECT_FALSE(StringRef("aab") > StringRef("aab"));
	EXPECT_TRUE(StringRef("aab") >= StringRef("aab"));
	EXPECT_FALSE(StringRef("aaa") >= StringRef("aab"));
	EXPECT_EQ(StringRef("aab"), StringRef("aab"));
	EXPECT_FALSE(StringRef("aab") == StringRef("aac"));
	EXPECT_FALSE(StringRef("aab") != StringRef("aab"));
	EXPECT_TRUE(StringRef("aab") != StringRef("aac"));
	EXPECT_EQ('a', StringRef("aab")[1]);
}

TEST(StringRefTest, SplitTest)
{
	auto expected = std::vector<StringRef>();

	expected.push_back("ab"); expected.push_back("c");
	auto parts = StringRef(",ab,,c,").split(",", -1, false);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	expected.push_back(""); expected.push_back("ab"); expected.push_back("");
	expected.push_back("c"); expected.push_back("");
	parts = StringRef(",ab,,c,").split(",", -1, true);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	expected.push_back("");
	parts = StringRef("").split(",", -1, true);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	parts = StringRef("").split(",", -1, false);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	parts = StringRef(",").split(",", -1, false);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	expected.push_back(""); expected.push_back("");
	parts = StringRef(",").split(",", -1, true);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	expected.push_back("a"); expected.push_back("b");
	parts = StringRef("a,b").split(",", -1, true);
	EXPECT_TRUE(parts == expected);

	// Test MaxSplit
	expected.clear(); parts.clear();
	expected.push_back("a,,b,c");
	parts = StringRef("a,,b,c").split(",", 0, true);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	expected.push_back("a,,b,c");
	parts = StringRef("a,,b,c").split(",", 0, false);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	expected.push_back("a"); expected.push_back(",b,c");
	parts = StringRef("a,,b,c").split(",", 1, true);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	expected.push_back("a"); expected.push_back(",b,c");
	parts = StringRef("a,,b,c").split(",", 1, false);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	expected.push_back("a"); expected.push_back(""); expected.push_back("b,c");
	parts = StringRef("a,,b,c").split(",", 2, true);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	expected.push_back("a"); expected.push_back("b,c");
	parts = StringRef("a,,b,c").split(",", 2, false);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	expected.push_back("a"); expected.push_back(""); expected.push_back("b");
	expected.push_back("c");
	parts = StringRef("a,,b,c").split(",", 3, true);
	EXPECT_TRUE(parts == expected);

	expected.clear(); parts.clear();
	expected.push_back("a"); expected.push_back("b"); expected.push_back("c");
	parts = StringRef("a,,b,c").split(",", 3, false);
	EXPECT_TRUE(parts == expected);
}

template<class Tuple, std::size_t N>
struct TuplePrinter {
    static void print(const Tuple& t) 
    {
        TuplePrinter<Tuple, N-1>::print(t);
        std::cout << ", " << std::get<N-1>(t);
    }
};
 
template<class Tuple>
struct TuplePrinter<Tuple, 1> {
    static void print(const Tuple& t) 
    {
        std::cout << std::get<0>(t);
    }
};
 
template<class... Args>
void print(const std::tuple<Args...>& t) 
{
    std::cout << "(";
    TuplePrinter<decltype(t), sizeof...(Args)>::print(t);
    std::cout << ")\n";
}

TEST(ParserTest, BasicTest)
{
	
}
