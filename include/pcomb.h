#ifndef PCOMB_MAIN_HEADER_H
#define PCOMB_MAIN_HEADER_H

// This is a header that pulls in all the headers for parsers and combinators
#include "Parser/PredicateCharParser.h"
#include "Parser/RegexParser.h"
#include "Parser/StringParser.h"

#include "Combinator/AltParser.h"
#include "Combinator/SeqParser.h"
#include "Combinator/ManyParser.h"
#include "Combinator/TokenParser.h"
#include "Combinator/ParserAdapter.h"
#include "Combinator/LazyParser.h"
#include "Combinator/LexemeParser.h"

#endif
