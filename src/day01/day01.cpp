#include <algorithm>
#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>

#include "../../include/utils.h"

namespace Puzzle1
{
	void solve(const std::string& infile)
	{
		std::ifstream inf { infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		int highestCals{ 0 };
		int elfCals{ 0 };

		while(!inf.eof())
		{
			std::string instr;
			std::getline(inf, instr);

			if (instr.length())
			{
				elfCals += std::stoi(instr);
				continue;
			}

			// new elf
			if (elfCals > highestCals)
			{
				highestCals = elfCals;
			}
			elfCals = 0;
		}

		std::cout << "Highest calories: " << highestCals << '\n';
	}
};

namespace Puzzle2
{
	void compareCals(std::array<int, 3> &arr, int cals)
	{
		if (cals < arr[0])
		{
			return;
		}

		arr[0] = cals;

		// Keep the least at position 0
		std::sort(arr.begin(), arr.end());
	}

	void solve(const std::string& infile)
	{
		std::ifstream inf{ infile };

		if (!inf)
		{
			throw std::runtime_error("could not open " + infile);
		}

		std::string instr;

		std::array<int, 3> highestCals{ 0, 0, 0 };
		int elfCals{ 0 };

		while(!inf.eof())
		{
			std::getline(inf, instr);

			if (!instr.length())
			{
				// new elf
				compareCals(highestCals, elfCals);
				elfCals = 0;

				continue;
			}

			elfCals += std::stoi(instr);
		}

		auto caloriesSum{ std::accumulate(highestCals.begin(), highestCals.end(), 0) };
		std::cout << "Top three elves combined calories: " << caloriesSum << '\n';
	}



};

int main()
{
	const std::string input{ utils::getFilePath(__FILE__) };

    Puzzle1::solve(input); 
	Puzzle2::solve(input);
	
	return 0;
}