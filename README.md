pcomb
======
A lightweight parser combinator library written in C++14.

## Features
* Efficiency is not the goal of this library. After all, parsing is almost never a perfomrance bottleneck of a program analysis system. 
* That being said, I try to avoid using heap allocation as much as possible. Most part of the parser is implemented with template metaprogramming instead of class inheritance (the TMP part can be greatly simplified if a future version of C++ compiler/library adds support for Concept). The only exception to this is the LazyParser class, where statically determine the type of its containing subparser is impossible.
* The parser part of the library is header-only. This is, of course, a direct consequence of using TMP. However, the StringRef part is not header-only. 
* Check examples/calc.cc for a simple calculator example.

## Usage
* Basic parser
```c++
#include "BasicParser.h"

using namespace pcomb;

auto matchAChar = ch('a');
auto matchAString = str("string");
auto matchARangeOfChar = range('a', 'z');
auto matchANumber = regex("[+-]?\\d+");  // the given regex should be in  ECMAScript syntax
auto matchAToken = token(str("token"));  // token() gnore preceding whitespaces before parsing the input 
```

* Combinators
```c++
#include "BasicParser.h"
#include "Combinator.h"

using namespace pcomb;

// Concatenation
auto matchAfollowedbyBfollowedbyC = seq(ch('A'), ch('B'), ch('C'));
// Choice
auto matchAorBorC = alt(ch('A'), ch('B'), ch('C'));
// Repeat
auto matchZeroOrMoreA = many(ch('A'));
auto matchOneOrMoreB = many(ch('B'), true);
```

* Parser attributes
```c++
#include "BasicParser.h"
#include "Combinator.h"

using namespace pcomb;

auto matchNumber = rule(
	token(many(range('0', '9'), true)),
	[] (auto digitVec)
	{
		return std::stoi(std::string(digitVec.begin(), digitVec.end()));
	}
}
);
```

* Recursive grammars
```c++
#include "BasicParser.h"
#include "Combinator.h"

using namespace pcomb;

// In C++, a variable cannot be used to initialize itself.
// This is why when we have a recursive non-terminal we need to declare it first before we fill in the details later
auto parenChar0 = LazyParser<char>();

auto charOrAnotherParen = alt(
	ch([] { return true; }),
	rule(
		seq('(', parenChar0, ')'),
		[] (auto triple) { return std::get<char>(triple); }
	)
);

auto& parenChar = parenChar0.set(charOrAnotherParen);
```

## Compilers support
pcomb relies on the C++14 standard, which means you have to compile it with
  - GCC version >= 4.9
  - Clang version >= 3.4
Also, make sure that the C++ STL you use (libstdc++ on Linux, and libc++ on OS X) is up-to-date with your compiler.
