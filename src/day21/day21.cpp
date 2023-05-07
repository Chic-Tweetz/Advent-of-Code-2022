// --- Day 21: Monkey Math ---

#include <exception>
#include <filesystem>
#include <forward_list>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "debug.h"	// Enable debug macros with -d flag
#include "log.h"	// Enable with -l flag
#include "utils.h"

using monkeynum_t = long;

monkeynum_t operationResult(monkeynum_t l, monkeynum_t r, char op)
{
	switch (op)
	{
		case '+': return l + r;
		case '-': return l - r;
		case '*': return l * r;
		case '/': return l / r;
		case '=': return l == r; // for root
		default: return 0;
	}
}

class Monkey
{
protected:
	monkeynum_t m_num;
public:
	Monkey(monkeynum_t num) : m_num{ num } {}
	
	virtual ~Monkey() {}
	virtual monkeynum_t get() const
	{
		return m_num;
	}

	// For humn
	void set(monkeynum_t newNum)
	{
		m_num = newNum;
	}
};

class MathsMonkey : public Monkey
{
private:
	const Monkey *m_left;
	const Monkey *m_right;

	char m_operation;

public:
	MathsMonkey(const Monkey *left, const Monkey *right, char operation) :
		Monkey(operationResult(left->get(), right->get(), operation)),
		m_left{ left }, m_right{ right }, m_operation{ operation }
	{
	}

	virtual monkeynum_t get() const override
	{
		return operationResult(m_left->get(), m_right->get(), m_operation);
	}
};

class Solver
{
private:
	monkeynum_t m_equals;

public:
	Solver(monkeynum_t equals = 0) : m_equals{ equals }
	{
	}

	monkeynum_t get() const { return m_equals; } 

	void next(char op, monkeynum_t val, bool bConstOnLeft = false)
	{
		if (bConstOnLeft)
		{
			if (op != '=') DL(m_equals << " = " << val << " " << op << " x");
			switch (op)
			{
				case '-':
				{
					m_equals = val - m_equals;
					break;	
				}
				case '/': // never happens
				{
					m_equals = val / m_equals;
					break;
				}
				case '+': 
				{
					m_equals -= val;
					break;
				}
				case '*': 
				{
					m_equals /= val;
					break;
				}
				case '=':
				{
				m_equals = val;
				}
				default : return;
			}
			return;
		}
		if (op != '=') DL(m_equals << " = x " << op << " " << val);
		switch (op)
		{
			case '+': 
			{
				m_equals -= val;
				break;
			}
			break;
			case '-':
			{
				m_equals += val;
				break;	
			}
			case '*': 
			{
				m_equals /= val;
				break;
			}
			case '/':
			{
				m_equals *= val;
				break;
			}
			case '=':
			{
				m_equals = val;
			}
			default : return;
		}
	}
};

namespace Puzzle1
{
	void solve(const std::string& infile)
	{
		auto inlines{ utils::bufferLines(infile) }; 

		std::unordered_map<std::string, monkeynum_t> monkeyMap;

		// Get all the monkeys which don't depend on other monkeys (just numbers)
		for (size_t i{ 0 }; i < inlines.size(); ++i)
		{
			if (inlines[i][6] <= '9')
			{
				monkeyMap[inlines[i].substr(0, 4)] = std::atoi(inlines[i].data() + 6);
			}
		}

		// Loop through monkeys, resolving maths monkeys until all are done
		bool bAllDone{ true };
		do
		{
			bAllDone = true;
			for (size_t i{ 0 }; i < inlines.size(); ++i)
			{
				std::string id{ inlines[i].substr(0, 4) };

				if (inlines[i][6] >= 'A')
				{
					if (monkeyMap.contains(id)) continue;

					const std::string left{ inlines[i].substr(6, 4) };
					if (monkeyMap.contains(left))
					{
						const std::string right{ inlines[i].substr(13, 4) };
						if (monkeyMap.contains(right))
						{
							char op{ inlines[i][11] };
							monkeyMap[id] = operationResult(monkeyMap[left], monkeyMap[right], op);
							continue;
						}
					}
					bAllDone = false;
				}
			}
		} while (!bAllDone);

		Logger log{};
		for (auto &&nm : monkeyMap)
		{
			DL(nm.first << " = " << nm.second);
			log << nm.first << " = " << nm.second << '\n';
		}

		utils::printAnswer("monkey named root: \"", monkeyMap["root"], "\"");

	}

};

