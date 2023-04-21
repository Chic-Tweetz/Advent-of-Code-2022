// --- Day 4: Camp Cleanup ---

#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#include "utils.h"

namespace Day4
{
	// std::array<int, 4> getRanges(std::string_view elfpair) // string_view caused a subscript out of range error on Windows not sure why
	std::array<int, 4> getRanges(const std::string & elfpair)
	{
		static const std::array<char, 4> splitChars{ '-', ',', '-', '\0' };
		std::array<int, 4> ranges{};
		size_t arrInd{ 0 };

		size_t start{ 0 };
		size_t end{ 0 };

		for (auto ch : splitChars)
		{
			while (elfpair[end + 1] != ch && end++ < elfpair.length() - 1);

			ranges[arrInd++] = std::atoi(elfpair.substr(start, end).data());

			start = end += 2; // oo dirty
		}

		return ranges;
	}
}

namespace Puzzle1
{
	// Not passing by ref, should be move semantics here
	bool checkContained(std::array<int, 4> pairs)
    {
        // Our ranges pair are 0,1 and 2,3 for each elf's min & max
		// Does either pair contain the other?
		return (pairs[0] >= pairs[2] && pairs[1] <= pairs[3]) ||
		       (pairs[0] <= pairs[2] && pairs[1] >= pairs[3]);
    }

	void solve(const std::string& infile)
	{
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}
		
		int containedCount{ 0 };
        while (inf)
        {
            std::string elfpair;
            std::getline(inf, elfpair);

            if (elfpair.length() < 1)
            {
				utils::printAnswer("contained count: ", containedCount);
                return; // finished
            }

			containedCount += checkContained(Day4::getRanges(elfpair));
		}
	}
};

namespace Puzzle2
{
	bool checkOverlap(std::array<int, 4> pairs)
    {
        // Our ranges pair are 0,1 and 2,3 for each elf's min & max
		return 	(pairs[0] <= pairs[3] && pairs[1] >= pairs[2]);
    }


	void solve(const std::string& infile)
	{
		std::ifstream inf{ infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		int overlapCount{ 0 };
        while (inf)
        {
            std::string elfpair;
            std::getline(inf, elfpair);

            if (elfpair.length() < 1)
            {
				utils::printAnswer("overlap count: ", overlapCount);
                return; // finished
            }

			overlapCount += checkOverlap(Day4::getRanges(elfpair));
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
		std::cerr << e.what() << '\n';
	}

	return 0;

}
