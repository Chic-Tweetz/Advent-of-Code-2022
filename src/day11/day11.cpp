
// #define TEST_INPUT
// #define DEBUG

#include <deque>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "utils.h"


// Initially tried to brute force with unsigned long long
// But the numbers were TOO big and we had to puzzle out some modulo and 
// Lowest common denominator shenanigans
using MonkeyNum_T = long;

// Unfortunately, this is now a huge mess :)

namespace maffs
{
	// gcd and lcm from https://www.geeksforgeeks.org/lcm-of-given-array-elements/
	// Recursively find greatest common denominator
	MonkeyNum_T gcd(MonkeyNum_T a, MonkeyNum_T b)
	{
		if (b == 0)
		{
			return a;
		}
		return gcd(b, a % b);
	}

	MonkeyNum_T lcm(const std::vector<MonkeyNum_T> &divisors)
	{
		MonkeyNum_T lcm{ *divisors.begin() };

		std::for_each(divisors.begin() + 1, divisors.end(),
			[&lcm](MonkeyNum_T val)
			{
				lcm = (val * lcm) / gcd(val, lcm);
			});

		return lcm;
	}

}

namespace Ops
{
	MonkeyNum_T add(MonkeyNum_T old, MonkeyNum_T add)
	{
		return old + add;
	}
	MonkeyNum_T mul(MonkeyNum_T old, MonkeyNum_T mul)
	{
		return old * mul;
	}
	// extra "discard" parameter so we can use the same function template
	MonkeyNum_T sqr(MonkeyNum_T old, MonkeyNum_T discard = 0)
	{
		discard = 0;
		return old * old + discard;
	}
};

class MonkeyFactory;
class Monkey
{
	friend class MonkeyFactory;
public:
	void printInfo()
	{
		std::cout << "Monkey " << m_id << ":\nItems: ";
		for (auto &item : m_items)
		{
			std::cout << item << ' ';
		}
		std::cout << "\nOperation result from 3 with operand " << m_operand << " : " << m_operation(3, m_operand)
			<< "\nOperation result from 7 with operand " << m_operand << " : " << m_operation(7, m_operand)
			<< "\nTest divisor: " << m_testDivisor
			<< "\nTrue monkey: " << m_trueMonkeyId << ", false: " << m_falseMonkeyId << "\n\n";
		
	}
	
	int getInspections() const { return m_inspections; } 

private:
	Monkey(
			int id,
			const std::deque<MonkeyNum_T> &startingItems,
			std::function<MonkeyNum_T(MonkeyNum_T, MonkeyNum_T)> operation,
			MonkeyNum_T operand,
			int divisor,
			int trueMonkeyId,
			int falseMonkeyId
		) :
				
			m_id{ id },
			m_items{ std::move(startingItems) },
			m_operation{ operation },
			m_operand{ operand }, 
			m_testDivisor{ divisor }, 
			m_trueMonkeyId{ trueMonkeyId },
			m_falseMonkeyId{ falseMonkeyId }
	{
	}

	void inspectItem(bool bReduceWorry)
	{
		if (bReduceWorry)
		{
			m_items.front() = m_operation(m_items.front(), m_operand) / 3;
		}
		else
		{
			// We want to keep our item numbers down to avoid overflow and such
			// So can we keep a low number that follows the correct divisibility checks?
			// Common multiple for every monkey?
			// 23, 19, 13, 17
			// multiply starting numbers by those... no
			// add lcm to starting numbers?
			// then add modulo result from test to lcm from there on out ?
			// intuitively i think that might make sense?
			// or do we
			// do the maths operation
			// THEN modulo item % lcm
			// THEN do the divisible check
			m_items.front() = m_operation(m_items.front(), m_operand);
		}

		++m_inspections;

	}

	int testItem() const
	{
		if (m_items.front() % m_testDivisor)
		{
			return m_falseMonkeyId;
		}
		else
		{
			return m_trueMonkeyId;
		}
	}

	MonkeyNum_T throwItem()
	{
		auto item{ m_items.front() };
		m_items.pop_front();
		return item;
	}

