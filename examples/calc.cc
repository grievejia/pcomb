#include "pcomb.h"

#include <iostream>
#include <stdexcept>

// This files shows how to use pcomb to write a simple calculator

using namespace pcomb;

// AST definitions
class Expr
{
public:
	virtual long eval() const = 0;
	virtual ~Expr() = default;
};
using ExprPtr = std::unique_ptr<Expr>;

class NumExpr: public Expr
{
private:
	long num;
public:
	NumExpr(long n): num(n) {}

	long eval() const override { return num; }
};

class BinExpr: public Expr
{
private:
	char opCode;
	ExprPtr lhs, rhs;
public:
	BinExpr(char o, ExprPtr l, ExprPtr r): opCode(o), lhs(std::move(l)), rhs(std::move(r)) {}

	long eval() const override
	{
		assert(lhs && rhs);
		switch (opCode)
		{
			case '+':
				return lhs->eval() + rhs->eval();
			case '-':
				return lhs->eval() - rhs->eval();
			case '*':
				return lhs->eval() * rhs->eval();
			case '/':
				return lhs->eval() / rhs->eval();
			default:
				throw std::runtime_error("Invalid opcode");
		}
	}
};

// Grammar of the calculator
auto inum = rule
(
	token(regex("[+-]?\\d+")),
	[] (auto digits) -> long
	{
		return std::stoul(digits.to_string());
	}
);

auto expr0 = LazyParser<ExprPtr>();

auto nexpr = alt
(
	rule(inum, [] (auto n) -> ExprPtr { return std::make_unique<NumExpr>(n); }),
	rule
	(
		seq(token(ch('(')), expr0, token(ch(')'))),
		[] (auto triple)
		{
			return std::move(std::get<1>(triple));
		}
	)
);

auto factor = rule
(
	seq(nexpr, many(seq(alt(token(ch('*')), token(ch('/'))), nexpr))),
	[] (auto pair) -> ExprPtr
	{
		ExprPtr retExpr = std::move(std::get<0>(pair));
		auto& exprVec = std::get<1>(pair);

		for (auto& pair: exprVec)
			retExpr = std::make_unique<BinExpr>(std::get<0>(pair), std::move(retExpr), std::move(std::get<1>(pair)));

		return std::move(retExpr);
	}
);

auto term = rule
(
	seq(factor, many(seq(alt(token(ch('+')), token(ch('-'))), factor))),
	[] (auto pair) -> ExprPtr
	{
		ExprPtr retExpr = std::move(std::get<0>(pair));
		auto& factorVec = std::get<1>(pair);

		for (auto& pair: factorVec)
			retExpr = std::make_unique<BinExpr>(std::get<0>(pair), std::move(retExpr), std::move(std::get<1>(pair)));

		return std::move(retExpr);
	}
);

auto& expr = expr0.setParser(term);

auto parser = line(expr);

void parseLine(const std::string& lineStr)
{
	InputStream ss(lineStr);
	auto parseResult = parser.parse(ss);
	if (parseResult.hasError())
	{
		auto remainingStream = parseResult.getInputStream();
		std::cout << "Parsing failed at line " << remainingStream.getLineNumber() << ", column " << remainingStream.getColumnNumber() << "\n";
		return;
	}

	std::cout << "Result = " << parseResult.getOutput()->eval() << "\n";
}

int main()
{
	std::cout << "Simple calculator powered by pcomb\n";
	while (true)
	{
		std::cout << "> ";

		auto lineStr = std::string();
		std::getline(std::cin, lineStr);

		if (lineStr.empty())
			break;

		// getline() won't keep the newline character, which is annoying
		lineStr.push_back('\n');
		parseLine(lineStr);
	}
	std::cout << "Bye bye!" << std::endl;
}
