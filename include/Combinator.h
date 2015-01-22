#ifndef PCOMB_COMBINATOR_H
#define PCOMB_COMBINATOR_H

#include "ParserTypes.h"

#include <memory>
#include <tuple>

namespace pcomb
{

// The SeqParser combinator applies multiple parsers (p0, p1, p2, ...) consequtively. If p0 succeeds, it parse the rest of the input string with (p1, p2, ...). If one of the parsers fails, the entire combinator fails. Otherwise, return the result in a tuple
// This is literally the most brain-burning part of this library. You can see that there are LOTS of template metaprogramming going on here and making sure that all of them works is REALLY hard. Even simple tasks like fmap on the type of a tuple require me to craft a recursion somehow with TMP... The good thing is that we have decltype and declval, which save some work.
template <typename ...Parsers>
class SeqParser
{
private:
	std::tuple<Parsers...> parsers;

	template <typename Tuple, size_t I>
	struct AttrTypeImpl
	{
		using type = decltype(
			std::tuple_cat(
				std::declval<typename AttrTypeImpl<Tuple, I-1>::type>(),
				std::make_tuple(std::declval<ParserAttrType<std::tuple_element_t<I-1, Tuple>>>())
			)
		);
	};
	template <typename Tuple>
	struct AttrTypeImpl<Tuple, 1>
	{
		using type = std::tuple<ParserAttrType<std::tuple_element_t<0, Tuple>>>;
	};

	template <typename Tuple, size_t I>
	struct SeqNParserImpl
	{
		static auto parse(const Tuple& t, const StringRef& str)
		{
			using CurrentParserType = decltype(std::get<I-1>(t));
			static_assert(IsParser<CurrentParserType>::value, "SeqParser only accepts parser type");
			using CurrentParserAttrType = ParserAttrType<CurrentParserType>;

			auto prevRes = SeqNParserImpl<Tuple, I-1>::parse(t, str);
			using PrevParserAttrTypes = decltype(prevRes->first);
			using CurrentParserAttrTypes = decltype(std::tuple_cat(std::declval<PrevParserAttrTypes>(), std::make_tuple(std::declval<CurrentParserAttrType>())));

			if (prevRes)
			{
				auto curRes = std::get<I-1>(t).parse(prevRes->second);
				if (curRes)
					return std::experimental::make_optional(std::make_pair(std::tuple_cat(std::move(prevRes->first), std::make_tuple(std::move(curRes->first))), curRes->second));
				else
					return ParseResult<CurrentParserAttrTypes>();
			}
			else
				return ParseResult<CurrentParserAttrTypes>();
		}
	};

	template <typename Tuple>
	struct SeqNParserImpl<Tuple, 1>
	{
		static auto parse(const Tuple& t, const StringRef& str)
		{
			using CurrentParserType = decltype(std::get<0>(t));
			static_assert(IsParser<CurrentParserType>::value, "SeqParser only accepts parser type");
			using CurrentParserAttrType = ParserAttrType<CurrentParserType>;
			using CurrentParserAttrTypes = std::tuple<CurrentParserAttrType>;

			auto res = std::get<0>(t).parse(str);
			if (res)
				return std::experimental::make_optional(std::make_pair(std::make_tuple(std::move(res->first)), res->second));
			else
				return ParseResult<CurrentParserAttrTypes>();
		}
	};
public:
	SeqParser(Parsers... ps): parsers(std::forward_as_tuple(ps...)) {}

	using AttrType = typename AttrTypeImpl<decltype(parsers), sizeof...(Parsers)>::type;

	ParseResult<AttrType> parse(StringRef str) const
	{
		return SeqNParserImpl<std::tuple<Parsers...>, sizeof...(Parsers)>::parse(parsers, str);
	}
};

// The AltParser combinator applies multiple parser (p0, p1, p2, ...) in turn. If p0 succeeds, it returns what p0 returns; otherwise, it tries p1 and return what p1 returns if it succeeds; otherwise, try p2, and so on
// This class still needs lots of template metaprogramming. But it's easier than SeqParser becuase we have a stronger requirement on its template parameters (they must have compatible attribute types)
template <typename ...Parsers>
class AltParser
{
private:
	std::tuple<Parsers...> parsers;