	void catchItem(MonkeyNum_T item)
	{
		m_items.push_back(item);
	}

	const int m_id;
	std::deque<MonkeyNum_T> m_items;
	std::function<MonkeyNum_T(MonkeyNum_T, MonkeyNum_T)> m_operation;
	MonkeyNum_T m_operand;
	int m_testDivisor;
	int m_trueMonkeyId;
	int m_falseMonkeyId;
	
	MonkeyNum_T m_inspections{ 0 };

};

// More than a factory, now this handles everything really
class MonkeyFactory
{
public:
	static void reset()
	{
		// m_highestMonkeyId = 0;
		m_monkeys.clear();

	}

	static void findDivisorLcm()
	{
		std::vector<MonkeyNum_T> divisors;
		for (auto &&monkey : m_monkeys)
		{
			divisors.push_back(monkey.m_testDivisor);
		}
		m_divisorsLcm = maffs::lcm(divisors);
	}


		// 	while (monkey.m_items.size())
		// {
		// 	monkey.inspectItem(bReduceWorry);
		// 	size_t throwTo{ static_cast<size_t>( monkey.testItem() ) };
		// 	MonkeyNum_T item { monkey.throwItem() };
		// 	m_monkeys[throwTo].catchItem(item);

	static void takeTurns2()
	{
		findDivisorLcm();
		for (auto &monkey : m_monkeys)
		{
			while (monkey.m_items.size())
			{
				++monkey.m_inspections;
				throwItem(monkey);
			}
		}
	}

	// hey it worked!
	static void throwItem(Monkey &monkey)
	{
		// Keep the values down without invalidating tests
		monkey.m_items.front() = monkey.m_items.front() % m_divisorsLcm;
		// Operation
		monkey.m_items.front() = monkey.m_operation(monkey.m_items.front(), monkey.m_operand);
		// Test
		if (monkey.m_items.front() % monkey.m_testDivisor)
		{
			// False monkey
			m_monkeys[static_cast<size_t>(monkey.m_falseMonkeyId)].m_items.push_back(monkey.m_items.front());
		}
		else
		{
			// True monkey
			m_monkeys[static_cast<size_t>(monkey.m_trueMonkeyId)].m_items.push_back(monkey.m_items.front());
		}

		// Pop
		monkey.m_items.pop_front();
	}


	static void printCurrentMonkeyBusiness()
	{
		std::cout << '\n';
		for (auto &&monkey : m_monkeys)
		{
			std::cout << "Monkey " << monkey.m_id << ": ";
			// for (auto &&item : monkey.m_items)
			// {
			// 	std::cout << item << ' ';
			// }
			std::cout << "inspected items " << monkey.m_inspections << " times";
			std::cout << '\n';
		}
	}
	static void takeTurns(bool bReduceWorry = true)
	{
		for (auto &&monkey : m_monkeys)
		{
			takeTurn(monkey, bReduceWorry);
		}
	}

	static MonkeyNum_T solution()
	{
		MonkeyNum_T highest{ 0 };
		MonkeyNum_T second{ 0 };
		for (auto &&monkey : m_monkeys)
		{	
			if (monkey.m_inspections > second)
			{
				if (monkey.m_inspections >= highest)
				{
					second = highest;
					highest = monkey.m_inspections;
				}
				else
				{
					second = monkey.m_inspections;
				}
			}
		}
		return highest * second;
	}

