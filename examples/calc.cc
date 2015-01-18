#include "BasicParser.h"
#include "Combinator.h"

#include <iostream>
#include <stdexcept>

// This files shows how to use pcomb to write a simple calculator

using namespace pcomb;

// Grammar of the calculator
auto digit = range('0', '9');
auto unum = rule
(
	token(many(digit, true)),
	[] (auto const& digits) -> long
	{
		auto numStr = std::string(digits.begin(), digits.end());
		return std::stoul(numStr);
	}
);
auto nnum = rule
(
	token(seq(ch('-'), unum)),
	[] (auto const& pair)
	{
		long n= std::get<long>(pair);
		return -n;
	}
);
auto inum = alt(unum, nnum);

class Expr
{
public:
	virtual long eval() const = 0;
	virtual bool isNum() const = 0;
	virtual void print() const = 0;
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
	bool isNum() const override { return true; }
	void print() const override { std::cout << "(" << num << ")"; }
};

class BinExpr: public Expr
{
private:
	char opCode;
	ExprPtr lhs, rhs;
public:
	BinExpr(char o, ExprPtr r): opCode(o), rhs(std::move(r)) {}
	void setLhs(ExprPtr l) { lhs = std::move(l); }
	bool isLhsValid() const { return lhs != nullptr; }
	bool isFactor() const { return opCode == '*' || opCode == '/'; }
	ExprPtr moveLhs() { return std::move(lhs); }
	Expr* getLhs() { return lhs.get(); }

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
	bool isNum() const override { return false; }
	void print() const override
	{
		//assert(lhs && rhs);
		std::cout << "(" << opCode << " ";
		if (lhs)
			lhs->print();
		else
			std::cout << "#";
		std::cout << " ";
		if (rhs)
			rhs->print();
		else
			std::cout << "#";
		std::cout << ")";
	}
};

using OptExpr = std::experimental::optional<ExprPtr>;
auto factor0 = LazyParser<OptExpr>();

auto factor = rule
(
	seq(inum, factor0),
	[] (auto pair) -> ExprPtr
	{
		long i = 0;
		OptExpr optExpr;
		std::tie(i, optExpr) = std::move(pair);

		auto numExpr = std::make_unique<NumExpr>(i);
		if (optExpr)
		{
			auto binExpr = static_cast<BinExpr*>((*optExpr).get());
			while (binExpr->isLhsValid())
			{
				assert(!binExpr->getLhs()->isNum());
				binExpr = static_cast<BinExpr*>(binExpr->getLhs());
			}
			binExpr->setLhs(std::move(numExpr));
			return std::move(*optExpr);
		}
		else
			return std::move(numExpr);
	}
);

auto& factor1 = factor0.setParser(
	alt
	(
		rule(
			seq(alt(token(ch('*')), token(ch('/'))), factor),
			[] (auto pair) -> OptExpr
			{
				char ch;
				ExprPtr expr = nullptr;
				std::tie(ch, expr) = std::move(pair);

				if (!expr->isNum())
				{
					auto binExpr = static_cast<BinExpr*>(expr.get());
					auto tmpExpr = binExpr->moveLhs();
					binExpr->setLhs(std::make_unique<BinExpr>(ch, std::move(tmpExpr)));
					return std::experimental::make_optional(std::move(expr));
				}
				else
				{
					std::unique_ptr<Expr> retExpr = std::make_unique<BinExpr>(ch, std::move(expr));
					return std::experimental::make_optional(std::move(retExpr));
				}
			}
		),
		rule(str(""), [] (auto) -> OptExpr { return OptExpr(); })
	)
);

auto term0 = LazyParser<OptExpr>();

auto term = rule
(
	seq(factor, term0),
	[] (auto pair) -> ExprPtr
	{
		ExprPtr leftExpr = nullptr;
		OptExpr optExpr;
		std::tie(leftExpr, optExpr) = std::move(pair);

		if (optExpr)
		{
			auto binExpr = static_cast<BinExpr*>((*optExpr).get());
			while (binExpr->isLhsValid())
			{
				assert(!binExpr->getLhs()->isNum());
				binExpr = static_cast<BinExpr*>(binExpr->getLhs());
			}
			binExpr->setLhs(std::move(leftExpr));
			return std::move(*optExpr);
		}
		else
			return std::move(leftExpr);
	}
);

auto& term1 = term0.setParser(
	alt
	(
		rule(
			seq(alt(token(ch('+')), token(ch('-'))), term),
			[] (auto pair)
			{
				char ch;
				ExprPtr expr = nullptr;
				std::tie(ch, expr) = std::move(pair);

				if (!expr->isNum())
				{
					auto binExpr = static_cast<BinExpr*>(expr.get());
					if (!binExpr->isFactor())
					{
						auto tmpExpr = binExpr->moveLhs();
						binExpr->setLhs(std::make_unique<BinExpr>(ch, std::move(tmpExpr)));
						return std::experimental::make_optional(std::move(expr));
					}
				}
				
				std::unique_ptr<Expr> retExpr = std::make_unique<BinExpr>(ch, std::move(expr));
				return std::experimental::make_optional(std::move(retExpr));
			}
		),
		rule(str(""), [] (auto) { return OptExpr(); })
	)
);

void parseLine(StringRef line)
{
	auto parseResult = term.parse(line);
	if (!parseResult)
	{
		std::cout << "Parsing failed\n";
		return;
	}

	if (!parseResult->second.rstrip().empty())
	{
		std::cout << "Excessive input: " << parseResult->second.str() << "\n";
		return;
	}

	std::cout << "Result = " << parseResult->first->eval() << "\n";
}

int main()
{
	std::cout << "Simple calculator powered by pcomb\n";
	while (true)
	{
		std::cout << "> ";

		auto line = std::string();
		std::getline(std::cin, line);

		if (line.empty())
			break;

		parseLine(line);
	}
	std::cout << "Bye bye!" << std::endl;
}