	template <typename Tuple, size_t I>
	struct AttrTypeImpl
	{
	private:
		using PrevType = typename AttrTypeImpl<Tuple, I-1>::type;
		using ElemType = std::tuple_element_t<I-1, Tuple>;
		static_assert(IsParser<ElemType>::value, "AltParser only accepts parser type");
		using CurrType = ParserAttrType<ElemType>;
	public:
		static_assert(MoreGeneralType<PrevType, CurrType>::exist, "AltParser's subparsers must be compatible!");
		using type = typename MoreGeneralType<PrevType, CurrType>::type;
	};
	template <typename Tuple>
	struct AttrTypeImpl<Tuple, 1>
	{
	private:
		using ElemType = std::tuple_element_t<0, Tuple>;
	public:
		static_assert(IsParser<ElemType>::value, "AltParser only accepts parser type");
		using type = ParserAttrType<ElemType>;
	};

	template <typename Tuple, size_t I>
	struct AltNParserImpl
	{
		static auto parse(const Tuple& t, const StringRef& str)
		{
			auto constexpr tupleId = std::tuple_size<Tuple>::value - I;
			auto res = std::get<tupleId>(t).parse(str);
			if (res)
				return std::experimental::make_optional(std::move(*res));
			else
				return AltNParserImpl<Tuple, I-1>::parse(t, str);
		}
	};

	template <typename Tuple>
	struct AltNParserImpl<Tuple, 1>
	{
		static auto parse(const Tuple& t, const StringRef& str)
		{
			auto constexpr tupleId = std::tuple_size<Tuple>::value - 1;
			return std::get<tupleId>(t).parse(str);
		}
	};
public:
	AltParser(Parsers... ps): parsers(std::forward_as_tuple(ps...)) {}

	using AttrType = typename AttrTypeImpl<decltype(parsers), sizeof...(Parsers)>::type;

	ParseResult<AttrType> parse(StringRef str) const
	{
		return AltNParserImpl<std::tuple<Parsers...>, sizeof...(Parsers)>::parse(parsers, str);
	}
};

// The ManyParser combinator applies one parser p0 repeatedly. The result of each application of p0 is pushed into a vector, which is the result of the entire combinator. If nonEmpty is true, the combinator will fail if the vector is empty.
template <typename ParserA>
class ManyParser
{
private:
	ParserA pa;
	bool nonEmpty;
public:
	static_assert(IsParser<ParserA>::value, "ManyParser only accepts parser type");

	using AttrType = std::vector<ParserAttrType<ParserA>>;

	ManyParser(ParserA a, bool moreThanOne = false): pa(std::move(a)), nonEmpty(moreThanOne) {}

	ParseResult<AttrType> parse(StringRef str) const
	{
		auto retVec = AttrType();
		auto resStr = str;

		while (auto paResult = pa.parse(resStr))
		{
			retVec.emplace_back(std::move(paResult->first));
			resStr = paResult->second;
		}

		if (nonEmpty && retVec.empty())
			return ParseResult<AttrType>();
		else
			return std::experimental::make_optional(std::make_pair(std::move(retVec), resStr));
	}
};

// TokenParser takes a parser p and strip the whitespace of the input string before feed it into p
template <typename ParserA>
class TokenParser
{
private:
	ParserA parser;
public:
	static_assert(IsParser<ParserA>::value, "TokenParser only accepts parser type");

	using AttrType = ParserAttrType<ParserA>;

	TokenParser(ParserA p): parser(std::move(p)) {}

	ParseResult<AttrType> parse(StringRef str) const
	{
		return parser.parse(str.lstrip());
	}
};

// The ParserAdapter combinator takes another parser p0, fails if p0 fails, and convert p0's result to T using the function specified by the user if succeeds.
template <typename Converter, typename ParserA>
class ParserAdapter
{
private:
	ParserA parser;
	Converter conv;
public:
	static_assert(IsParser<ParserA>::value, "ParserAdapter only accepts parser type");
	//static_assert(IsCallableWithArgType<Converter, ParserAttrType<ParserA>>::value, "ParserAdapter's convertion function does not match the parser's attribute type");