	static void makeMonkey(const std::vector<std::string>& monkeyInfo)
	{
		std::deque<MonkeyNum_T> startingItems;
		utils::doOnSplit(monkeyInfo[1].data() + 18, ", ",
			[&startingItems](const std::string& splitStr)
			{
				startingItems.push_back(static_cast<MonkeyNum_T>(std::stoi(splitStr)));
			});
		
		MonkeyNum_T (*operation)(MonkeyNum_T, MonkeyNum_T);
		
		MonkeyNum_T operand{ static_cast<MonkeyNum_T>(std::atoi(monkeyInfo[2].data() + 24)) };

		if (monkeyInfo[2][25] == 'o') // Operation: new = old * old
		{
			operation = Ops::sqr;
		}
		else
		{
			operation = monkeyInfo[2][23] == '+' ? Ops::add : Ops::mul;
		}

		// operation = Monkey::add;

		int divisor{ std::atoi(monkeyInfo[3].data() + 21) };

		int trueMonkeyId{ std::atoi(monkeyInfo[4].data() + 29) };
		int falseMonkeyId{ std::atoi(monkeyInfo[5].data() + 30) };

		// m_highestMonkeyId = std::max(m_highestMonkeyId, std::max(trueMonkeyId, falseMonkeyId));

		Monkey monkey{ static_cast<int>(m_monkeys.size()), startingItems, operation, operand, divisor, trueMonkeyId, falseMonkeyId };
		
		m_monkeys.push_back(monkey);

	}
private:
	// inline static int m_highestMonkeyId{ 0 };
	
	inline static std::vector<Monkey> m_monkeys{};
	inline static MonkeyNum_T m_divisorsLcm;
	
	static void takeTurn(Monkey &monkey, bool bReduceWorry)
	{
		while (monkey.m_items.size())
		{
			monkey.inspectItem(bReduceWorry);
			size_t throwTo{ static_cast<size_t>( monkey.testItem() ) };
			MonkeyNum_T item { monkey.throwItem() };
			m_monkeys[throwTo].catchItem(item);
		}
	}

};

namespace Puzzle1
{
	std::string solve(const std::string& infile)
	{
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		while(inf)
		{
			auto monkeyInfo{ utils::getLinesUntil(inf,
				[](const std::string &line)
				{
					return line == "";
				} ) };

			if (monkeyInfo.size() >= 6)
			{
				MonkeyFactory::makeMonkey(monkeyInfo);
			}
		}

		for (int round{ 0 }; round < 20; ++round)
		{
			// std::cout << "Round " << round << ":\n\n";
			MonkeyFactory::takeTurns();
		}

		std::string solution { std::to_string( MonkeyFactory::solution() ) };
		std::cout << "Monkey Business after 20 rounds: " << solution << '\n';
		return solution;
	}
};

namespace Puzzle2
{
	std::string solve(const std::string& infile)
	{
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		while(inf)
		{
			auto monkeyInfo{ utils::getLinesUntil(inf,
				[](const std::string &line)
				{
					return line == "";
				} ) };

			if (monkeyInfo.size() >= 6)
			{
				MonkeyFactory::makeMonkey(monkeyInfo);
			}
		}

		for (int round{ 0 }; round < 10000; ++round)
		{
			MonkeyFactory::takeTurns2();

#ifdef DEBUG
			if (round == 0 || round == 19 || (round + 1) % 1000 == 0)
			{
				std::cout << "\nround " << round + 1 << '\n';
				MonkeyFactory::printCurrentMonkeyBusiness();
			}
#endif
		}

		std::string solution { std::to_string( MonkeyFactory::solution() ) };
		std::cout << "Monkey Business after 10000 rounds: " << solution << '\n';
		return solution;
	}
};

int main()
{
	const std::string input{ utils::getFilePath(__FILE__) };
	
	auto solution1{ Puzzle1::solve(input) }; 
	
	#ifdef TEST_INPUT
	if (solution1 == "10605")
	#else
	if (solution1 == "117640")
	{
		std::cout << "\n***  CORRECT  ***\n\n";
	}
	else
	{
		std::cout << "\n*** INCORRECT! ***\n\n";
	}
	#endif

	MonkeyFactory::reset();

	auto solution2{ Puzzle2::solve(input) };
	#ifdef TEST_INPUT
	if (solution2 == "2713310158")
	#else
	if (solution2 == "30616425600")
	#endif
	{
		std::cout << "\n***  CORRECT  ***\n\n";
	}
	else
	{
		std::cout << "\n*** INCORRECT! ***\n\n";
	}
	
	return 0;
}