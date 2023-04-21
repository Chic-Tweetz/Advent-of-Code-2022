// --- Day 3: Rucksack Reorganization ---

#include <algorithm>
#include <array>
#include <exception> 
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "utils.h"

namespace Day3
{
    int charValue(char ch)
    {
        // a-z = 1-26, A-Z = 27-52
        constexpr int lowerSub{ 'a' - 1 };  
        constexpr int upperSub{ 'A' - 27 }; 

        return ch > 'Z' ? ch - lowerSub : ch - upperSub;
    }
};

namespace Puzzle1
{
    char findSharedItems(std::string_view comp1, std::string_view comp2)
    {
        std::vector<char> ignore;
        ignore.reserve(comp1.length());

        for (auto ch1 : comp1)
        {
            // Might be more efficient if we ignore chars we've already checked:
            if (std::find(ignore.begin(), ignore.end(), ch1) != ignore.end())
            {
                continue;
            }

            // Does our char from compartment 1 appear in compartment 2?
            for (auto ch2 : comp2)
            {
                if (ch1 == ch2)
                {
                    return ch1;
                }
            }
            ignore.push_back(ch1);
        }
        return ' ';
    }

    char findSharedItems(std::string_view rucksack)
    {
        std::string_view firstHalf{ rucksack.substr(0, rucksack.length() / 2) };
        std::string_view secondHalf{ rucksack.substr(rucksack.length() / 2, rucksack.length() / 2) };

        char dupe{ findSharedItems(firstHalf, secondHalf) };
        if (dupe != ' ')
        {
            return dupe;
        }
        else
        {
            std::cerr << "no duplicates (this shouldn't happen!)\n";
            throw std::runtime_error("couldn't find a solution!");
        }
    }

	void solve(const std::string& infile)
	{
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

        int prioritiesSum{ 0 };

        while (inf)
        {
            std::string instr;
            std::getline(inf, instr);

            if (instr.length() > 1)
            {
                prioritiesSum += Day3::charValue(findSharedItems(instr));
            }
            else
            {
                utils::printAnswer("priorities sum: ", prioritiesSum);
                return; // finished
            }
        }
	}
};

namespace Puzzle2
{

    char findSharedBadge(std::array<std::string, 3> elfGroup)
    {
        std::vector<char> ignore;

        for (auto ch1: elfGroup[0])
        {
            if (std::find(ignore.begin(), ignore.end(), ch1) != ignore.end())
            {
                continue;
            }

            if (std::find(elfGroup[1].begin(), elfGroup[1].end(), ch1) == elfGroup[1].end())
            {
                ignore.push_back(ch1);
                continue;
            }

            if (std::find(elfGroup[2].begin(), elfGroup[2].end(), ch1) != elfGroup[2].end())
            {
                return ch1;
            }
        }

        return ' ';
    }

	void solve(const std::string& infile)
	{
		std::ifstream inf{ infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

        std::array<std::string, 3> elfGroup;
        int sum{ 0 };

        while (inf)
        {
            for (size_t i{ 0 }; i < 3; ++i)
            {
                std::getline(inf, elfGroup[i]);
                
                if(elfGroup[i].length() < 1)
                {
                    utils::printAnswer("priorities sum: ", sum);
                    return; // finished
                }
            }

            sum += Day3::charValue(findSharedBadge(elfGroup));
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