// You need to get the dependencies in order I think
namespace Puzzle2
{
	void solve(const std::string& infile)
	{
		auto inlines{ utils::bufferLines(infile) }; 

		std::unordered_map<std::string, Monkey*> constMonkeys;
		std::unordered_map<std::string, size_t> humnDependentMonkeys{  };
		std::unordered_set<std::string> humnDependentMonkeyIds{}; // For quick id checks
		// Can get rid of the string here, this fetches the whole input line anwyay
		std::list<std::pair<std::string, size_t>> humnDependentIndices{ { "humn", 0 }}; // This gets popped later so the index doesn't matter

		bool bAllDependenciesFound{ true };
		// bool bAllNumericMonkeysResolved{ true };
		do
		{
			bAllDependenciesFound = true;
			// bAllNumericMonkeysResolved = true;
			for (size_t i{ 0 }; i < inlines.size(); ++i)
			{
				std::string id{ inlines[i].substr(0, 4) };

				if (inlines[i][6] >= 'A')
				{
					if (humnDependentMonkeys.contains(id)) continue;

					const std::string left{ inlines[i].substr(6, 4) };
					const std::string right{ inlines[i].substr(13, 4) };
					// if (humnDependentMonkeys.contains(left) || humnDependentMonkeys.contains(right))
					if (humnDependentIndices.back().first == left || humnDependentIndices.back().first == right)
					{
						humnDependentMonkeyIds.insert(id);
						humnDependentMonkeys.insert({ id, i });
						humnDependentIndices.push_back( { id, i } );
						bAllDependenciesFound = false;
					}

				}
				else if (!constMonkeys.contains(id))
				{
					// Pure numeric monkeys
					constMonkeys.insert({ id, new Monkey{ std::atol(inlines[i].data() + 6) } });
				}
			}
		} while (!bAllDependenciesFound);

		bool bMathsMonkeysResolved{ true };

		do
		{
			bMathsMonkeysResolved = true;

			for (size_t i{ 0 }; i < inlines.size(); ++i)
			{
				if (inlines[i][6] >= 'A')
				{
					std::string id{ inlines[i].substr(0, 4) };

					if (constMonkeys.contains(id) || humnDependentMonkeyIds.contains(id))
					{
						continue;
					}
					
					std::string left{ inlines[i].data() + 6, 4 };
					std::string right{ inlines[i].data() + 13, 4 };

					bMathsMonkeysResolved = false;

					if (constMonkeys.contains(left) && constMonkeys.contains(right))
					{
						char op{ inlines[i][11] };
						constMonkeys.insert({ id, new Monkey{ operationResult(constMonkeys.at(left)->get(), constMonkeys.at(right)->get(), op) } });
					}
				}
			}
		} while (!bMathsMonkeysResolved);

		constMonkeys.erase("humn");

		Solver solver;
		for (auto i{ humnDependentIndices.rbegin() }; i != --humnDependentIndices.rend(); ++i )
		{
			auto &line{ inlines[i->second] };
			char op{ i->first == "root" ? '=' : line[11] };

			std::string left{ line.data() + 6, 4 };
			std::string right{ line.data() + 13, 4 };

			bool bLeftConst{ constMonkeys.contains(left) };
			monkeynum_t constMonkeyVal{ bLeftConst ? constMonkeys[left]->get() : constMonkeys[right]->get() };

			solver.next(op, constMonkeyVal, bLeftConst);

		}
		
		auto answer{ solver.get() };

		utils::printAnswer("humn must yell ", answer);
		
		for (auto &&monkey : constMonkeys)
		{
			delete monkey.second;
		}
	}
};

int main(int argc, char* argv[])
{
	flags::set(argc, argv);

	const std::string input{ utils::inputFile(__FILE__) };

	try
	{
		if (utils::doP1()) Puzzle1::solve(input); 
		if (utils::doP2()) Puzzle2::solve(input);
	}
	catch(const std::exception& e)
	{
		std::cerr << "exception: " << e.what() << '\n';
	}

	return 0;

}