	using AdapterInputType = ParserAttrType<ParserA>;
	using AdapterOutputType = std::result_of_t<Converter(AdapterInputType)>;

	using AttrType = AdapterOutputType;

	ParserAdapter(ParserA p, Converter c): parser(std::move(p)), conv(std::move(c)) {}

	ParseResult<AttrType> parse(StringRef str) const
	{
		auto pResult = parser.parse(str);
		if (!pResult)
			return ParseResult<AttrType>();

		return std::experimental::make_optional(std::make_pair(conv(std::move(pResult->first)), pResult->second));
	}
};

// LazyParser allows the user to declare a parser first before giving its full definitions. This is useful for recursive grammar definitions.
// It is essentially a pointer to another parser. The challenge here is that it is not possible to tell, at compile time, what the type of that pointer might be because that type could appear AFTER the definition of this LazyParser. Therefore, the only solution I came up with is to use type erasure to allow that pointer points to any parser type.
template <typename A>
class LazyParser
{
private:
	class PhantomParser
	{
	private:
		struct ParserConcept
		{
			virtual ParseResult<A> parse(StringRef str) const = 0;
			virtual ParserConcept* clone() const = 0;
			virtual ~ParserConcept() {}
		};

		template <typename T>
		class ParserModel: public ParserConcept
		{
		private:
			T value;
		public:
			static_assert(IsParser<T>::value, "LazyParser only accepts parser");
			static_assert(std::is_convertible<ParserAttrType<T>, A>::value, "LazyParser parser type mismatch");

			ParserModel(T v): value(std::move(v)) {}
			ParserModel* clone() const override
			{
				return new ParserModel<T>(value);
			}
			ParseResult<A> parse(StringRef str) const
			{
				return value.parse(str);
			}
		};

		std::unique_ptr<ParserConcept> impl;
	public:
		PhantomParser(): impl(nullptr) {}
		template <typename Parser>
		PhantomParser(Parser&& p): impl(std::make_unique<ParserModel<Parser>>(p)) {}
		PhantomParser(const PhantomParser& rhs)
		{
			if (rhs.impl.get() == nullptr)
				impl = nullptr;
			else
				impl = rhs->clone();
		}
		PhantomParser(PhantomParser&& rhs) = default;
		PhantomParser& operator=(const PhantomParser& rhs)
		{
			if (rhs.impl.get() == nullptr)
				impl = nullptr;
			else
				impl = rhs->clone();
		}
		PhantomParser& operator=(PhantomParser&& rhs) = default;

		ParseResult<A> parse(StringRef str) const
		{
			assert(impl != nullptr);
			return impl->parse(str);
		}
	};

	std::shared_ptr<PhantomParser> parser;
public:
	using AttrType = A;

	LazyParser(): parser(std::make_shared<PhantomParser>()) {}

	template <typename Parser>
	LazyParser<A>& setParser(Parser p)
	{
		*parser = std::move(p);
		return *this;
	}

	ParseResult<AttrType> parse(StringRef str) const
	{
		return parser->parse(str);
	}
};

template <typename ...Parsers>
AltParser<Parsers...> alt(Parsers... parsers)
{
	return AltParser<Parsers...>(std::move(parsers)...);
}

template <typename ...Parsers>
SeqParser<Parsers...> seq(Parsers... parsers)
{
	return SeqParser<Parsers...>(std::move(parsers)...);
}

template <typename ParserA>
ManyParser<ParserA> many(ParserA p0, bool moreThanOne = false)
{
	return ManyParser<ParserA>(std::move(p0), moreThanOne);
}

template <typename ParserA>
TokenParser<ParserA> token(ParserA p)
{
	return TokenParser<ParserA>(std::move(p));
}

template <typename Converter, typename ParserA>
ParserAdapter<Converter, ParserA> rule(ParserA p, Converter c)
{
	return ParserAdapter<Converter, ParserA>(std::move(p), std::move(c));
}

}

#endif
