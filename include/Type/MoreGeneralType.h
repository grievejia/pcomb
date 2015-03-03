#ifndef PCOMB_MORE_GENERAL_TYPE_H
#define PCOMB_MORE_GENERAL_TYPE_H

#include <type_traits>

namespace pcomb
{

// MoreGeneralType returns P2 if P1 is convertible to P2 and returns P1 if P2 is convertible to P1. 
template <typename P1, typename P2, typename = void>
struct MoreGeneralType { static constexpr bool exist = false; };
template <typename P1>
struct MoreGeneralType<P1, P1>
{
	static constexpr bool exist = true;
	using type = P1;
};
template <typename P1, typename P2>
struct MoreGeneralType <P1, P2, typename std::enable_if<std::is_convertible<P1, P2>::value && !std::is_same<P1, P2>::value>::type>
{
	static constexpr bool exist = true;
	using type = P2;
};
template <typename P1, typename P2>
struct MoreGeneralType <P1, P2, typename std::enable_if<std::is_convertible<P2, P1>::value && !std::is_same<P1, P2>::value>::type>
{
	static constexpr bool exist = true;
	using type = P1;
};

}

#endif
