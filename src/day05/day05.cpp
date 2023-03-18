#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "utils.h"

namespace Day5
{
	using CrateStackType = std::deque<char>;
	using CrateStacksType = std::vector<CrateStackType>;

	const uint crateSpacing{ 4 }; // each letter is 4 indices apart: [Z] [F]

	// Should give us 9 crate deques with the top crate at each index at pop_front()
	CrateStacksType makeCratesVector(const std::string &instr)
	{
		// 4 chars per crate, 3 for the last crate per line which has no trailing space
		const uint stacksCount{ static_cast<uint>( std::ceil(instr.length() / static_cast<double>(crateSpacing)) ) };
		
		CrateStacksType crates;
		crates.reserve(stacksCount);

		// Creating empty deques for our crates to go into
		for(uint i{ 0 }; i < stacksCount; ++i)
		{
			crates.push_back(CrateStackType{});
		}

		return crates;

	}

	bool buildInitialCrateStacks(const std::string& instr, CrateStacksType &crates)
	{
		for(size_t i{ 0 }; i < crates.size(); ++i)
		{
			const int letterOffset{ 1 }; // start from the second char each line (miss the [ )
			const char endChar{ '1' }; // when we find a 1, we've found all the crates

			char crateCh{ instr[letterOffset + i * crateSpacing] };
			
			if (crateCh == endChar)
			{
				return false;
			}
			
			if (crateCh != ' ')
			{
				crates[i].push_back(crateCh);
			}
		}
		
		return true;
	}

    std::array<uint, 3> parseInstruction(const std::string & instr)
    {
        const std::array<std::string, 3> instructionWords{ "move ", " from ", " to " };
        std::array<uint, 3> instruction;

        std::string tonumber{};

        size_t chindex{ 0 };
        
        for (size_t i{ 0 }; i < instruction.size(); ++i)
        {
            chindex += instructionWords[i].length();

            while (chindex < instr.length() && instr[chindex] != ' ')
            {   
                tonumber.push_back(instr[chindex++]);
            }

			// Silly bool cast so that we get proper indices when i > 0
            instruction[i] = static_cast<uint>( std::stoi(tonumber) ) - static_cast<bool>(i);
            tonumber = "";
        }
		
		return instruction;
	
    }

    void printTopCrates(const CrateStacksType &crates)
    {
        std::cout << "Top crates: \n";
        for (auto& stack : crates)
        {
            std::cout << stack.front();
        }
        std::cout << '\n';
    }
};

namespace Puzzle1
{
	using namespace Day5;

	void moveCrates(CrateStacksType &crates, const std::array<uint, 3> &instruction)
	{
		// [0] = repeats [1] = from [2] = to
		for(uint i{ 0 }; i < instruction[0]; ++i)
		{
			crates[instruction[2]].push_front(crates[instruction[1]].front());
			crates[instruction[1]].pop_front();
		}
	}

	void solve(const std::string& infile)
	{
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		std::string instr;
		std::getline(inf, instr);

		auto crates{ makeCratesVector(instr) }; 

		while(buildInitialCrateStacks(instr, crates))
		{
			std::getline(inf, instr);
		}

		// It's just one empty line but we'll pretend we need to find the instructions
		while(instr.substr(0, 4) != "move")
		{
			std::getline(inf, instr);
		}
		
		while(inf && instr.length())
		{
			moveCrates(crates, parseInstruction(instr));
			std::getline(inf, instr);
		}

		printTopCrates(crates);
	}
};

namespace Puzzle2
{
	using namespace Day5;

	void moveCrates(CrateStacksType &crates, const std::array<uint, 3> &instruction)
    {
		// Stack on the bottom, then back to the top so we get the right order
        for (size_t i{ 0 }; i < instruction[0]; ++i)
        {
            crates[instruction[1]].push_back(crates[instruction[1]].front());
            crates[instruction[1]].pop_front();
        }
        for (size_t i{ 0 }; i < instruction[0]; ++i)
        {
            crates[instruction[2]].push_front(crates[instruction[1]].back());
            crates[instruction[1]].pop_back();
        }
    }

	void solve(const std::string& infile)
	{
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		std::string instr;
		std::getline(inf, instr);

		auto crates{ makeCratesVector(instr) }; 

		while(buildInitialCrateStacks(instr, crates))
		{
			std::getline(inf, instr);
		}

		// It's just one empty line but we'll pretend we need to find the instructions
		while(instr.substr(0, 4) != "move")
		{
			std::getline(inf, instr);
		}
		
		while(inf && instr.length())
		{
			moveCrates(crates, parseInstruction(instr));
			std::getline(inf, instr);
		}

		printTopCrates(crates);
	}
};

int main()
{
	const std::string input{ utils::getFilePath(__FILE__) };

    Puzzle1::solve(input); 
	Puzzle2::solve(input);
	
	return 0;
}
