// Copy and paste for each day for a quick start

// #define DEBUG
// #define TESTINPUT

#ifdef DEBUG
#  define DPRINT(x) std::cout << x
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

std::string_view dig(std::string_view left/*, const int depth = 0*/)
{
	if (left[0] != '[')
	{
		return left;
	}

	size_t start{ 1 };
	size_t length{ 0 };
	int brackets{ 1 };
	size_t closeBracket{ 1 };

	while (closeBracket < left.length())
	// for (size_t i{ 0 }; i < left.length(); ++i)
	{
		if (left[closeBracket] == '[')
		{
			++brackets;
		}
		else if (left[closeBracket] == ']' && --brackets == 0)
		{
			length = closeBracket - start;
			break;
		}
		++closeBracket;
	}

	return dig( { left.data() + start, length } );
}

std::vector<std::string_view> split(std::string_view str)
{	
	std::vector<std::string_view> list;

	int brackets{ 0 };
	size_t start{ 0 };

	for (size_t i{ 0 }; i < str.length(); ++i)
	{
		if (str[i] == '[')
		{
			if (++brackets == 1)
			{
				start = i;
			}
		}

		else if (str[i] == ']' && !(--brackets))
		{	
			list.push_back( std::string_view {
				str.data() + start, i - start + 1
			});

			if (str[i + 1] == ',')
			{
				start = i + 2;
				++i;
			}

			start = i + 1;
		}
		else if (!brackets && str[i] == ',')
		{
			list.push_back( std::string_view {
				str.data() + start, i - start
			});

			start = i + 1;
		}
		else if (i == str.length() - 1)
		{	
			list.push_back( std::string_view {
				str.data() + start, str.length() - start
			});
		}
	}

	return list;
} 

int compare(int left, int right)
{
	if (left < right)
	{
		return 1;
	}
	if (left > right)
	{
		return -1;
	}
	return 0;
}

int compare(std::vector<std::string_view> left, std::vector<std::string_view> right, const int depth = 0)
{
	for (size_t i{ 0 }; i < std::max(left.size(), right.size()); ++i)
	{
		for (int j{ 0 }; j < depth; ++j)
		{
			DPRINT(" ");
		}
		if (i >= left.size())
		{
			DPRINT("- Left side ran out of items, so inputs are in the right order\n");
			return 1;
		}
		if (i >= right.size())
		{
			DPRINT("- Right side ran out of items, so inputs are not in the right order\n");
			return -1;
		}
		DPRINT("- Compare " << left[i] << " vs " << right[i] << '\n');
		// l & l
		if (left[i][0] == '[' && right[i][0] == '[')
		{
			auto leftSplit{ split(std::string_view{ left[i].data() + 1, left[i].length() - 2 }) };
			auto rightSplit{ split(std::string_view{ right[i].data() + 1, right[i].length() - 2 }) };
			
			auto innerComp{ compare(leftSplit, rightSplit, depth + 1) };
			if (innerComp != 0) 
			{
				return innerComp;
			}
		}
		// i & i
		else if (!(left[i][0] == '[') && !(right[i][0] == '['))
		{
			if (std::atoi(left[i].data()) < std::atoi(right[i].data()))
			{
				for (int j { 0 }; j <= depth; ++j)
				{
					DPRINT("  ");
				}
				DPRINT("- Left side is smaller, so inputs are in the RIGHT order");
				return 1;
			}
			if (std::atoi(left[i].data()) > std::atoi(right[i].data()))
			{
				for (int i { 0 }; i <= depth; ++i)
				{
					DPRINT("  ");
				}
				DPRINT("- Right side is smaller, so inputs are in the WRONG order");
				return -1;
			}
			continue;
		}
		// l & i
		else if (left[i][0] == '[')
		{
			for (int j { 0 }; j <= depth; ++j)
				{
					DPRINT("  ");
				}
			DPRINT("- Mixed types; convert right to [" << right[i] << "] and retry comparison\n");
			
			std::string convert{ "[" + std::string{ right[i] } + "]" };
			std::vector<std::string_view> rightcopy{ right.begin() + static_cast<int>(i), right.end() };
			rightcopy[0] = convert;

			std::vector<std::string_view> leftcopy{ left.begin() + static_cast<int>(i), left.end() };
			
			auto innercomp{ compare(leftcopy, rightcopy, depth + 1) };
			if (innercomp != 0)
			{
				return innercomp;
			}
			continue;
		}
		// l & i
		else if (right[i][0] == '[')
		{
			for (int j { 0 }; j <= depth; ++j)
				{
					DPRINT("  ");
				}
			DPRINT("- Mixed types; convert left to [" << left[i] << "] and retry comparison\n");
			
			std::string convert{ "[" + std::string{ left[i] } + "]" };

			std::vector<std::string_view> leftcopy{ left.begin() + static_cast<int>(i), left.end() };
			leftcopy[0] = convert;

			std::vector<std::string_view> rightcopy{ right.begin() + static_cast<int>(i), right.end() };
			
			auto innercomp{ compare(leftcopy, rightcopy, depth + 1) };
			if (innercomp != 0)
			{
				return innercomp;
			}
			continue;
		}
	}
	return 0; // Lists run out at the same time
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

			auto splittyLeft{ split(left) };
			auto splittyRight{ split (right) };

			auto comp{ compare(splittyLeft, splittyRight) };

			if (comp == 1)
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
				if (compare(split(left), split(right)) > 0)
				{
					return true;
				}
				else
				{
					return false;
				}
			});

		std::cout << '\n' ;

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