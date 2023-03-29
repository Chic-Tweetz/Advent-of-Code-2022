// At some point I started causing seg faults

// #define DEBUG
// #define TESTINPUT

bool bPrint{ false };

#ifdef DEBUG
#  define DPRINT(x) if (bPrint) { std::cout << x; }
#  define DERR(x) std::cerr << x
#else
#  define DPRINT(x)
#  define DERR(x)
#endif

#include <algorithm>
#include <charconv>
#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#include "utils.h"

// Find the next comma and return a string starting from the following char
std::string_view next(std::string_view in)
{
	if (in[0] == '[')
	{
		return std::string_view{ in.data() + 1, in.length() - 1 };
	}

	if (auto newStart{ in.find(',') }; newStart != std::string::npos)
	{
		++newStart;
		auto newSv{ std::string_view{ in.data() + newStart, in.length() - newStart } };

		return newSv;
	}

	return std::string_view(in.data(), 0 );
}

std::strong_ordering compareUnlike(std::string_view list, std::string_view num)
{
	// Move through the string to find a number rather than a list
	const auto listNumInd{ list.find_first_not_of('[') };

	// List was empty ( ie "[]" )
	if (list[listNumInd] == ']')
	{
		return std::strong_ordering::less;
	}

	if (auto comp{ std::atoi(list.data() + listNumInd) <=> std::atoi(num.data()) }; comp != 0)
	{
		return comp;
	}

	// List has more items
	if (list.find(',') != std::string::npos)
	{
		return std::strong_ordering::greater;
	}

	// List has no more items
	return std::strong_ordering::less; 
}

std::strong_ordering compareItems(std::string_view left, std::string_view right)
{
	// List vs number
	if (left[0] == '[' && right[0] != '[')
	{
		return compareUnlike(left, right);
	}
	
	// Number vs list
	if (left[0] != '[' && right[0] == '[')
	{
		// Flipped for right vs left comparisons
		return 0 <=> compareUnlike(right, left);
	}

	// Number vs number
	return std::atoi(left.data()) <=> std::atoi(right.data());
}

// I'm sure these ifs and elses can be done better
std::strong_ordering compareLists(std::string_view left, std::string_view right)
{
	// I tried to move this out but you actually do want to return the == 0 here
	// Run out of items checks - string end:
	if (left.length() <= 1 || right.length() <= 1)
	{
		return left.length() <=> right.length();
	}
	// Only left list empty
	if (left[0] == ']' && right[0] != ']')
	{
		return std::strong_ordering::less;
	}
	
	// Only right list empty
	if (left[0] != ']' && right[0] == ']')
	{
		return std::strong_ordering::greater;
	}

	// Actual comparisons happen here
	if (const auto comp{ compareItems(left, right) }; comp != 0)
	{
		return comp;
	}

	// If all comparisons have returned 0, move to the next item in the list
	return compareLists(next(left), next(right));
}

namespace Puzzle1
{
	void solve(const std::string& infile)
	{
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		int solution{ 0 };
		int count = 0;
		while (inf)
		{
			std::string left;
			std::string right;
			std::string discard;

			std::getline(inf, left);
			std::getline(inf, right);
			std::getline(inf, discard);

			++count;
			
			DPRINT("== Pair " << count << " ==\n");

			auto comp{ compareLists(left, right) };

			if (comp < 0)
			{
				solution += count;
			}

			DPRINT("\n\n");
		}

		std::cout << "Sum of indices of correctly ordered lists: " << solution << '\n'; 

#ifdef TESTINPUT
		if (solution == 13)
#else
		if (solution == 5393)
#endif
		{
			std::cout << "CORRECT\n";
		}
		else
		{
			std::cout << "INCORRECT\n";
		}
	}
};

namespace Puzzle2
{
	void solve(const std::string& infile)
	{
		std::ifstream inf{ infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		auto lines{ utils::bufferLines(infile) };

		lines.push_back("[[2]]"); // Divider packets
		lines.push_back("[[6]]");

		std::sort(lines.begin(), lines.end(),
		[](std::string_view left, std::string_view right)
		{
			return compareLists(left, right) < 0;
		});

		auto divpacket1{ std::find(lines.begin(), lines.end(), "[[2]]") };
		auto divpacket2{ std::find(lines.begin(), lines.end(), "[[6]]") };

		auto divp1index{ divpacket1 - lines.begin() + 1 };
		auto divp2index{ divpacket2 - lines.begin() + 1 };

		auto solution { divp1index * divp2index };

		std::cout << "Product of indices of sorted divider packets: " << solution << '\n';

#ifdef TESTINPUT
		if (solution == 140)
#else
		if (solution == 26712)
#endif
		{
			std::cout << "CORRECT\n";
		}
		else
		{
			std::cout << "INCORRECT\n";
		}
	}
};

int main()
{
#ifdef TESTINPUT
	const std::string input{ utils::getFilePath(__FILE__, "test") };
#else
	const std::string input{ utils::getFilePath(__FILE__) };
#endif

	Puzzle1::solve(input);
	Puzzle2::solve(input);

	return 0;
}
